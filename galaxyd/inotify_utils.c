/*
 * Galaxy - A filesystem monitoring tool.
 * Copyright (C) 2005  Gabriel Munoz <gabriel@xusia.net>
 *
 * Based on inotify-utils by Robert Love:
 *   http://www.kernel.org/pub/linux/kernel/people/rml/inotify/utils/
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

#if HAVE_SYS_SELECT_H
#  include <sys/select.h>
#endif

#if HAVE_PTHREAD_H
#  include <pthread.h>
#endif

#if HAVE_ERRNO_H
#  include <errno.h>
#endif

#if HAVE_LIBGLIB_2_0
#  include <glib.h>
#endif

#include "inotify.h"
#include "inotify-syscalls.h"
#include "inotify_utils.h"

#include "event_queue.h"
#include "ihandler_thread.h"
#include "error.h"

#define ALL_MASK 0xffffffff
#define EVENTQ_SIZE 128

extern pthread_mutex_t inotify_wds_mutex;
extern GHashTable *inotify_wds;

struct inotify_event *eventq[EVENTQ_SIZE];
int eventq_head = 0;
int eventq_tail = 0;

static int inotify_fd = -1;

int
open_dev(void)
{
	inotify_fd = inotify_init ();
	if (inotify_fd < 0)
		err_inotify_init(errno);

	return inotify_fd;
}

int close_dev (int fd)
{
	int r;

	if ( (r = close (fd)) < 0)
		err_close(errno);

	return r;
}


void
print_mask(int mask)
{
	fprintf(stderr, "  + Mask names = ");
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

	//fprintf(stderr, "0x%08x\n", mask);
	fprintf(stderr, "\n");
}

void
print_event(struct inotify_event *event)
{
	char *path;

	pthread_mutex_lock(&inotify_wds_mutex);
	path = g_hash_table_lookup(inotify_wds, &(event->wd));
	pthread_mutex_unlock(&inotify_wds_mutex);

	fprintf(stderr, "event[%d]", event->wd);
	if (event->len)
		fprintf(stderr, ": '%s/%s'", path, event->name);
	fprintf(stderr, " => ");
	print_mask (event->mask);
}

void
print_events(queue_t q)
{
	struct inotify_event *event;
	while (!queue_empty (q))
	{
		event = queue_front (q);
		queue_dequeue (q);
		print_event (event);
		free (event);
	}
}

int
read_events(queue_t q, int fd)
{
	char buffer[16384];
	size_t buffer_i;
	struct inotify_event *pevent, *event;
	ssize_t r;
	size_t event_size;
	int count = 0;

#ifdef DEBUG_READ_EVENTS
	err_msg("DEBUG[read_events]: Reading some inotify events...\n");
#endif
	r = read (fd, buffer, 16384);
	if (r <= 0) {
		err_read(errno);
		fprintf(stderr, "read(fd, buffer, 16384) = ");
		return r;
	}

#ifdef DEBUG_READ_EVENTS
	err_msg("  + read %d bytes\n", r);
	err_msg("  + sizeof(inotify_event) = %d\n", sizeof(struct inotify_event));
	err_msg("  => Parsing inotify events and queuing them...\n");
#endif

	buffer_i = 0;
	while (buffer_i < r) {
		/* Parse events and queue them ! */
#ifdef DEBUG_READ_EVENTS
		err_msg("     => Event #%d\n", count);
#endif
		pevent = (struct inotify_event *)&buffer[buffer_i];
		event_size = sizeof(struct inotify_event) + pevent->len;
		event = malloc(event_size);
		memmove(event, pevent, event_size);
#ifdef DEBUG_READ_EVENTS
		err_msg("        + Inotify watch descriptor = %d\n", event->wd);
#endif
		queue_enqueue(event, q);
		buffer_i += event_size;
		count++;
	}
#ifdef DEBUG_READ_EVENTS
	err_msg("  + Total number of events read = %d\n", count);
#endif

	return count;
}

int
event_check(int fd)
{
	struct timeval timeout;
	int r;
	fd_set rfds;

	timeout.tv_sec = 4;
	timeout.tv_usec = 0;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	r = select (fd+1, &rfds, NULL, NULL, &timeout);

	return r;
}

