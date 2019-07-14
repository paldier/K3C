/***************************************************************************** *
 *     File Name  : errorhd.c                                                  *
 *     Project    : UGW                                                        *
 *     Description: Error Handler Daemon main program and core.                *
 *                                                                             *
 ******************************************************************************/
#define _GNU_SOURCE
#include <stdio.h>
#include <time.h>
#include <syslog.h>
#include <stdlib.h>
#include <unistd.h>
#include <libconfig.h>
#include <string.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#ifdef UTS_MODE
#include <telemetry.h>
#endif
#include "errorhd.h"
#include "errorhd_log.h"

#define ERRORHD_CONFIG_PATH                                             "/opt/errorhd.cfg"
#define ERRORHD_DEFAULT_NUM_LAST_LOGS_TO_STORE  5
#define ERRORHD_MAX_NUM_LAST_LOGS_TO_STORE          25
#define ERRORHD_MAX_LOG_PATH                                          128
#define ERRORHD_KERNEL_CRASHLOG_PATH                    "/sys/kernel/debug/crashlog"
#define ERRORHD_KERNELCRASH_ZIP_PATH                      "/tmp/kernelcrash.zip"
#define ERRORHD_MAX_PAD_COMMAND_LEN			512

uint16_t LOGLEVEL = SYS_LOG_DEBUG;
uint16_t LOGTYPE = SYS_LOG_TYPE_FILE_AND_CONSOLE;

static char logs_path[ERRORHD_MAX_LOG_PATH] = "/opt/padlogs/";
static uint logs_num = ERRORHD_DEFAULT_NUM_LAST_LOGS_TO_STORE;
static bool errorhdDisabled = false;
static int child_pid = -1;

static int errorhd_readConfFromFile(char *path);
static int errorhd_dispatch(struct eh_info *info);
static void errorhd_listener(void);
static void errorhd_signalHandler(int nSigNum, siginfo_t *pxSigInfo, void *pvContext __attribute__((unused)));
static void errorhd_signalRegister(void);
static void* errorhd_handle_kernel_crash(void *ptr);
static int errorhd_set_running_state(bool state);
static void remove_spaces(char *source);
static void remove_new_line(char *str);
static void changes_slashs_to_hyphens(char *source);
static int errorhd_pad(const char *suffix, const char *headline,
			  const char *subsys, const char *plugin,
			  bool usb, int remove, const char *path,
			  bool tftp, int count, bool blocking);
#ifdef UTS_MODE
static void errorhd_extract_epc_register(char *path_to_crashlog_file, char *epc, size_t buflen);
static void errorhd_extract_ra_register(char *path_to_crashlog_file, char *epc, size_t buflen);
static void* create_pad_uts_report(void *ptr);
#else
static void* run_pad(void *ptr);
#endif

static int errorhd_pad(const char *suffix, const char *headline,
			  const char *subsys, const char *plugin,
			  bool usb, int remove, const char *path,
			  bool tftp, int count, bool blocking)
{
	char cmd[ERRORHD_MAX_PAD_COMMAND_LEN] = { 0 };
	int ret = 0, status = 0, len = 0;
	FILE *filePtr = NULL;

	len = snprintf(cmd, ERRORHD_MAX_PAD_COMMAND_LEN, "pad");

	if (suffix)
		len += snprintf(cmd + len, ERRORHD_MAX_PAD_COMMAND_LEN - len, " -s \"%s\"", suffix);
	if (headline)
		len += snprintf(cmd + len, ERRORHD_MAX_PAD_COMMAND_LEN - len, " -H \"%s\"", headline);
	if (subsys)
		len += snprintf(cmd + len, ERRORHD_MAX_PAD_COMMAND_LEN - len, " -m \"%s\"", subsys);
	if (plugin)
		len += snprintf(cmd + len, ERRORHD_MAX_PAD_COMMAND_LEN - len, " -p \"%s\"", plugin);
	if (usb)
		len += snprintf(cmd + len, ERRORHD_MAX_PAD_COMMAND_LEN - len, " -u");
	if (remove)
		len += snprintf(cmd + len, ERRORHD_MAX_PAD_COMMAND_LEN - len, " -r %d", remove);
	if (path)
		len += snprintf(cmd + len, ERRORHD_MAX_PAD_COMMAND_LEN - len, " -f \"%s\"", path);
	if (tftp)
		len += snprintf(cmd + len, ERRORHD_MAX_PAD_COMMAND_LEN - len, " -C");
	if (count >= 0)
		len += snprintf(cmd + len, ERRORHD_MAX_PAD_COMMAND_LEN - len, " -c %d", count);

	/*
	Temporary solution - add support to not call to PAD. It needed for automation.
	In case that /etc/disable_scapi_pad exists, don't call to PAD, just print to log.
	*/
	if ((filePtr = fopen("/etc/disable_scapi_pad", "r")) != NULL ) {
		fclose(filePtr);
		LOGF_LOG_INFO("pad won't be called, scapi_pad is disabled, command=%s\n", cmd);
		return ret;
	}

	LOGF_LOG_INFO("Calling pad, command=%s\n", cmd);
	ret = errorhd_spawn(cmd, blocking, &status);
	LOGF_LOG_INFO("COMPLETED calling pad, command=%s\n", cmd);

	return ret || status;
}

