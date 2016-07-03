#if HAVE_CONFIG_H
#  include <config.h>
#endif

#if HAVE_STDLIB_H
#  include <stdlib.h>
#endif

#if HAVE_STRING_H
#  include <string.h>
#endif

#if HAVE_PTHREAD_H
#  include <pthread.h>
#endif

#include <stdio.h>

#if HAVE_UNISTD_H
#  include <unistd.h>
#endif

#if HAVE_SQLITE3_H
#  include <sqlite3.h>
#endif

#include <getopt.h>
#include <sys/shm.h> 
#include <sys/sem.h>
#include <sys/ipc.h> 
#include <time.h>
#include <signal.h>

#include <galaxy.h>
#include <error.h>

#include "plogd.h"

#define SHMEM_LENGTH  4097

struct shmem {
	char name[SHMEM_LENGTH];
	char version[SHMEM_LENGTH];
};

struct thread_data_t {
	struct galaxy_t galaxy;
	struct sembuf sops;
	struct shmem *shmem;
	sqlite3 *db;
};

static pthread_t galaxy_reciever, signal_thread;
static sigset_t mask;

void
usage(FILE *iostream)
{
	fprintf(iostream, "Usage: plogd [-r <root_directory>]\n");
	fprintf(iostream, "  -r <dir>, --root-dir=<dir>  Specify the root directory to log all package\n");
	fprintf(iostream, "                              events. Defaults to '/' (root), but using this\n");
	fprintf(iostream, "                              option argument you can treat any directory as\n");
	fprintf(iostream, "                              the root directory (useful if you are building\n");
	fprintf(iostream, "                              in a chroot environment.\n");
}

static void *
signal_handler(void *arg)
{
	int err, signo;

	while (1) {
		err = sigwait(&mask, &signo);
		switch (signo) {
			case SIGINT:
				err_msg("DEBUG[signal_handler]: SIGINT caught.\n");
				pthread_cancel(galaxy_reciever);
				return NULL;
				break;
			case SIGQUIT:
				err_msg("DEBUG[signal_handler]: SIGQUIT caught.\n");
				break;
			default:
				err_msg("warning[signal_handler]: Unexpected signal %d.\n", signo);
				break;
		}
	}

	return NULL;
}

static void *
recv_events(void *arg)
{
	struct thread_data_t *tdata;
	struct galaxy_event_t *gevent;
	char fmt_time[200];
	struct tm *local_time;

	tdata = (struct thread_data_t *)arg;

	while (1) {
		gevent = galaxy_receive(&(tdata->galaxy));
		if (gevent == NULL) {
			err_msg("warning[recv_events]: gevent is NULL. Ignoring this event.\n");
			continue;
		}
		local_time = localtime(&(gevent->timestamp));
		if (local_time == NULL) {
			err_msg("warning[recv_events]: Unable to convert Epoch time to localtime.\n");
			continue;
		}
		if (strftime(fmt_time, sizeof(fmt_time), "%Y-%m-%d %H:%M:%S", local_time) == 0) {
			err_msg("warning[recv_events]: Unable to format time string.\n");
			continue;
		}
		err_msg("%s[0x%x]: %s\n", fmt_time, gevent->mask, gevent->name);
	}

	return NULL;
}

