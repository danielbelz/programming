/**
 * Monitors status changes in a proc entry
 * gcc monitor_proc.c -o procmon
 */

#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PROC_ENTRY "/proc/some/entry/here"

int main()
{
	int mfd = open(PROC_ENTRY, O_RDONLY, 0);
	struct pollfd pfd;
	int rv;

	int changes = 0;
	pfd.fd = mfd;
	pfd.events = POLLERR | POLLPRI;
	pfd.revents = 0;
	while ((rv = poll(&pfd, 1, 5)) >= 0) {
		if (pfd.revents & POLLERR) {
			fprintf(stdout, "Entry status changed. %d.\n", changes++);
		}

		pfd.revents = 0;

		if (changes > 10) {
			exit(EXIT_FAILURE);
		}
	}

    return 0;
}