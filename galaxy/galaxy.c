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

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if HAVE_UNISTD_H
#  include <unistd.h>
#endif

#if HAVE_PTHREAD_H
#  include <pthread.h>
#endif

#include <signal.h>

#if HAVE_ERRNO_H
#  include <errno.h>
#endif

#include <stdio.h>

#include "galaxy.h"
#include "error.h"
#include "thread.h"

/* the following are legal, implemented events that user-space can watch for */
#define IN_ACCESS   0x00000001  /* File was accessed */
#define IN_MODIFY   0x00000002  /* File was modified */
#define IN_ATTRIB   0x00000004  /* Metadata changed */
#define IN_CLOSE_WRITE    0x00000008  /* Writtable file was closed */
#define IN_CLOSE_NOWRITE  0x00000010  /* Unwrittable file closed */
#define IN_OPEN     0x00000020  /* File was opened */
#define IN_MOVED_FROM   0x00000040  /* File was moved from X */
#define IN_MOVED_TO   0x00000080  /* File was moved to Y */
#define IN_CREATE   0x00000100  /* Subfile was created */
#define IN_DELETE   0x00000200  /* Subfile was deleted */
#define IN_DELETE_SELF    0x00000400  /* Self was deleted */

/* the following are legal events.  they are sent as needed to any watch
 * */
#define IN_UNMOUNT    0x00002000  /* Backing fs was unmounted */
#define IN_Q_OVERFLOW   0x00004000  /* Event queued overflowed */
#define IN_IGNORED    0x00008000  /* File was ignored */

/* helper events */
#define IN_CLOSE    (IN_CLOSE_WRITE | IN_CLOSE_NOWRITE) /* close */
#define IN_MOVE     (IN_MOVED_FROM | IN_MOVED_TO) /* moves */

/* special flags */
#define IN_ISDIR    0x40000000  /* event occurred against dir */
#define IN_ONESHOT    0x80000000  /* only send event once */

pthread_t galaxy_reciever, signal_thread;
static sigset_t mask;

void
print_mask(int mask)
{
  if (mask & IN_ACCESS)
    fprintf(stderr, "ACCESS ");
  if (mask & IN_MODIFY)
    fprintf(stderr, "MODIFY ");
  if (mask & IN_ATTRIB)
    fprintf(stderr, "ATTRIB ");
  if (mask & IN_CLOSE)
    fprintf(stderr, "CLOSE ");
  if (mask & IN_OPEN)
    fprintf(stderr, "OPEN ");
  if (mask & IN_MOVED_FROM)
    fprintf(stderr, "MOVE_FROM ");
  if (mask & IN_MOVED_TO)
    fprintf(stderr, "MOVE_TO ");
  if (mask & IN_DELETE)
    fprintf(stderr, "DELETE ");
  if (mask & IN_CREATE)
    fprintf(stderr, "CREATE ");
  if (mask & IN_DELETE_SELF)
    fprintf(stderr, "DELETE_SELF ");
  if (mask & IN_UNMOUNT)
    fprintf(stderr, "UNMOUNT ");
  if (mask & IN_Q_OVERFLOW)
    fprintf(stderr, "Q_OVERFLOW ");
  if (mask & IN_IGNORED)
    fprintf(stderr, "IGNORED " );

  if (mask & IN_ISDIR)
    fprintf(stderr, "(dir) ");
  else
    fprintf(stderr, "(file) ");

  fprintf(stderr, "0x%08x\n", mask);
}

static void *
signal_handler(void *arg)
{
	int err, signo;

	while (1) {
		err = sigwait(&mask, &signo);

		switch (signo) {
			case SIGINT:
				fprintf(stderr, "DEBUG: SIGINT caught\n");
				pthread_cancel(galaxy_reciever);
				return 0;
				break;
			case SIGQUIT:
				fprintf(stderr, "DEBUG: SIGQUIT caught\n");
				break;
			default:
				fprintf(stderr, "warning: unexpected signal %d\n", signo);
				break;
		}
	}
}

static int
create_signal_thread(pthread_t *id)
{
	int err;
	sigset_t oldmask;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGQUIT);

	err = pthread_sigmask(SIG_BLOCK, &mask, &oldmask);
	if (err != 0) {
		err_pthread_sigmask(errno);
		return err;
	}

	err = create_joinable_thread(id, signal_handler, 0);
	if (err < 0) {
		err_create_joinable_thread(errno);
		err_msg("error[create_signal_thread]: Unable to create signal thread.");
		return err;
	}

	return err;
}

