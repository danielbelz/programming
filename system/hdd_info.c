/**
 * Retrieve HDD Serial Number
 * gcc hdd_info.c -o hdinfo
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/hdreg.h>
#include <errno.h>

void main(int argc, char** argv)
{
	static struct hd_driveid hd;
	const char* devname;
	int fd;

	if (argc != 2) {
		fprintf(stderr, "Usage: hdinfo <device>\n");
		return 1;
	}
	devname = argv[1];

	if ((fd = open(devname, O_RDONLY | O_NONBLOCK)) < 0) {
		printf("ERROR opening %s\n", devname);
		exit(1);
	}

	if (!ioctl(fd, HDIO_GET_IDENTITY, &hd)) {
		printf("%.20s\n", hd.serial_no);
	} else if (errno == -ENOMSG) {
		printf("No serial number available\n");
	} else {
		perror("ERROR: HDIO_GET_IDENTITY");
		exit(1);
	}

    return 0;
}