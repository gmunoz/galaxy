/*
 * Galaxy - A filesystem monitoring tool.
 * Copyright (C) 2005  Gabriel Munoz <gabriel@xusia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>

#if HAVE_STDLIB_H
#  include <stdlib.h>
#endif

#if HAVE_STRING_H
#  include <string.h>
#endif

#if HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif

#if HAVE_SYS_STAT_H
#  include <sys/stat.h>
#endif

#if HAVE_FCNTL_H
#  include <fcntl.h>
#endif

#if HAVE_UNISTD_H
#  include <unistd.h>
#endif

#if HAVE_SYS_IOCTL_H
#  include <sys/ioctl.h>
#endif

#if HAVE_SYS_SELECT_H
#  include <sys/select.h>
#endif

#if HAVE_DIRENT_H 
#  include <dirent.h>
#endif

#if HAVE_ERRNO_H
#  include <errno.h>
#endif

#if HAVE_PTHREAD_H
#  include <pthread.h>
#endif

#if HAVE_LIBGLIB_2_0
#  include <glib.h>
#endif

#include <libintl.h>
#define _(String) gettext (String)

#include <getopt.h>

#include "galnet.h"
#include "crawler_thread.h"
#include "signal_thread.h"
#include "server_thread.h"
#include "iwatch_thread.h"
#include "watch.h"
#include "inotify_utils.h"
#include "list.h"
#include "event_queue.h"
#include "error.h"

pthread_t crawler, watcher, signaler, server;
pthread_mutex_t inotify_wds_mutex = PTHREAD_MUTEX_INITIALIZER;

GHashTable *inotify_wds = NULL;

//#define LOCKFILE "/var/run/galaxyd.pid"
#define LOCKFILE "/tmp/galaxyd.pid"
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int
lockfile(int fd)
{
	struct flock fl;

	fl.l_type = F_WRLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;

	return fcntl(fd, F_SETLK, &fl);
}

int
already_running(void)
{
	int fd;
	char buf[16];

	fd = open(LOCKFILE, O_RDWR | O_CREAT, LOCKMODE);
	if (fd < 0) {
		err_open(errno);
		err_msg("error[already_running]: Unable to open lock file.\n");
		return -1;
	}
	if (lockfile(fd) < 0) {
		if (errno == EACCES || errno == EAGAIN) {
			err_msg("error[already_running]: galaxyd is already running.\n");
			close(fd);
			return -1;
		}
		err_msg("error[already_running]: Unexpected file locking error %s: %s\n",
			LOCKFILE, strerror(errno));
		return -1;
	}
	ftruncate(fd, 0);
	sprintf(buf, "%ld", (long)getpid());
	write(fd, buf, strlen(buf) + 1);
	close(fd);

	return 0;
}

void
destroy(gpointer key, gpointer value, gpointer user_data)
{
	free(key);
	free(value);
}

void
notifier_destroy(gpointer key, gpointer value, gpointer user_data)
{
	free(key);
	list_destroy(value);
}

void
usage(FILE *iostream)
{
	fprintf(iostream, "Usage: galaxyd [-h] [-v] [-r] [-p PRUNE_LIST] [DIRECTORY]\n");
	fprintf(iostream, "  -h              Displays this information.\n");
	fprintf(iostream, "  -p PRUNE_LIST   Prune the colon-separated directories from the galaxy\n");
	fprintf(iostream, "                  search path.\n");
	fprintf(iostream, "  -r              Recursively add Galaxy watches.\n");
	fprintf(iostream, "  -v              Output version information and exit.\n");
}

int
main (int argc, char **argv)
{
	queue_t q;
	int err, fd, listenfd, c, version, recursive, option_index, i;
	int lone_args;
	char *galaxy_search_path, *galaxy_prune_path, *prune_dir_args = NULL;
	list_t *dirs, *prune_dirs = NULL;
	static struct option long_options[] = {
		{"help", 0, 0, 'h'},
		{"prune", 1, 0, 'p'},
		{"recursive", 0, 0, 'r'},
		{"version", 0, 0, 'v'}
	};

	/* Only allow one instance. */
	if (already_running() == -1) {
		err_msg("error[main]: %s is already running.\n", argv[0]);
		exit(1);
	}

	dirs = list_create(free);
	if (dirs == NULL) {
		err_msg("error[main]: Unable to create directories list.\n");
		return (1);
	}

	prune_dirs = list_create(free);
	if (prune_dirs == NULL) {
		err_msg("error[main]: Unable to create prune list.\n");
		return (1);
	}

	option_index = version = recursive = err = 0;
	while ((c = getopt_long(argc, argv, "hp:rv",
		     long_options, &option_index)) != -1) {
		switch (c) {
			case 'h':
				usage(stdout);
				exit(0);
				break;
			case 'p':
				prune_dir_args = optarg;
				break;
			case 'r':
				recursive = 1;
				break;
			case 'v':
				printf("%d.%d.%d\n", GALAXY_MAJOR, GALAXY_MINOR, GALAXY_RELEASE);
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

	/* Iterate over all remaining lone arguments. Each argument is assumed
	 * to be a directory that needs to be added as a top-level galaxy
	 * watch. */
	lone_args = argc - optind;
	for (i = 0; i < lone_args; i++) {
		list_push(dirs, strdup(argv[optind + i]));
	}

	/* Add directories in the colon-separated environment variable
	 * GALAXY_SEARCH_PATH to the directories list to add watches. */
	galaxy_search_path = getenv("GALAXY_SEARCH_PATH");
	if (galaxy_search_path) {
		char *dir;
		while (dir = strtok(galaxy_search_path, ":")) {
			list_push(dirs, strdup(dir));
			galaxy_search_path = NULL;
		}
	}

	/* Add command-line supplied prune directories, if any. */
	if (prune_dir_args) {
		char *dir;
		while (dir = strtok(prune_dir_args, ":")) {
			list_push(prune_dirs, strdup(dir));
			prune_dir_args = NULL;
		}
	}

	/* Add prune directories from the environment variable
	 * GALAXY_PRUNE_PATH, which is a colon-separated list. */
	galaxy_prune_path = getenv("GALAXY_PRUNE_PATH");
	if (galaxy_prune_path) {
		char *dir;
		while (dir = strtok(galaxy_prune_path, ":")) {
			list_push(prune_dirs, strdup(dir));
			galaxy_prune_path = NULL;
		}
	}

	/* If directories list is zero, add the current working directory as
	 * the directory to search. */
	char *cwd;
	if (list_size(dirs) == 0) {
		cwd = getenv("PWD");
		if (cwd)
			list_push(dirs, strdup(cwd));
	}

	/* Init locale data. */
	/* TODO: PACKAGE and LOCALEDIR should be provided by config.h. */
	/*
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdeomain(PACKAGE);
	*/

	init_client_watches_container();

	listenfd = serv_listen(GALAXY_SOCKET);
	if (listenfd < 0) {
		err_serv_listen(listenfd);
		exit(1);
	}

	inotify_wds = g_hash_table_new(g_int_hash, g_int_equal);

	fd = open_dev();
	if (fd < 0)
		return 0;

	q = queue_create (128);

	/* Signal thread */
	err = create_signal_thread(&signaler);
	if (err < 0) {
		err_msg("error: Unable to create signal thread.\n");
		return 1;
	}

	/* Server thread */
	err = create_server_thread(&server, listenfd);
	if (err < 0) {
		err_msg("error: Unable to create server thread.\n");
		return 1;
	}

	/* Inotify event watcher thread */
	err = create_iwatch_thread(&watcher, fd, q);
	if (err < 0) {
		err_msg("error: Unable to create inotify event watcher thread.\n");
		return 1;
	}

	/* Directory crawler thread */
	err = create_crawler_thread(&crawler, fd, dirs, prune_dirs,
		inotify_wds, recursive);
	if (err < 0) {
		err_msg("error: Unable to create crawler thread.\n");
		return 1;
	}

	pthread_join(crawler, NULL);
	pthread_join(watcher, NULL);
	pthread_join(server, NULL);
	pthread_join(signaler, NULL);

	queue_destroy (q);

	close_dev (fd);

	g_hash_table_foreach(inotify_wds, destroy, NULL);
	g_hash_table_destroy(inotify_wds);

	close(listenfd);

	destroy_client_watches_container();

	list_destroy(dirs);
	list_destroy(prune_dirs);

	err_msg("Exiting now!\n");

	return 0;
}
