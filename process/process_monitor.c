/**
 * Starts the processes listed in the configuration file if they are not running yet.
 * Otherwise, monitors the process and respawns it if it terminates.
 * gcc process_monitor.c -o procmon
 */


#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <ctype.h>

int global; /* In BSS segement, will automatically be assigned '0'*/
#define THRESHOLD 10

typedef struct _process_list
{
	char* procname;
	int pid;
	unsigned int respawns;
	unsigned char quickrespawns;
	time_t uptime;
	struct _process_list* next;
} proclst;

static char* trim(char* s)
{
	char *s1 = s, *s2 = &s[strlen(s) - 1];
	while ((isspace(*s2)) && (s2 >= s1)) {
		s2--;
    }
	*(s2 + 1) = '\0';

	while ((isspace(*s1)) && (s1 < s2)) {
		s1++;
    }

	strcpy(s, s1);
	return s;
}

static void start_process(proclst* p_proc, int respawn)
{
	int pid = 0;

	printf("Request to %s process %s\n", (respawn) ? "respawn" : "start", p_proc->procname);
	printf("Respawns: %d, Quickrespawns:%d, Uptime=%d\n", p_proc->respawns, p_proc->quickrespawns, (int)(time(NULL) - p_proc->uptime));
	if (respawn) {
		// Check for the uptime and quick respawn count
		time_t tnow = time(NULL);
		if ((tnow - p_proc->uptime) < 60) {
			p_proc->quickrespawns--;
        } else {
			// Looks like the process was running for a while, give it more tries
			p_proc->quickrespawns = THRESHOLD;
        }

		// If uptime is low and respawn count is greater than THRESHOLD, don't respawn anymore
		if (p_proc->quickrespawns == 0) {
			printf("Process %s is respawning too fast, disabling it\n", p_proc->procname);
			return;
		}
	}

	if ((pid = fork()) < 0) {
		perror("fork");
		abort();
	} else if (pid == 0) {
		// Child process
		char* argv[10];
		int i = 0;

		argv[i] = strtok(p_proc->procname, " ");
		while (argv[i] != NULL && i < 9) {
			argv[++i] = strtok(NULL, " ");
        }
		argv[++i] = (char*)0;

		printf("child PID =  %d, parent pid = %d, will run %s\n", getpid(), getppid(), p_proc->procname);
		if (respawn) {
			sleep(2);
        }
		execvp(argv[0], &argv[0]);
		_exit(1);
	}
	p_proc->pid = pid;
	p_proc->uptime = time(NULL);
	if (respawn) {
		p_proc->respawns++;
    }
}

static unsigned char path_exists(const char* path)
{
	struct stat buf;
	return (stat(path, &buf) == 0 || errno != ENOENT);
}

void register_process(proclst** list, const char* pathname)
{
	printf("Register Process [%s]\n", pathname);

	proclst* new_proc = (proclst*)malloc(sizeof(proclst));
	new_proc->procname = strdup(pathname);
	new_proc->respawns = 0;
	new_proc->quickrespawns = THRESHOLD;
	new_proc->uptime = 0;

	new_proc->next = (*list);
	(*list) = new_proc;
}

void populate_list(proclst** list, int argc, char* argv[])
{
	FILE* configfile = fopen(argv[1], "r");
	if (configfile == NULL) {
		// Try default location in case of error
		configfile = fopen("/etc/procmon.conf", "r");
	}

	if (configfile != NULL) {
		char delim[] = ",";
		char line[1024];

		while (fgets(line, sizeof(line), configfile) != NULL) {
			char* token;
			char* path = NULL;

			/* Skip blank lines and comments */
			if (line[0] == '\n' || line[0] == '#') {
				continue;
            }

			token = strtok(line, delim);
			path = strdup(token);

			if (path_exists(trim(path))) {
				register_process(list, path);
			} else {
				fprintf(stderr, "The path \"%s\" does not exists.\n", path);
			}

			// Free the memory allocated through strdup
			if (path) {
				free(path);
            }
		}
	} else {
		// No config file, exit.
		exit(0);
	}
}

proclst* pid_lookup(proclst* list, int pid)
{
	proclst* plst = list;
	printf("Looking up for %d\n", pid);
	while (NULL != plst) {
		if (pid == plst->pid)
			return (plst);
		plst = plst->next;
	}
	return (NULL);
}

int main(int argc, char* argv[])
{
	proclst* plst = NULL;
	proclst* p = NULL;
	populate_list(&plst, argc, argv);

	/* Start processes. */
	p = plst;
	while (NULL != p) {
		start_process(p, 0);
		p = p->next;
	}

	/* Wait for children to exit. */
	int status;
	pid_t pid;
	while (1) {
		pid = wait(&status);
		printf("Child with PID %ld exited with status 0x%x.\n", (long)pid, status);
		proclst* p = pid_lookup(plst, pid);
		start_process(p, 1);
	}
}