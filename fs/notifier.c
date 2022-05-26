/**
 * Monitors files/folders for events and execute optional action upon new event
 * gcc notifier.c -o notifier
 */


#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <unistd.h>

/* size of the event structure */
#define EVENT_SIZE (sizeof(struct inotify_event))
/** reasonable guess as to size of 1024 events */
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))
/** the length of the action */
#define ACTION_LENGTH (50 + FILENAME_MAX)

/** the linked list of the path we wish to listen to */
struct filedata
{
	char* pathname;        /* the file path */
	int wd;                /* the fd of the file */
	uint32_t listen_to;    /* the flags we should listen to with the current file */
	char* action[10];      /* the action to be taken when the notification occur */
	char* watchfor;        /* watching for action on this file */
	struct filedata* next; /** the next record */
};

/** the record that stores information regarding inotify and the linked list */
struct file_list
{
	int fd;                 /** the file descriptor of inotify */
	struct filedata* files; /** the linked list for each requested files */
};

typedef struct filedata filestruct;
typedef struct file_list filelist;

/**
 *	init_list:
 *		Initializes the file_list
 */
static struct file_list* init_list(void)
{
	int fd = inotify_init();
	if (fd < 0) {
		perror("inotify_init");
		return NULL;
	}

	filelist* list = (filelist*)malloc(sizeof(filelist));
	list->fd = fd;
	list->files = NULL;
	return list;
}

/**
 *	path_exists:
 *		Check if the path actually exists
 *	@param argv - The file or folder to be checked
 */
static unsigned char path_exists(const char* path)
{
	struct stat buf;
	return (stat(path, &buf) == 0 || errno != ENOENT);
}

/**
 *	register_file:
 *		Process the parameters and add a notification watch to the inotify
 *	@param list - A list of the files we listen to
 *	@param pathname - The full path of the file/folder we want to watch
 *	@param flags - The flags we want to watch for for the specified pathname
 *	@param action - the action to be taken upon an event happen to the watched element
 *	@param watchfor - Watch for an specific file to be created/delete in the folder, or NULL
 */
static unsigned char register_file(struct file_list* list, const char* pathname, uint32_t flags, const char* action, const char* watchfor)
{
	/** Check if the file or folder exists */
	if (path_exists(pathname)) {
		struct filedata* fdata = (struct filedata*)malloc(sizeof(struct filedata));
		int i = 0;
		char* tok;
		char* s_action = strdup(action);

		/** Copy the pathname to the structure */
		fdata->pathname = strdup(pathname);

		/** Parse the action string, split into argv array for execvp */
		tok = strtok(s_action, " ");
		fdata->action[i] = tok ? strdup(tok) : tok;
		while (fdata->action[i] != NULL && i < 9) {
			tok = strtok(NULL, " ");
			fdata->action[++i] = tok ? strdup(tok) : tok;
		}
		fdata->action[++i] = (char*)0;
		free(s_action);

		/** Set the watch point */
		if (watchfor != NULL) {
			fdata->watchfor = strdup(watchfor);
        } else {
			fdata->watchfor = NULL;
        }

		/** Set the flags, update the list, add the notification */
		fdata->listen_to = flags;
		fdata->next = list->files;
		fdata->wd = inotify_add_watch(list->fd, pathname, flags);

		if (fdata->wd < 0) {
			free(fdata);
			perror("inotify_add_watch");
			return 0;
		}

		list->files = fdata;
		return 1;
	}
	perror("file not found");
	return 0;
}

/**
 *	populate_list:
 *		Register the files we want to monitor
 *		We need:
 *			file or folder name
 *			flags to monitor
 *				 IN_ACCESS, IN_ATTRIB, IN_CLOSE_WRITE, IN_CLOSE_NOWRITE, IN_CREATE, IN_OPEN,
 *				 IN_DELETE, IN_DELETE_SELF, IN_MODIFY, IN_MOVE_SELF, IN_MOVED_FROM, IN_MOVED_TO
 *			the script/app to run when the action happens (scripts do accept parameters)
 *			a specific file that we should be watching for (or NULL if none)
 *	@param list - A list of the files we listen to
 */
