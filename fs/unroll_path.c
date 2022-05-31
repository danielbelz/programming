/**
 * Unrolls the give path including the redirections
 * gcc unroll_path.c -o unroll_path
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <limits.h> /* PATH_MAX */

int count_features(char* path, int* slashes, int* backdirs)
{
	int bd = 0;
	int s = 0;
	// Count depth and number of backdirs
	printf("PATH = [%s]\n", path);
	char* token = strtok(path, "/");
	while (token != NULL) {
		s++;
		printf("S=%d  %s\n", s, token);
		if (token[0] == '.' && token[1] == '.') {
			if (strlen(token) > 2) {
				printf("Invalid path = [%s]\n", path);
				return -1;
			}
			printf("Backdir\n");
			bd++;
		} else if (token[0] == '.' && strlen(token) == 1) {
			printf("This dir, skipping\n");
			s--;
		}
		token = strtok(NULL, "/");
	}

	*slashes = s;
	*backdirs = bd;
	return 0;
}

int unroll_path(char* fullpath)
{
	int slashes = 0;
	int backdir = 0;
	bool is_folder = false;

	printf("FULLPATH = [%s]\n", fullpath);
	char* cf = strdup(fullpath);
	if (count_features(cf, &slashes, &backdir) == -1) {
		free(cf);
		return -1;
	}
	free(cf);

	// If it ends with / it's probably a folder
	if (fullpath[strlen(fullpath) - 1] == '/') {
		is_folder = 1;
    }

	// Allocate array for entries.
	int arrsz = slashes - backdir;
	char** patharr = calloc((arrsz + 1), sizeof(char*));

	printf("FULLPATH = [%s]\n", fullpath);

	// Get the valid entries
	int s_start = 0, s_end = 0;
	int j = 0;

	char* token = strtok(fullpath, "/");
	while (token != NULL) {
		printf("J=%d  %s\n", j, token);
		if (token[0] == '.' && token[1] == '.') {
			printf("Backdir\n");
			j--;
			if (j < 0) {
				printf("ERROR Invalid path (below root)\n");
				free(patharr);
				return -1;
			} else {
				char* bdir = patharr[j];
				printf("Releasing %d:%s %p\n", j, bdir, bdir);
				if (patharr[j]) {
					free(patharr[j]);
					patharr[j] = NULL;
				}
			}
		} else if (token[0] == '.' && strlen(token) == 1) {
			printf("This dir, skipping\n");
		} else {
			patharr[j] = strdup(token);
			j++;
		}
		token = strtok(NULL, "/");
		printf("token = %s\n", token);
	}

	// Build the path to be validated
	memset(fullpath, 0, PATH_MAX);
	s_start = 0;
	for (j = 0; j <= arrsz; j++) {
		char* bdir = patharr[j];
		if (bdir) {
			fullpath[s_start] = '/';
			s_start++;
			memcpy(&fullpath[s_start], bdir, strlen(bdir));
			s_start += strlen(bdir);
			printf("PATH = %s\n", fullpath);
			free(bdir);
		}
	}
	if (is_folder) {
		fullpath[s_start] = '/';
		s_start++;
	}

	free(patharr);
	printf("Unrolled path = [%s]\n", fullpath);
	return 0;
}

int main(void)
{
	char buf[PATH_MAX]; /* PATH_MAX incudes the \0 so +1 is not required */
	char fullpath[PATH_MAX] = { 0 };
	char* localFileName = "//folder/.//.././../c/d/";
	snprintf(fullpath, 127, "%s", localFileName);

	unroll_path(fullpath);
	printf("Fullpath = [%s]\n", fullpath);

	return 0;
}