/* This file is licensed under the GPL v2 (http://www.gnu.org/licenses/gpl2.txt) (some parts was originally borrowed from proc events example)

   pmon.c

   code highlighted with GNU source-highlight 3.1 */

#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/connector.h>
#include <linux/cn_proc.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

#define CGROUPD_FIFO "/tmp/cgroup_fifo_pids"
#define CGROUPD_PID_LOCK_FILE "/var/run/cgroup_notifier.pid"

//#define printf(...)
int fifo_fd;

//Enum Declaration for Process State(FORK || EXEC|| EXIT) Scenarios

enum process_state{ event_fork, event_exec, event_exit};

typedef struct Processdetails
{
	enum process_state state;
	unsigned int pid;
	unsigned int ppid;
}Processdetails1;

static void exit_self(int status)
{
	remove(CGROUPD_FIFO);
	exit(status);
}

/*
 * connect to netlink
 * returns netlink socket, or -1 on error
 */

static int nl_connect(void)
{
	int rc;
	int nl_sock;
	struct sockaddr_nl sa_nl;

	nl_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);
	if (nl_sock == -1) {
		perror("socket");
		return -1;
	}

	sa_nl.nl_family = AF_NETLINK;
	sa_nl.nl_groups = CN_IDX_PROC;
	sa_nl.nl_pid = getpid();

	rc = bind(nl_sock, (struct sockaddr *)&sa_nl, sizeof(sa_nl));
	if (rc == -1) {
		perror("bind");
		close(nl_sock);
		return -1;
	}
	return nl_sock;
}


/*Subscribe on proc events (process notifications)*/

static int set_proc_ev_listen(int nl_sock, bool enable)
{
	int rc;
	struct __attribute__ ((aligned(NLMSG_ALIGNTO))) {
		struct nlmsghdr nl_hdr;
		struct __attribute__ ((__packed__)) {
			struct cn_msg cn_msg;
			enum proc_cn_mcast_op cn_mcast;
		};
	} nlcn_msg;

	memset(&nlcn_msg, 0, sizeof(nlcn_msg));
	nlcn_msg.nl_hdr.nlmsg_len = sizeof(nlcn_msg);
	nlcn_msg.nl_hdr.nlmsg_pid = getpid();
	nlcn_msg.nl_hdr.nlmsg_type = NLMSG_DONE;

	nlcn_msg.cn_msg.id.idx = CN_IDX_PROC;
	nlcn_msg.cn_msg.id.val = CN_VAL_PROC;
	nlcn_msg.cn_msg.len = sizeof(enum proc_cn_mcast_op);

	nlcn_msg.cn_mcast = enable ? PROC_CN_MCAST_LISTEN : PROC_CN_MCAST_IGNORE;

	rc = send(nl_sock, &nlcn_msg, sizeof(nlcn_msg), 0);
	if (rc == -1) {
		perror("netlink send");
		return -1;
	}
	return 0;
}

/* handle a single process event*/

static volatile bool need_exit = false;
static int handle_proc_ev(int nl_sock)
{
	int rc;
	Processdetails1 ps_details;
	struct __attribute__ ((aligned(NLMSG_ALIGNTO))) {
		struct nlmsghdr nl_hdr;
		struct __attribute__ ((__packed__)) {
			struct cn_msg cn_msg;
			struct proc_event proc_ev;
		};
	} nlcn_msg;

	while (!need_exit) {
		rc = recv(nl_sock, &nlcn_msg, sizeof(nlcn_msg), 0);
		if (rc == 0) {
			/* shutdown? */
			return 0;
		} else if (rc == -1) {
			if (errno == EINTR) continue;
			perror("netlink recv");
			return -1;
		}
		ps_details.ppid = 0;
		switch (nlcn_msg.proc_ev.what) {
			case PROC_EVENT_NONE:
				printf("set mcast listen ok\n");
				break;
			case PROC_EVENT_FORK:
				ps_details.pid = nlcn_msg.proc_ev.event_data.fork.child_pid;
				ps_details.ppid = nlcn_msg.proc_ev.event_data.fork.parent_pid;
				ps_details.state = event_fork;
				write(fifo_fd, &ps_details, sizeof(Processdetails1));
				break;

			case PROC_EVENT_EXEC:
				ps_details.pid = nlcn_msg.proc_ev.event_data.exec.process_pid;
				ps_details.state = event_exec;
				write(fifo_fd, &ps_details, sizeof(Processdetails1));
				break;

			case PROC_EVENT_EXIT:
				ps_details.pid = nlcn_msg.proc_ev.event_data.exit.process_pid;
				ps_details.state = event_exit;
				write(fifo_fd, &ps_details, sizeof(Processdetails1));
				break;

			default:
				break;
		}
	}

	return 0;
}

static void signal_handler(int signo)
{
	switch (signo) {
		case SIGINT:
			need_exit = true;
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
	remove(CGROUPD_FIFO);
}

int main(void)
{
	int nl_sock;
	int rc = EXIT_SUCCESS;
	char fifo_name[]=CGROUPD_FIFO;

	prepare_self();

	fifo_fd=mkfifo(fifo_name,0666);		
	if(fifo_fd==-1)
		return 0;
	fifo_fd=open(fifo_name,O_WRONLY);
	if(fifo_fd==-1)
		return 0;

	signal(SIGINT, &signal_handler);
	signal(SIGTERM, &signal_handler);
	signal(SIGSEGV, &signal_handler);
	signal(SIGQUIT, &signal_handler);

	nl_sock = nl_connect();
	if (nl_sock == -1)
		exit_self(EXIT_FAILURE);

repeat:
	rc = set_proc_ev_listen(nl_sock, true);
	if (rc == -1) {
		rc = EXIT_FAILURE;
		goto out;
	}

	rc = handle_proc_ev(nl_sock);
	if (rc == -1) {
		set_proc_ev_listen(nl_sock, false);
		sleep(2); /*Wait for some time for buffer space incase buffer is full for kernel events*/
		goto repeat;
	}

	set_proc_ev_listen(nl_sock, false);

out:
	close(nl_sock);
	exit_self(rc);
	return (rc);
}
