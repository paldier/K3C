/***********************************************************************************************************************
 *     FILENAME       : cgroups_daemon.c
 *
 *     DESCRIPTION    : Limit, account, and isolate resource usage (CPU, memory, disk I/O, etc.) of process groups.
 *	  	       Allocates required amount of memory and CPU shares for the specified group (high,low and default)
 *		       and also used to group the processes based on the group to which they belong to.
 *     Revision History    :
 *     DATE                NAME         	    REFERENCE                 REASON
 *     -----------------------------------------------------------------------------------------------------------------
 *     21 Aug 2014        Lantiq				       Cgroups Implementation
 *									      
 *     Copyright @ 2014 Lantiq Comm
 ************************************************************************************************************************/

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/connector.h>
#include <linux/cn_proc.h>
#include <sched.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>

#define MAX_NO_ENTRIES 20
#define MAX_LINE_LEN 200
#define BS 200
#define MAX_NO_GROUPS 3 /* Group Names are High,low and default */
#define MAX_NAME_LEN 32
#define MAX_READ_LEN 16

#define SMB_DBG 1
#define MAX_CPU 2

#define CGROUPD_PID_LOCK_FILE "/var/run/cgroup_daemon.pid"
#define CGROUPD_FIFO "/tmp/cgroup_fifo_pids"
#define CGROUPD_DEBUG_FIFO "/tmp/debug_cgroup"
#define CGROUPD_USER_CONFIG "/etc/cgroups.conf"

enum process_state { event_fork, event_exec, event_exit};
char cmdlineread[BS], buf1[BS]; 
unsigned int guiEnableDebug;
static int smbd_dynamic_affinity = 0; /* dynamic affinity is disabled by default */
unsigned int guiMaxEntries;
void Debug_print(char *fmt,...);

//Stucture Definitions

typedef struct ProcessNode
{
	char process_name[MAX_NAME_LEN];
	char group_name[MAX_NAME_LEN];
	unsigned int pid;
	unsigned long mask;
}ProcessNode1;

typedef struct MemNode
{
	char group_name[MAX_NAME_LEN];
	unsigned int MemTake;
	unsigned int CpuShare;
}MemNode1;

typedef struct Processdetails
{
	enum process_state state;
	unsigned int pid;
	unsigned int ppid;
}Processdetails1;


ProcessNode1 Process_Node[MAX_NO_ENTRIES];
MemNode1 Memory_Node[MAX_NO_GROUPS];



// Structure Definition for Linked List

struct pid_process_info {
	int nPid;
	char sProcessName[MAX_NAME_LEN];
	int mask;
	struct pid_process_info *next;
};

struct pid_process_info *pxpid_process_list_head = NULL;

typedef struct smb_pid {
	int pid;
	int pid_affinity;
	struct smb_pid *next;
} SMB_NODE;

SMB_NODE *smb_list_head = NULL;

int add_smb_pid_node(int pid, int affinity);
SMB_NODE *get_smb_node(int pid);
int delete_smb_node (int pid);

int add_smb_pid_node(int pid, int affinity)
{
	SMB_NODE *Node;
	Node = (SMB_NODE *)malloc(sizeof(SMB_NODE));
	if (Node == NULL) {
		printf("ERROR - memory allocation failed\n");
		return -1;
	}

	Node->pid = pid ;
	Node->pid_affinity = affinity;
	Node->next = smb_list_head ;
	smb_list_head = Node;

	return 0;
}

SMB_NODE *get_smb_node(int pid)
{
	SMB_NODE *Node;

	if (smb_list_head == NULL)
		return NULL;

	Node = smb_list_head;

	while (Node) {
		if (Node->pid == pid)
			return Node;
		Node = Node->next;	
	}

	return NULL;
}

int delete_smb_node(int pid)
{
	SMB_NODE *prev, *Node;

	if ( smb_list_head == NULL)
		return -1;

	prev = Node = smb_list_head;

	while (Node)
	{
		if (Node->pid == pid) {
			if (Node != smb_list_head)
				prev->next = Node->next;
			else
				smb_list_head = Node->next;

			free(Node);
			return 0;
		}

		prev = Node;
		Node = Node->next;	
	}
	printf("ERROR - pid %d not found\n", pid);
	return -1;
}

static void exit_self(int status)
{
	remove(CGROUPD_PID_LOCK_FILE);
	remove(CGROUPD_DEBUG_FIFO);
	exit(status);
}

