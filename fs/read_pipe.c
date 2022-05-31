#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_BUF 1024

/*
Shell script to send data to this pipe:

#!/bin/bash

fifo_name="/tmp/myfifo";

# Create fifo if it doesn't exist
[ -p $fifo_name ] || mkfifo $fifo_name;

echo "Data from shell to application" > $fifo_name;
echo "END" > $fifo_name;

*/


void read_pipe()
{
	int fd;
	char* myfifo = "/tmp/myfifo";
	int i = 0;

	mkfifo(myfifo, 0666);
	char buf[MAX_BUF];

	fd = open(myfifo, O_RDONLY);

	while ((strncmp(buf, "END", 3) != 0)) {
		if ((i = read(fd, buf, MAX_BUF)) > 0) {
			buf[i - 1] = '\0';
			printf("Received [%s], %d bytes long\n", buf, i);
		}
	}

	close(fd);
}

int main()
{
	read_pipe();
	return 0;
}