static void remove_spaces(char *source)
{
	char *i = source;
	char *j = source;

	if (source == NULL) {
		return;
	}

	while (*j != '\0') {
		*i = *j++;
		if (*i != ' ') {
			i++;
		}
	}
	*i =  '\0';
}

static void remove_new_line(char *str)
{
	uint32_t i = 0;

	if (str == NULL) {
		return;
	}

	for (i = 0; i < strlen(str); i++) {
		if (str[i] == '\n') {
			str[i] = '\0';
			break;
		}
	}
}

static void changes_slashs_to_hyphens(char *str)
{
	uint32_t i = 0;

	if (str == NULL) {
		return;
	}

	for (i = 0; i < strlen(str); i++) {
		if (str[i] == '/') {
			str[i] = '-';
		}
	}
}

static int errorhd_set_running_state(bool state)
{
	FILE *f;

	if (state) {
		f = fopen(ERRORHD_STATE_FILE, "w");
		if (f != NULL) {
			fclose(f);
		} else {
			LOGF_LOG_ERROR("Failed to create %s (%s)\n", ERRORHD_STATE_FILE, strerror(errno));
			return -1;
		}
	} else {
		if(remove(ERRORHD_STATE_FILE)) {
			LOGF_LOG_ERROR("Failed to remove %s (%s)\n", ERRORHD_STATE_FILE, strerror(errno));
			return -1;
		}
	}

	return 0;
}

static int errorhd_readConfFromFile(char *path)
{
	config_t cfg;
	config_setting_t *setting;
	const char *str;
	DIR *d;
	int ret = 0;

	LOGF_LOG_INFO("reading conf file, path = %s\n", path);

	if (access(ERRORHD_CONFIG_PATH, F_OK) == -1) {
		LOGF_LOG_ERROR("Configuration file %s not found!\n", path);
		return -1;
	}

	config_init(&cfg);
	/* Read the file. If there is an error, report it and exit. */
	if (!config_read_file(&cfg, ERRORHD_CONFIG_PATH)) {
		LOGF_LOG_ERROR("Error reading config file!\n");
		ret = -1;
		goto out;
	}

	setting = config_lookup(&cfg, "errorhd");
	if (!setting) {
		LOGF_LOG_ERROR("Can't find errorhd config!\n");
		ret = -1;
		goto out;
	}

	if (config_setting_lookup_string(setting, "Enable", &str)) {
		LOGF_LOG_INFO("config Enable = %s\n", str);
		if (!strcmp(str, "false")) {
			errorhdDisabled = true;
		} else {
			errorhdDisabled = false;
		}
	}

	if (config_setting_lookup_string(setting, "LogsLocation", &str)) {
		LOGF_LOG_INFO("errorhd: config Logs Location = %s\n", str);
		d = opendir(str);
		if (d) {
			LOGF_LOG_INFO("directory %s found\n", str);
			strlcpy(logs_path, str, sizeof(logs_path));
			closedir(d);
		} else {
			/* Try to create dir */
			ret = mkdir(str, 0700);
			if (ret) {
				LOGF_LOG_ERROR("Can't create dir %s : %s\n", str, strerror(errno));
				goto out;
			}
		}
	}

	if (config_setting_lookup_string(setting, "NumLastLogsToStore", &str)) {
		int numLogs = 1;
		numLogs = strtoul(str, NULL, 0);
		if ((numLogs >= 1 && numLogs <=  ERRORHD_MAX_NUM_LAST_LOGS_TO_STORE)) {
			LOGF_LOG_INFO("config number of logs to store = %d\n", numLogs);
			logs_num = (uint)numLogs;
		} else {
			LOGF_LOG_ERROR("Error in NumLastLogsToStore = %s, valid numbers are 1 to 25!\n", str);
			ret = -1;
			goto out;
		}
	}

out:
	config_destroy(&cfg);
	return ret;
}