int
read_and_print_events(queue_t q, int fd)
{
	while (1)
	{
		if (!queue_empty(q))
			print_events (q);

		if (event_check(fd) > 0) {
			int r;

			r = read_events (q, fd);

			if (r < 0)
				break;
		}
	}

	return 0;
}

/*
 * Adds a directory name into the inotify watch list.
 *
 * Return Value:
 *   Returns -1 on error. On success, it will return a new file
 *   descriptor for new inotify watch.
 *
 * See Also:
 *   inotify_add_watch(2)
 */
int
galaxy_add_watch(const char *dirname, uint32_t mask)
{
	int *wd;

	assert(dirname);

	wd = malloc(sizeof(int));
	if (wd == NULL) {
		err_malloc(errno);
		err_msg("error[galaxy_add_watch]: Unable to malloc an int.\n");
		return -1;
	}

	*wd = inotify_add_watch(inotify_fd, dirname, mask);
	if (*wd < 0) {
		err_inotify_add_watch(errno);
		err_msg("error[galaxy_add_watch]: Unable to add inotify watch for '%s'\n",
			dirname);
		free(wd);
		return -1;
	}

	pthread_mutex_lock(&inotify_wds_mutex);
	g_hash_table_insert(inotify_wds, wd, g_strdup(dirname));
	pthread_mutex_unlock(&inotify_wds_mutex);

	return *wd;
}

/*
 * Obsolete in favor of galaxy_add_watch().
 */
static int
watch_dir(int fd, const char *dirname, uint32_t mask)
{
	int wd;

	wd = inotify_add_watch (fd, dirname, mask);
	if (wd < 0)
		err_inotify_add_watch(errno);

	fprintf(stderr, "  + watching: %s\n", dirname);

	return wd;
}

/*
 * Removes an existing watch from an inotify instance.
 *
 * Return Value:
 *   Returns -1 on error, or zero on success.
 *
 * See Also:
 *   inotify_rm_watch(2)
 */
int
galaxy_remove_watch(__u32 wd)
{
	int ret;

	ret = inotify_rm_watch(inotify_fd, wd);
	if (ret < 0) {
		err_inotify_rm_watch(errno);
		err_msg("error[galaxy_remove_watch]: Unable to remove watch for watch #%d\n",
			wd);
		return -1;
	}

	return ret;
}

/*
 * Obsolete in favor of galaxy_remove_watch
 */
static int
ignore_wd(int fd, int wd)
{
	int ret;

	ret = inotify_rm_watch (fd, wd);

	if (ret < 0)
		err_inotify_rm_watch(errno);

	return ret;
}

void
err_inotify_init(int err)
{
	err_msg("error: inotify_init(2) failed.\n");
	switch (err) {
		case ENFILE:
			err_msg("       The system limit on the total number of file descriptors has\n");
			err_msg("       been reached.\n");
			break;
		case EMFILE:
			err_msg("       The user limit on the total number of inotify instances has been\n");
			err_msg("       reached.\n");
			break;
		case ENOMEM:
			err_msg("       Insufficient kernel memory is available.\n");
			break;
	}
}

void
err_inotify_add_watch(int err)
{
	err_msg("error: inotify_add_watch(2) failed.\n");
	switch (err) {
		case EBADF:
			err_msg("       The given file descriptor is not valid.\n");
			break;
		case EINVAL:
			err_msg("       The given event mask contains no legal events.\n");
			break;
		case ENOMEM:
			err_msg("       Insufficient kernel memory was available.\n");
			break;
		case ENOSPC:
			err_msg("       The user limit on the total number of inotify watches was\n");
			err_msg("       reached or the kernel failed to allocate a needed  resource.\n");
			err_msg("       Try increasing `/proc/sys/fs/inotify/max_user_watches'.\n");
			break;
		case EACCES:
			err_msg("       Read access to the given file is not permitted.\n");
			break;
		case EFAULT:
			err_msg("       `path' points outside of the process's accessible address space.\n");
			break;
	}
}

void
err_inotify_rm_watch(int err)
{
	err_msg("error: inotify_rm_watch(2) failed.\n");
	switch (err) {
		case EBADF:
			err_msg("       `fd' is not a valid file descriptor.\n");
			break;
		case EMFILE:
			err_msg("       The user limit on the total number of inotify instances has been\n");
			err_msg("       reached.\n");
			break;
		case EINVAL:
			err_msg("       The watch descriptor wd is not valid.\n");
			break;
	}
}