int main(int argc, char **argv)
{
	int err, c, version, option_index, mem_d, sem_d, regexp_len;
	char *root_dir = NULL, *seed, *home_dir;
	static struct option long_options[] = {
		{"help", 0, 0, 'h'},
		{"root-dir", 1, 0, 'r'},
		{"version", 0, 0, 'v'}
	};
	key_t sem_key, shm_key;
	sigset_t oldmask;
	struct thread_data_t *tdata;

	/* Parse command-line arguments (if any). */
	option_index = version = err = 0;
	while ((c = getopt_long(argc, argv, "hr:v", long_options,
		     &option_index)) != -1) {
		switch (c) {
			case 'h':
				usage(stdout);
				exit(0);
			case 'r':
				root_dir = optarg;
				break;
			case 'v':
				printf("%d.%d.%d\n", PLOGD_MAJOR, PLOGD_MINOR, PLOGD_RELEASE);
				exit(0);
			case '?':
				err = 1;
				break;
		}
	}

	/* Check if any invalid parameters were passed into this program. */
	if (err) {
		usage(stderr);
		exit(1);
	}

	/* There should be no lone command-line arguments. Exit oterwise. */
	if (argc != optind) {
		err_msg("error[main]: There should be no lone command-line arguments.\n");
		usage(stderr);
		exit(1);
	}

	/* Store the optional root directory. Append the watch regular
	 * expression to the root directory. */
	if (root_dir) {
		regexp_len = strlen(root_dir) + 4;
	} else {
		regexp_len = 4;
	}
	if (root_dir && root_dir[strlen(root_dir) - 1] != '/')
		regexp_len++;
	char regexp[regexp_len];
	regexp[0] = '\0';
	strcpy(regexp, "^");
	if (root_dir) {
		strcat(regexp, root_dir);
		if (root_dir && root_dir[strlen(root_dir) - 1] != '/')
			strcat(regexp, "/");
	}
	strcat(regexp, ".*");
	err_msg("DEBUG[main]: regexp = %s\n", regexp);

	/* Allocate the galaxy event thread data structure. */
	tdata = malloc(sizeof(struct thread_data_t));
	if (tdata == NULL) {
		err_malloc(errno);
		err_msg("error[main]: Unable to allocate thread data structure.\n");
		exit(1);
	}

	/* Initialize the sqlite database. */
	home_dir = getenv("HOME");
	if (home_dir == NULL) {
		err_msg("error[main]: Unable to get environment home directory.\n");
		err_msg("    Please set your $HOME environment variable appropriately.\n");
		exit(1);
	}

	char dbname[strlen(home_dir) + 11];
	strcpy(dbname, home_dir);
	strcat(dbname, "/.plog.db");
	err_msg("DEBUG[main] db name = %s\n", dbname);
	err = sqlite3_open("plog.db", &(tdata->db));
	if (err) {
		err_msg("error[main]: Unable to open database `plog.db':\n\t.\n",
			sqlite3_errmsg(tdata->db));
		exit(1);
	}

	/* Init the IPC Shared memory segment. */
	/* Get two SysV IPC keys (use home directory and some number). */
	seed = getenv("HOME");
	if (!seed) {
		err_msg("error[main]: Unable to create System V IPC key.\n");
		err_msg("    The $HOME environment variable is not set. Please set $HOME to your\n");
		err_msg("    home directory and try again.\n");
		exit(1);
	}
	shm_key = ftok(seed, 23);
	if (shm_key == -1) {
		/* TODO: Add ftok() error handler. */
		err_stat(errno);  /* Man page refers to an errno with stat(2). */
		err_msg("error[main]: Unable to create System V IPC key.\n");
		exit(1);
	}
	sem_key = ftok(seed, 24);
	if (sem_key == -1) {
		/* TODO: Add ftok() error handler. */
		err_stat(errno);  /* Man page refers to an errno with stat(2). */
		err_msg("error[main]: Unable to create System V IPC key.\n");
		exit(1);
	}

	/* Get the shared memory descriptor. Limit to user of this process. */
	mem_d = shmget(shm_key, sizeof(struct shmem),  IPC_CREAT | 0600);
	if (mem_d < 0) {
		/* TODO: Add shmget() error handler. */
		err_msg("error[main]: Unable to allocate shared memory segment.\n");
		exit(1);
	}

	/* Attach this memory segment to a system selected address. */
	tdata->shmem = shmat(mem_d, 0, 0);
	if ((signed long)(tdata->shmem) == -1) {
		/* TODO: Add shmat() error handler. */
		err_msg("error[main]: Unable to attach shared memory segment.\n");
		exit(1);
	}

	/* Get a semaphore to protect this region. */
	sem_d = semget(sem_key, 1, IPC_CREAT | 0600);
	if (sem_d == -1) {
		/* TODO: Add semget() error handler. */
		err_msg("error[main]: Unable to get a semaphore set identifier.\n");
		exit(1);
	}

	/* Initialize the shared variable. */
	strcpy(tdata->shmem->name, "none");
	strcpy(tdata->shmem->version, "0");

	/* Set the semaphore to one to allow a single process to access the
	 * shared variable at a time. */
	tdata->sops.sem_num = 0;
	tdata->sops.sem_op = 1;
	tdata->sops.sem_flg = 0;
	if (semop(sem_d, &(tdata->sops), 1) < 0) {
		err_msg("error[main]: Unable to get semaphore.\n");
		exit(1);
	}

	/* Init signal handler thread. */
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGQUIT);
	err = pthread_sigmask(SIG_BLOCK, &mask, &oldmask);
	if (err != 0) {
		err_pthread_sigmask(errno);
		exit(1);
	}
	pthread_create(&signal_thread, NULL, signal_handler, 0);

	/* Init the galaxy connection. */
	err = galaxy_connect(&(tdata->galaxy));
	if (err < 0) {
		err_msg("error[main]: Unable to connect to galaxy server.\n");
		exit(1);
	}
	galaxy_ignore_watch(&(tdata->galaxy), GAL_ALL_EVENTS, "^/dev");
	galaxy_ignore_watch(&(tdata->galaxy), GAL_ALL_EVENTS, "^/proc");
	galaxy_ignore_watch(&(tdata->galaxy), GAL_ALL_EVENTS, "^/sys");
	galaxy_watch(&(tdata->galaxy), GAL_CREATE | GAL_DELETE | GAL_MODIFY, regexp);

	pthread_create(&galaxy_reciever, NULL, recv_events, tdata);

	pthread_join(galaxy_reciever, NULL);
	pthread_join(signal_thread, NULL);

	galaxy_close(&(tdata->galaxy));
	sqlite3_close(tdata->db);

	free(tdata);

	return 0;
}
