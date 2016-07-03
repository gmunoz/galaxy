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
#  include <stdlib.h>
#endif

#if HAVE_PTHREAD_H
#  include <pthread.h>
#endif

#if HAVE_ERRNO_H
#  include <errno.h>
#endif

#include "iwatch_thread.h"
#include "thread.h"
#include "ihandler_thread.h"
#include "inotify_utils.h"
#include "event_queue.h"
#include "error.h"

struct watch_data_t {
	int fd;
	queue_t q;
} watch_data_t;

static void *
watch(void *arg)
{
	int fd, err;
	queue_t q;
	struct inotify_event *event;

	fd = ((struct watch_data_t *)arg)->fd;
	q = ((struct watch_data_t *)arg)->q;
	free(arg);

	while (1) {
		while (!queue_empty(q)) {
			event = queue_front(q);
			queue_dequeue(q);
			err = create_ihandler_thread(event);
			if (err < 0)
				err_msg("warning[watch]: Unable to spawn inotify event handler for event->wd #%d\n", event->wd);
		}

		if (event_check(fd) > 0) {
			int r;
			r = read_events(q, fd);
			if (r < 0)  /* read(2) in read_events() returned an error. */
				break;
		}
	}

	return NULL;
}

int
create_iwatch_thread(pthread_t *id, int fd, queue_t q)
{
	int err;
	struct watch_data_t *wdata;

	wdata = malloc(sizeof(struct watch_data_t));
	if (wdata == NULL) {
		err_malloc(errno);
		err_msg("error[create_iwatch_thread]: Unable to malloc watch data.\n");
		return -1;
	}

	wdata->fd = fd;
	wdata->q = q;

	err = create_joinable_thread(id, watch, wdata);
	if (err < 0)
		err_create_joinable_thread(errno);

	return err;
}