#ifdef UTS_MODE
static void errorhd_extract_epc_register(char *path_to_crashlog_file, char *epc, size_t buflen)
{
	FILE *crashlog = NULL;
	char line[256] = { 0 };
	char *str = NULL;

	crashlog = fopen(path_to_crashlog_file, "r");
	if (!crashlog) {
		LOGF_LOG_ERROR("error in opening the file %s\n", path_to_crashlog_file);
		return;
	}

	/* Look for the epc register
	   epc shows the address of the instruction that caused a crash */
	while (fgets(line, sizeof(line), crashlog)) {
		if (strstr(line, "epc   :")) {
			str = strtok(line, ":");
			str = strtok(NULL, " ");
			str = strtok(NULL, "+");
			if (str) {
				strlcpy(epc, str, buflen);
				remove_new_line(epc);
				break;
			}
		}
	}

	fclose(crashlog);
}

static void errorhd_extract_ra_register(char *path_to_crashlog_file, char *ra, size_t buflen)
{
	FILE *crashlog = NULL;
	char line[256] = { 0 };
	char *str = NULL;

	crashlog = fopen(path_to_crashlog_file, "r");
	if (!crashlog) {
		LOGF_LOG_ERROR("error in opening the file %s\n", path_to_crashlog_file);
		return;
	}

	/* Look for the ra register
	   ra shows the return address of the instruction that caused a crash */
	while (fgets(line, sizeof(line), crashlog)) {
		if (strstr(line, "ra    :")) {
			str = strtok(line, ":");
			str = strtok(NULL, " ");
			str = strtok(NULL, "+");
			if (str) {
				strlcpy(ra, str, buflen);
				remove_new_line(ra);
				break;
			}
		}
	}

	fclose(crashlog);
}