static void populate_list(filelist* list)
{
	register_file(list, "/tmp/nftest/fchg", IN_MODIFY, "/scripts/notify_action.sh fchg modify", NULL);
	register_file(list, "/tmp/nftest", IN_CREATE, "/scripts/notify_action.sh create", "testcreate");
}

/**
 *	find_filedata:
 *		Loop through the file_list to find the one specified in wd
 *	@param list - A list of the files we listen to
 *	@param wd - Descriptor of the list element we want to find
 */
static struct filedata* find_filedata(struct file_list* list, int wd)
{
	struct filedata* filepos = list->files;
	while (NULL != filepos) {
		if (wd == filepos->wd)
			return (filepos);
		filepos = filepos->next;
	}
	return (NULL);
}

/**
 *	exec_event:
 *		Fork the process and execute the action
 *	@param argv - The list of arguments to be passed
 */
void exec_event(char* argv[])
{
	int status;
	pid_t pid;

	pid = fork();

	if (pid == -1) {
		/* fork error - cannot create child */
		perror("fork error - cannot create child");
		exit(1);

	} else if (pid == 0) { /* code for child */
		/** Execute the action */
		execvp(argv[0], &argv[0]);
		_exit(1);

	} else {
		/* code for parent */
	}
}

#ifdef DEBUG
void log_event(struct inotify_event* pevent, char* fname, char* action)
{
	char* msg = NULL;
	if (pevent->mask & IN_ACCESS)
		msg = "was read";
	if (pevent->mask & IN_ATTRIB)
		msg = "Metadata changed";
	if (pevent->mask & IN_CLOSE_WRITE)
		msg = "was opened for writing and is now closed";
	if (pevent->mask & IN_CLOSE_NOWRITE)
		msg = "was not opened for writing and is now closed";
	if (pevent->mask & IN_CREATE)
		msg = "created in watched directory";
	if (pevent->mask & IN_DELETE)
		msg = "deleted from watched directory";
	if (pevent->mask & IN_DELETE_SELF)
		msg = "Watched file/directory was itself deleted";
	if (pevent->mask & IN_MODIFY)
		msg = "was modified";
	if (pevent->mask & IN_MOVE_SELF)
		msg = "Watched file/directory was itself moved";
	if (pevent->mask & IN_MOVED_FROM)
		msg = "moved out of watched directory";
	if (pevent->mask & IN_MOVED_TO)
		msg = "moved into watched directory";
	if (pevent->mask & IN_OPEN)
		msg = "was opened";

	printf("EVENT: [%s]\n\t %s, action is %s (%04X:%s)\n",
	         fname,
	         msg,
	         action,
	         pevent->mask,
	         (pevent->len) ? pevent->name : fname);
}
#endif

/**
 * listen:
 *		listen to inotify events
 *	@param list - A list of the files we listen to
 */
static void listen(struct file_list* list)
{
	ssize_t len, i = 0;
	char buff[EVENT_BUF_LEN] = { 0 };

	len = read(list->fd, buff, EVENT_BUF_LEN - 1);

	while (i < len) {
		struct inotify_event* pevent = (struct inotify_event*)&buff[i];
		struct filedata* filepos = find_filedata(list, pevent->wd);

#ifdef DEBUG
		log_event(pevent, filepos->pathname, filepos->action[0]);
		printf("Event Received, executing\n");
#endif
		/** Check if the event is related to a specific file we are watching for */
		if (filepos->watchfor == NULL || (filepos->watchfor && pevent->len && strcmp(pevent->name, filepos->watchfor) == 0)) {
			exec_event(filepos->action);
		}

		i += sizeof(struct inotify_event) + pevent->len;
	}
}

int main(int argc, char* argv[])
{

	fprintf(stderr, "Starting notifier\n");

	filelist* list = init_list();
	if (NULL == list) {
		return (1);
    }

	populate_list(list);

	while (1) {
		listen(list);
	}

	return (0);
}