static void *
receive_notifications(void *arg)
{
	struct galaxy_t *galaxy;
	struct galaxy_event_t *gevent;

	galaxy = (struct galaxy_t *)arg;

	while (1) {
		err_msg("Receiving galaxy event...");
		gevent = galaxy_receive(galaxy);
		if (gevent == NULL) {
			err_msg("warning[receive_notifications]: gevent is NULL!");
		}

		err_msg("gevent->mask = %d gevent->name = %s gvent->timestamp = %d\n",
			gevent->mask, gevent->name, gevent->timestamp);
		print_mask(gevent->mask);
	}

	return NULL;
}

void
usage(FILE *iostream)
{
	fprintf(iostream, "Usage: %s [-h] [-v] [-r] [-p PRUNE_LIST] [DIRECTORY]\n");
	fprintf(iostream, "  -h              Displays this information.\n");
	fprintf(iostream, "  -p PRUNE_LIST   Prune the colon-separated directories from the galaxy\n");
	fprintf(iostream, "                  search path.\n");
	fprintf(iostream, "  -r              Recursively add Galaxy watches.\n");
	fprintf(iostream, "  -v              Output version information and exit.\n");
}

int main(int argc, char **argv)
{
	int c, help, version, recursive, err, i;
	struct galaxy_t galaxy;
	struct galaxy_event_t *gevent;

	help = version = recursive = err = 0;
	while ((c = getopt(argc, argv, "hp:rv")) != -1) {
		switch (c) {
			case 'h':
				usage(stdout);
				exit(0);
				break;
			case 'p':
				break;
			case 'r':
				recursive = 1;
				break;
			case 'v':
				version = 1;
				break;
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

	printf("argc = %d\n", argc);
	if (argc <= 1) {
		err_msg("Usage: %s <Regular expression to match file with>", argv[0]);
		exit(-1);
	}

	err = galaxy_connect(&galaxy);
	if (err < 0) {
		err_msg("error: Unable to connect to server.");
		err_msg("err = %d", err);
		return 1;
	}

	/* galaxy_watch (galaxy_t, regexp to match, Inotify mask) */
	//galaxy_ignore_mask(&galaxy, IN_OPEN);
/*
	galaxy_ignore_watch(&galaxy, IN_ALL_EVENTS, "^/proc");
	galaxy_ignore_watch(&galaxy, IN_ALL_EVENTS, "^/sys");
	galaxy_ignore_watch(&galaxy, IN_ALL_EVENTS, "^/media");
	galaxy_ignore_watch(&galaxy, IN_ALL_EVENTS, "^/tmp");
	galaxy_ignore_watch(&galaxy, IN_ALL_EVENTS, "^/root");
	galaxy_ignore_watch(&galaxy, IN_ALL_EVENTS, "^/home");
	galaxy_ignore_watch(&galaxy, IN_ALL_EVENTS, "^/mnt");
	galaxy_ignore_watch(&galaxy, IN_ALL_EVENTS, "^/dev");
	galaxy_ignore_watch(&galaxy, IN_ALL_EVENTS, "^/var");
	galaxy_ignore_watch(&galaxy, IN_ALL_EVENTS, "^/usr/src");
	galaxy_ignore_watch(&galaxy, IN_ALL_EVENTS, "^/usr/tmp");
	galaxy_ignore_watch(&galaxy, IN_ALL_EVENTS, "^/usr/var");
	galaxy_ignore_watch(&galaxy, IN_ALL_EVENTS, "^/usr/misc");
	galaxy_ignore_watch(&galaxy, IN_ALL_EVENTS, "^/sources");
	galaxy_ignore_watch(&galaxy, IN_ALL_EVENTS, "^/tools");
*/
	galaxy_watch(&galaxy, IN_CREATE | IN_DELETE | IN_MODIFY, argv[1]);
	galaxy_watch(&galaxy, IN_ALL_EVENTS, ".*");
	//galaxy_ignore_watch(&galaxy, IN_CLOSE, argv[1]);

	create_signal_thread(&signal_thread);

	//err = pthread_create(&galaxy_reciever, NULL, receive_notifications, &galaxy);
	err = create_joinable_thread(&galaxy_reciever, receive_notifications, &galaxy);
	if (err < 0) {
		err_create_joinable_thread(errno);
		err_msg("error: Unable to create thread.");
		galaxy_close(&galaxy);
		return 1;
	}

	pthread_join(galaxy_reciever, NULL);
	pthread_join(signal_thread, NULL);

	galaxy_close(&galaxy);

	return 0;
}