static void* create_pad_uts_report(void *ptr)
{
	char suffixStr[32] = { 0 };
	char file_path[128] = { 0 };
	char file_suffix[64] = { 0 };
	char *tmp_desc = NULL;
	char *tmp_data0 = NULL;
	struct eh_info *info = ptr;
	DIR *d;
	struct dirent *dir;
	unsigned int seed =0;
	char cmd[256] = { 0 };
	struct timeval current;

	prctl(PR_SET_NAME, "create_pad_uts_report");

	/* Check if to create padlog */
	if (!info->nopadlog) {
		/* Prepare suffixStr from desc_data0 w/o spaces or \n */
		tmp_desc = strdup(info->subsys);
		if (!tmp_desc) {
			LOGF_LOG_ERROR("Can't duplicate desc string!\n");
			goto end;
		}
		tmp_data0 = strdup(info->data[0]);
		if (!tmp_data0) {
			LOGF_LOG_ERROR("Can't duplicate data0 string!\n");
			goto end;
		}

		/* Remove spaces */
		if (strstr(tmp_desc, " "))
			remove_spaces(tmp_desc);
		if (strstr(tmp_data0, " "))
			remove_spaces(tmp_data0);
		/* Remove new line */
		if (strstr(tmp_desc, "\n"))
			remove_new_line(tmp_desc);
		if (strstr(tmp_data0, "\n"))
			remove_new_line(tmp_data0);
		/* Changes slashs to hyphens */
		if (strstr(tmp_desc, "/"))
			changes_slashs_to_hyphens(tmp_desc);
		if (strstr(tmp_data0, "/"))
			changes_slashs_to_hyphens(tmp_data0);

		/* Run PAD, wait for it to finish and send report to UTS */
		gettimeofday(&current, NULL);
		seed = (unsigned int)(current.tv_usec + time(NULL));
		srand(seed);

		snprintf(suffixStr, sizeof(suffixStr), "%.13s_%.13s_%d", tmp_desc, tmp_data0, (rand()%9000 + 1000)); /* 4 digits random number */
		errorhd_pad(suffixStr, info->headline, NULL, NULL, true, logs_num, logs_path, false, -1, ERRORHD_BLOCK);
	}

	if (!tm_is_telemd_running()) {
		goto end;
	}

	/* Check if to collect padlog */
	if (!info->nopadlog) {
		/* Look for the log file with the uniqueStr */
		d = opendir(logs_path);
		if (!d) {
			fprintf(stderr, "Can't open %s\n", logs_path);
			goto end;
		}

		snprintf(file_suffix, sizeof(file_suffix), "%s.zip", suffixStr);
		while ((dir = readdir(d)) != NULL) {
			if (strstr(dir->d_name, file_suffix)) {
				snprintf(file_path, sizeof(file_path), "%s/%s", logs_path, dir->d_name);
				break;
			}
		}
		closedir(d);

		/* If PAD failed, create log from /var/log/ */
		if (!strcmp(file_path,"")) {
			snprintf(file_path, sizeof(file_path), "%s/logger_%s.zip", logs_path, suffixStr);
			/* create the zip log file */
			snprintf(cmd, sizeof(cmd), "zip -r %s /var/log/", file_path);
			system(cmd);
			sprintf(cmd, "pad_cleanup.sh %s %u",logs_path, logs_num);
			system(cmd);
		}
	}

	/* create & send the UTS report */
	if (tm_create_send_record("crash", info->subsys, info->data[0], info->data[1], info->data[2], info->headline, file_path)) {
		LOGF_LOG_ERROR("Failed to send record to UTS daemon\n");
		goto end;
	}

end:
	if (info)
		free(info);
	if (tmp_desc)
		free(tmp_desc);
	if (tmp_data0)
		free(tmp_data0);

	return NULL;
}

#else
static void* run_pad(void *ptr)
{
	struct eh_info *info = ptr;

	/* Remove spaces */
	if (strstr(info->subsys, " "))
		remove_spaces(info->subsys);
	/* Remove new line */
	if (strstr(info->subsys, "\n"))
		remove_new_line(info->subsys);
	/* Changes slashs to hyphens */
	if (strstr(info->subsys, "/"))
		changes_slashs_to_hyphens(info->subsys);

	if (!info->nopadlog) {
		errorhd_pad(info->subsys, info->headline, NULL, NULL, true, logs_num, logs_path, false, -1, ERRORHD_UNBLOCK);
	}

	if (info)
		free(info);

	return NULL;
}
#endif

