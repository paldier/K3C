/***************************************************************************** *
 *     File Name  : errorhd_test.c                                             *
 *     Project    : UGW                                                        *
 *     Description: Error Handler Daemon test program.		                   *
 *                                                                             *
 ******************************************************************************/
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "errorhd.h"

int main(int argc, char *argv[])
{
	int opt;
	struct eh_info info = { 0 };

	while ((opt = getopt(argc, argv, "s:H:a:b:c:nh")) != -1) {
		switch (opt) {
		case 's':
			if (optarg) {
				strlcpy(info.subsys, optarg, sizeof(info.subsys));
			}
			break;
		case 'H':
			if (optarg) {
				strlcpy(info.headline, optarg, sizeof(info.headline));
			}
			break;
		case 'n':
			info.nopadlog = true;
			break;
		case 'a':
			if (optarg) {
				strlcpy(info.data[0], optarg, sizeof(info.data[0]));
			}
			break;
		case 'b':
			if (optarg) {
				strlcpy(info.data[1], optarg, sizeof(info.data[1]));
			}
			break;
		case 'c':
			if (optarg) {
				strlcpy(info.data[2], optarg, sizeof(info.data[2]));
			}
			break;
		case 'h':
		default:
			fprintf(stderr, "Usage: %s [options]\n"
				"\t-s <subsystem> - 1 word (32 chars)\n"
				"\t-H <headline>  - 1 line (256 chars)\n"
				"\t-a <data0>     - short description (64 chars)\n"
				"\t-b <data1>     - short description (64 chars)\n"
				"\t-c <data2>     - short description (64 chars)\n"
				"\t-n            - without padlog\n", argv[0]);
			exit(EXIT_SUCCESS);
		}
	}

	if (errorhd_trigger(&info)) {
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
