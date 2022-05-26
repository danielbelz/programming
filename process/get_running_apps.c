/**
 * Programatically gets the list of applications running in Linux (like ps)
 * gcc get_running_apps.c -o rnps
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>

static void get_executable(const char* dirName)
{
	FILE* f;
	char link[32];
	snprintf(link, 32, "%s/cmdline", dirName);

	char buff[1024];
	printf("link %s\n", link);

	f = fopen(link, "r");
	if (f != NULL) {
		fscanf(f, "%s", buff);
	}
	fclose(f);
	printf("%s %s\n", dirName, buff);
}

int main(int argc, char** argv)
{
	int uid = getuid();
	if (chdir("/proc") == -1) {
		printf("chdir /proc failed\n");
		return -1;
	}

	DIR* procDir;
	if ((procDir = opendir(".")) == NULL) {
		printf("cannot opendir(/proc)\n");
		return -1;
	}

	struct dirent* d;
	while ((d = readdir(procDir)) != NULL) {
		if (!atoi(d->d_name)) {
			continue;
		}
		struct stat st = { .st_dev = 0 };
		if (stat(d->d_name, &st)) {
			printf("Failed to stat [%s]\n", d->d_name);
		}
		if (st.st_uid == uid || uid == 0) {
			// allows an user to examine their own processes and root can examine any processes
			get_executable(d->d_name);
		}
	}
	closedir(procDir);

    return 0;
}