static int __attribute__((unused)) add_PID_pName_node(int nPid, char *pcProcessName, int mask)
{
	struct pid_process_info *pNode;

	pNode = (struct pid_process_info *)malloc(sizeof(struct pid_process_info));
	if (pNode == NULL) {
		printf("ERROR - memory allocation failed\n");
		return -1;
	}

	pNode->nPid = nPid;
	snprintf(pNode->sProcessName, MAX_NAME_LEN, "%s", pcProcessName);
	pNode->mask = mask;
	pNode->next = pxpid_process_list_head;
	pxpid_process_list_head = pNode;

	return 0;
}

static __attribute__((unused)) struct pid_process_info *get_PID_pName_node(int nPid)
{
	struct pid_process_info *pNode = NULL;

	if (pxpid_process_list_head == NULL)
		return NULL;

	pNode = pxpid_process_list_head;

	while (pNode != NULL) {

		if (pNode->nPid == nPid){
			return (pNode);
		}
		pNode = pNode->next;
	}
	return (NULL);
}

static __attribute__((unused)) int delete_node(struct pid_process_info *pDelNode)
{
	struct pid_process_info *pNode = NULL, *prev = NULL;

	if (pxpid_process_list_head == NULL)
		return -1;

	pNode = pxpid_process_list_head;
	prev = pxpid_process_list_head;

	while (pNode != NULL) {

		if ((pNode->nPid == pDelNode->nPid) &&
				(strcmp(pNode->sProcessName, pDelNode->sProcessName) == 0)){
			if (pNode != pxpid_process_list_head)
				prev->next = pNode->next;
			else
				pxpid_process_list_head = pNode->next;

			free(pNode);
			return 0;
		}

		prev = pNode;
		pNode = pNode->next;
	}
	/* If pNode not found */
	printf("ERROR: pid %d not found\n", pDelNode->nPid);
	return -1;
}

// Function to enable Debug_Prints at run time

void Debug_print(char *fmt,...){
	va_list arg_ptr;

	if (guiEnableDebug)
	{
		va_start(arg_ptr, fmt);
		vprintf(fmt, arg_ptr);
		va_end(arg_ptr);
	}
}

//utility function to read data from the config file separated the tokens

static void mystrtok(FILE *fpointer)
{
	int nindex,temp;
	char *str_temp, *saveptr1, *token;
	char arr[MAX_LINE_LEN];

	str_temp = saveptr1 = token = NULL;

	str_temp= (char *)malloc(MAX_LINE_LEN);
	if(str_temp == NULL)
	{
		Debug_print("Failed to allocate memory\n");
		exit_self(EXIT_FAILURE);
	}	
	nindex=temp=0;
	memset(str_temp,0,MAX_LINE_LEN);
	memset(Memory_Node,0,sizeof(MemNode1)*MAX_NO_GROUPS);
	memset(Process_Node,0,sizeof(ProcessNode1)*MAX_NO_ENTRIES);
	/* Scan each line of file */
	while (fgets(str_temp,MAX_LINE_LEN,fpointer)!=NULL){
		/*if the read line has # as its first character, skip it*/
		if(str_temp[0]=='#'){
			memset(str_temp,0,MAX_LINE_LEN);
			continue;
		}
		if(nindex<MAX_NO_GROUPS){
			token = strtok_r(str_temp, ":", &saveptr1);
			if(token != NULL){
				strncpy(Memory_Node[nindex].group_name, token, MAX_NAME_LEN-1);
				Debug_print("Group name is %s", Memory_Node[nindex].group_name);
			}

			token = NULL;
			token = strtok_r(NULL, ":", &saveptr1);
			if(token != NULL){
				Memory_Node[nindex].MemTake=atoi(token);
				Debug_print("Memory share %d", Memory_Node[nindex].MemTake);
			}

			token = NULL;
			token = strtok_r(NULL, "\n", &saveptr1);
			if(token != NULL){
				Memory_Node[nindex].CpuShare=atoi(token);
				Debug_print("cpu share is %d\n", Memory_Node[nindex].CpuShare);
			}

			if(Memory_Node[nindex].MemTake > 0)
			{
				memset(arr,0,MAX_LINE_LEN);
				snprintf(arr, sizeof(arr),"echo %d > /sys/fs/cgroup/memory/%s/memory.limit_in_bytes",
						Memory_Node[nindex].MemTake,Memory_Node[nindex].group_name);
				system(arr);
			}
			if(Memory_Node[nindex].CpuShare > 0)
			{
				memset(arr,0,MAX_LINE_LEN);
				snprintf(arr, sizeof(arr),"echo %d > /sys/fs/cgroup/cpu/%s/cpu.shares",
						Memory_Node[nindex].CpuShare,Memory_Node[nindex].group_name);
				system(arr);
			}
		} else {
			temp=nindex-MAX_NO_GROUPS;
			/* Scan process name, group */

			token = NULL;
			token = strtok_r(str_temp, ":", &saveptr1);
			if(token != NULL){
				strncpy(Process_Node[temp].process_name, token, MAX_NAME_LEN-1);
				Debug_print("process name is %s\n",Process_Node[temp].process_name);
			}

			token = NULL;
			token = strtok_r(NULL, "\n", &saveptr1);
			if(token != NULL){
				strncpy(Process_Node[temp].group_name, token, MAX_NAME_LEN-1);
				Debug_print("process group is %s\n",Process_Node[temp].group_name);
			}
		}
		nindex++;
		if(nindex >= (MAX_NO_ENTRIES+MAX_NO_GROUPS))
			break;
		memset(str_temp,0,MAX_LINE_LEN);
	}
	guiMaxEntries=nindex-MAX_NO_GROUPS;
	Debug_print("No of entries in the table--->%d\n",guiMaxEntries);
	free(str_temp);
}