static int errorhd_dispatch(struct eh_info *info)
{
	pthread_t thid;
	pthread_attr_t tattr;
	struct eh_info *local_info;

	local_info = malloc(sizeof(*local_info));
	if (!info) {
		LOGF_LOG_ERROR("malloc failed\n");
		goto out_error;
	}
	memcpy(local_info, info, sizeof(*local_info));

	/* Fill the subsys if missing */
	if (!strcmp(local_info->subsys, "")) {
		snprintf(local_info->subsys, sizeof(local_info->subsys), "errorhd");
	}

	/* Fill the headline if missing */
	if (!strcmp(local_info->headline, "")) {
		snprintf(local_info->headline, sizeof(local_info->headline), "high priority crash detected!, subsys=%s", local_info->subsys);
	}

	LOGF_LOG_INFO("\nsubsys = %s\ndata[0] = %s\ndata[1] = %s\ndata[2] = %s\npad = %s\nheadline = %s\n",
		local_info->subsys,
		local_info->data[0], local_info->data[1], local_info->data[2],
		(local_info->nopadlog ? "false" : "true"), local_info->headline);


#ifdef UTS_MODE
	/* Create thread to run PAD and send to UTS */
	pthread_attr_init(&tattr);
	pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
	if (pthread_create(&thid, &tattr, create_pad_uts_report, local_info)) {
		LOGF_LOG_ERROR("errorhd pthread_create failed\n");
		goto out_error;
	}
#else
	/* Create thread to run PAD */
	pthread_attr_init(&tattr);
	pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
	if (pthread_create(&thid, &tattr, run_pad, local_info)) {
		LOGF_LOG_ERROR("errorhd pthread_create failed\n");
		goto out_error;
	}
#endif

	return 0;

out_error:
	return -1;
}

static void errorhd_listener(void)
{
	errno = 0;

	child_pid = fork();
	if (child_pid < 0) {
		LOGF_LOG_ERROR("fork failed, exiting!\n");
		return;
	}

	do {
		/* parent process waits for signals */
		if (child_pid) {
			pause();
		} else {
			struct eh_info info = { 0 };
			size_t len = 0;
			FILE *f;

			/* child process listens on pipe */
			f = fopen(ERRORHD_NAMED_PIPE, "rb");
			if (f == NULL) {
				LOGF_LOG_ERROR("fopen failed\n");
				continue;
			}

			/* read struct eh_info */
			len = fread(&info, sizeof(struct eh_info), 1, f);
			fclose(f);
			if (len != 1) {
				continue;
			}

			/* dispatch to pad and uts */
			if (errorhd_dispatch(&info) < 0)
				LOGF_LOG_ERROR("eh_trigger failed\n");
		}
	} while (1);
}

static void errorhd_signalHandler(int nSigNum, siginfo_t *pxSigInfo, void *pvContext __attribute__((unused)))
{
	struct eh_info info = { 0 };

	switch (nSigNum) {
	case SIGHUP:
		break;
	case SIGTERM:
	case SIGSEGV:
	case SIGQUIT:
		LOGF_LOG_INFO("Exiting errohd!!, nSigNum = %d, sending PID = %d \n", nSigNum, pxSigInfo->si_pid);
		errorhd_set_running_state(false);
		kill(child_pid, SIGKILL);
		unlink(ERRORHD_NAMED_PIPE);
		unlink(ERRORHD_NAMED_PIPE_LOCK);
		exit(nSigNum);
		break;
	case SIGUSR1: //high priority crash
		LOGF_LOG_INFO("high priority crash detected!, nSigNum = %d, sending PID = %d \n",
			nSigNum, pxSigInfo->si_pid);
		snprintf(info.subsys, sizeof(info.subsys), "errorhd");
		snprintf(info.data[0], sizeof(info.data[0]), "%d", nSigNum);
		snprintf(info.data[1], sizeof(info.data[1]), "%d",  pxSigInfo->si_pid);
		snprintf(info.headline, sizeof(info.headline), "high priority crash detected!, nSigNum = %d, sending PID = %d \n",
				nSigNum, pxSigInfo->si_pid);
		/* dispatch to pad and uts */
		if (errorhd_dispatch(&info) < 0)
			LOGF_LOG_ERROR("eh_dispatch failed\n");
		break;
	case SIGUSR2:  //low priority crash
		LOGF_LOG_INFO("low priority crash detected!, nSigNum = %d, sending PID = %d \n", nSigNum, pxSigInfo->si_pid);
		break;
	}
}