//Reading the configuration file 

static void reload_config(void)
{
	FILE *fpointer;
	fpointer = fopen(CGROUPD_USER_CONFIG, "r");
	if(fpointer == NULL)
	{
		Debug_print("Error:%s could not be opened...!\n", CGROUPD_USER_CONFIG);
		exit_self(EXIT_FAILURE);
	}
	else
	{
		mystrtok( fpointer);
		fclose( fpointer );
	}
}

/* Sigusr1 Interrupt to re-read the configuration from input file*/
static void signal_handler(int signo)
{
	switch (signo) {
		case SIGUSR1:
			Debug_print("SIGUSR1 received ");
			reload_config();
			break;
		case SIGTERM:
		case SIGSEGV:
		case SIGQUIT:
			exit_self(0);
			break;
	}
	return;
}

static void prepare_self(void)
{
	int nFp;
	struct stat info;
	char sBuf[10];

	/* Check for Lock File and exist if cannot be locked. */
	nFp = open(CGROUPD_PID_LOCK_FILE, O_RDWR|O_CREAT, 0640);
	if (nFp < 0)
		exit(1);
	if (lockf (nFp, F_TLOCK, 0) < 0)
		exit(0);

	snprintf(sBuf, 10, "%d\n", getpid());
	write(nFp, sBuf, strlen(sBuf));
	close(nFp);

	/* Remove FIFO file if exists */
	remove(CGROUPD_DEBUG_FIFO);

	/* Create cgroup path in sysfs */
	if ((stat("/sys/fs/cgroup", &info) == 0) && (info.st_mode & S_IFDIR)) {
		mkdir("/sys/fs/cgroup/cpu", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		mkdir("/sys/fs/cgroup/memory", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		mkdir("/sys/fs/cgroup/cpu/default", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		mkdir("/sys/fs/cgroup/cpu/low", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		mkdir("/sys/fs/cgroup/cpu/high", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		mkdir("/sys/fs/cgroup/memory/default", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		mkdir("/sys/fs/cgroup/memory/low", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		mkdir("/sys/fs/cgroup/memory/high", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	} else {
		perror("cgroup_daemon Error: cgroup not mounted or unable to acces '/sys/fs/cgroup' path\n");
		exit_self(EXIT_FAILURE);
	}
}

static int is_smbd_process(const pid_t pid)
{
	char procfile[100];
	char name[30];

	sprintf(procfile, "/proc/%d/cmdline", pid);
	FILE* f = fopen(procfile, "r");
	if (f) {
		size_t size;
		size = fread(name, sizeof (char), sizeof (procfile), f);
		if (size > 0) {
			if ('\n' == name[size - 1])
				name[size - 1] = '\0';
		}
		fclose(f);
	}

	return !strncmp(name, "smbd", sizeof("smbd"));
}

#define SMBD_DEFAULT_AFFINITY 1 /* default is always CPU2 */

int main(void)
{
	char fifo_name[]=CGROUPD_FIFO;
	char fifo_debug[]=CGROUPD_DEBUG_FIFO;
	int fifo_fd = 0,fifo_debug_fd = 0;
	fd_set read_fd;
	Processdetails1 ps_details;
	char aucDebugValue[MAX_READ_LEN];
	int retval;
	ssize_t r;
	int smbd_affinity = SMBD_DEFAULT_AFFINITY;
	cpu_set_t cpu_mask;
	SMB_NODE *node;

	prepare_self();

	fifo_debug_fd=mkfifo(fifo_debug,0666);
	if(fifo_debug_fd <0){
		perror("failed to make fifo\n");
		exit_self(EXIT_FAILURE);
	}
	fifo_fd=open(fifo_name,O_RDONLY|O_NONBLOCK);
	fifo_debug_fd=open(fifo_debug,O_RDWR|O_NONBLOCK);

	/* open fifo for reading */
	if(fifo_fd < 0 || fifo_debug_fd < 0) {
		perror("failed to open fifo\n");
		exit_self(EXIT_FAILURE);
	}
	reload_config();
	signal(SIGUSR1, &signal_handler);
	signal(SIGTERM, &signal_handler);
	signal(SIGSEGV, &signal_handler);
	signal(SIGQUIT, &signal_handler);

	while (1) {
		FD_ZERO(&read_fd);
		FD_SET(fifo_fd,&read_fd);
		FD_SET(fifo_debug_fd,&read_fd);
		retval = select(fifo_debug_fd+1,&read_fd,NULL,NULL,NULL);
		if(retval < 0){
			Debug_print("Error:select unblock without any proper reason\n");
			continue;
		}

		if (FD_ISSET(fifo_debug_fd,&read_fd)){
			memset(aucDebugValue, '\0', sizeof(aucDebugValue));
			r = read(fifo_debug_fd, aucDebugValue, MAX_READ_LEN-1);
			if(r > 0) {
				char *pch;
				pch = strtok(aucDebugValue, " ");
				if (!strncmp(pch, "smbdaff", sizeof("smbdaff"))) {
					pch = strtok(NULL, " ");
					if (pch) {
						smbd_dynamic_affinity = (pch[0] == '0') ? 0 : 1;
						Debug_print("smbd_dynamic_affinity: value read from fifo is %d ascii value is %c\n",pch[0],pch[0]);
					}
				} else if (!strncmp(pch, "debug", sizeof("debug"))) {
					pch = strtok(NULL, " ");
					if (pch) {
						guiEnableDebug = (pch[0] == '0') ? 0 : 1;
						Debug_print("debug: value read from fifo is %d ascii value is %c\n",pch[0],pch[0]);
					}
				} else {
					printf("%s:%d: usage: echo <debug/smbdaff> <0/1> > %s\n", __func__, __LINE__, CGROUPD_DEBUG_FIFO);
				}
			}
		}

		if (FD_ISSET(fifo_fd,&read_fd)){
			r = read(fifo_fd, &ps_details, sizeof(Processdetails1));
			/*select unblocked but no data, which means the fifo is closed in another thread so exit 
			  from this thread as  well otherwise it will cause an infinite loop */
			if (r <= 0)
				exit_self(EXIT_FAILURE);
			if (ps_details.state == event_fork) {
				if (!is_smbd_process(ps_details.pid))
					continue;
				if (ps_details.ppid == 1)
					continue;
				Debug_print("smbd pid %d fork - smbd_affinity=%d\n", ps_details.pid, smbd_affinity);
				CPU_ZERO(&cpu_mask);
				CPU_SET(smbd_affinity, &cpu_mask);
				sched_setaffinity(ps_details.pid, sizeof(cpu_mask), &cpu_mask);
				add_smb_pid_node(ps_details.pid, smbd_affinity);
				smbd_affinity = (smbd_dynamic_affinity) ? !smbd_affinity : SMBD_DEFAULT_AFFINITY;
			} else if (ps_details.state == event_exit) {
				node = get_smb_node(ps_details.pid);
				if (node) {
					smbd_affinity = (smbd_dynamic_affinity) ? node->pid_affinity : SMBD_DEFAULT_AFFINITY;
					Debug_print("smbd pid %d exit (next smbd_affinity=%d)\n", ps_details.pid, smbd_affinity);
					delete_smb_node(ps_details.pid);
				}
			}
		}
	}

	exit_self(EXIT_FAILURE);
	return 0;

}