static void errorhd_signalRegister(void)
{
	struct sigaction xSigAction;
	memset(&xSigAction, 0, sizeof(struct sigaction));

	xSigAction.sa_sigaction = *errorhd_signalHandler;
	xSigAction.sa_flags |= SA_SIGINFO;

	sigaction(SIGQUIT, &xSigAction, NULL);
	sigaction(SIGHUP,  &xSigAction, NULL);
	sigaction(SIGTERM, &xSigAction, NULL);
	sigaction(SIGSEGV, &xSigAction, NULL);
	sigaction(SIGUSR1, &xSigAction, NULL);
	sigaction(SIGUSR2, &xSigAction, NULL);
}

static void* errorhd_handle_kernel_crash(void *ptr)
{
	char tempStr[MAX_DATA_SIZE] = { 0 };

	prctl(PR_SET_NAME, "errorhd_handle_kernel_crash");

	if (access(ERRORHD_KERNEL_CRASHLOG_PATH, F_OK)) {
		/* No previous crash */
		return ptr;
	}

	/* the system had recovered from a crash during the last restart <==> there exists a crashlog file */
	LOGF_LOG_INFO("Kernel crash detected!!\n");

	/* create the zip log file */
	snprintf(tempStr, sizeof(tempStr), "zip %s %s", ERRORHD_KERNELCRASH_ZIP_PATH, ERRORHD_KERNEL_CRASHLOG_PATH);
	system(tempStr);

#ifdef UTS_MODE
	/* Wait for UTS boot window (first 2 minutes without reports) */
	sleep(120);
	/* if uts is not running exit */
	if (!tm_is_telemd_running()) {
		return ptr;
	}

	/* create and update struct to be sent to the UTS */
	struct eh_info info = { 0 };
	strlcpy(info.subsys, "kernel", sizeof(info.subsys));
	strlcpy(info.data[0], "panic", sizeof(info.data[0]));

	errorhd_extract_epc_register(ERRORHD_KERNEL_CRASHLOG_PATH, tempStr, sizeof(tempStr));
	snprintf(info.data[1], sizeof(info.data[1]), "%s", tempStr);

	memset(tempStr, 0 ,sizeof(tempStr));
	errorhd_extract_ra_register(ERRORHD_KERNEL_CRASHLOG_PATH, tempStr, sizeof(tempStr));
	snprintf(info.data[2], sizeof(info.data[2]), "%s", tempStr);

	LOGF_LOG_INFO("\nsubsys = %s\ndata[0] = %s\ndata[1] = %s\ndata[2] = %s\nheadline = %s\n",
		info.subsys, info.data[0], info.data[1], info.data[2], info.headline);

	if (tm_create_send_record("crash", info.subsys, info.data[0], info.data[1], info.data[2],
		 info.headline, ERRORHD_KERNELCRASH_ZIP_PATH)) {
		LOGF_LOG_ERROR("Failed to send record to UTS daemon\n");
	}
#endif

	return ptr;
}

/*=============================================================================================*
 * Function Name : main / errorhd main														   *
 * Description	 : Wait for signals that indicate exceptions/crashes and collect logs/recover  *
 *=============================================================================================*/
int main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	char cmd[64] = { 0 };
	pthread_t thid;
	pthread_attr_t tattr;

	if (errorhd_readConfFromFile(ERRORHD_CONFIG_PATH)) {
		LOGF_LOG_ERROR("Failed to configure errorhd!");
		exit(EXIT_FAILURE);
	}

	if (errorhdDisabled) {
		LOGF_LOG_INFO("errorhd is disabled!");
		exit(EXIT_SUCCESS);
	}

	mkfifo(ERRORHD_NAMED_PIPE, 0666);

	snprintf(cmd, sizeof(cmd), "echo %d > /tmp/errorhd_pid.txt", getpid());
	system(cmd);

	errorhd_signalRegister();

	/* look for previous crashes - kernel, watchdog... */
	pthread_attr_init(&tattr);
	pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
	if (pthread_create(&thid, &tattr, errorhd_handle_kernel_crash, NULL)) {
		LOGF_LOG_ERROR("errorhd pthread_create failed\n");
	}

	errorhd_set_running_state(true);

	errorhd_listener();

	/* The proccess will not reach here */

	errorhd_set_running_state(false);
	exit(EXIT_SUCCESS);
}
