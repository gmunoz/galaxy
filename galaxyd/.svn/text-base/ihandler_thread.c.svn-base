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

#if HAVE_STRING_H
#  include <string.h>
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

#if HAVE_PCRE_H
#  include <pcre.h>
#endif

#include "ihandler_thread.h"
#include "thread.h"
#include "watch.h"
#include "galaxy.h"
#include "notifier.h"
#include "list.h"
#include "error.h"

extern pthread_mutex_t inotify_wds_mutex;
extern GHashTable *inotify_wds;

/*
 * Handles the following internal events:
 *   - Adding a new directory
 *   - Removing an existing watch directory
 *   - Unmounting of a directory
 *
 * An internal event is one that is meaningful to the galaxy daemon,
 * and not necessary information for any clients that may be interested
 * in the file/directory that this event has occured on. It is required
 * info to maintain the state of the watch directories.
 *
 * Return Value:
 *   Return -1 on error, otherwise zero on success.
 */
static int
handle_internal_actions(const struct inotify_event *event, const char *filename)
{
#ifdef DEBUG_HANDLE_INTERNAL_ACTIONS
	err_msg("  => DEBUG[handle_internal_actions]: Checking if internal event...\n");
#endif
	if (event->mask & IN_CREATE && event->mask & IN_ISDIR) {
		/* Event: Create a directory. */
		int wd;
#ifdef DEBUG_HANDLE_INTERNAL_ACTIONS
		err_msg("     + Create a new directory event detected.\n");
#endif
		wd = galaxy_add_watch(filename, IN_ALL_EVENTS);
		if (wd < 0) {
			err_msg("error[handle_internal_actions]: Unable to add watch event for directory '%s'\n", filename);
			return -1;
		}
	} else if (event->mask & IN_DELETE_SELF && event->mask & IN_ISDIR) {
		/* Event: Delete a directory. */
		int err;
#ifdef DEBUG_HANDLE_INTERNAL_ACTIONS
		err_msg("     + Delete a directory event detected.\n");
#endif
		err = galaxy_remove_watch(event->wd);
		if (err < 0) {
			err_msg("error[handle_internal_actions]: Unable to remove watch event for directory '%s'\n", filename);
			return -1;
		}
	} else if (event->mask & IN_UNMOUNT) {
		/* Event: Unmount a directory. */
#ifdef DEBUG_HANDLE_INTERNAL_ACTIONS
		err_msg("     + Unmount a directory event detected.\n");
#endif
	} else if (event->mask & IN_Q_OVERFLOW) {
#ifdef DEBUG_HANDLE_INTERNAL_ACTIONS
		err_msg("     + Kernel inotify queue overflowed event detected.\n");
#endif
	} else if (event->mask & IN_IGNORED) {
#ifdef DEBUG_HANDLE_INTERNAL_ACTIONS
		err_msg("     + Kernel inotify event ignored event detected.\n");
#endif
	} else {
#ifdef DEBUG_HANDLE_INTERNAL_ACTIONS
		err_msg("     + Not a recognized internal event. No action will be taken.\n");
#endif
	}

	return 0;
}

/*
 * Handles an inotify event. Each registered application with a watch
 * that is evaluated as a match when compared to the event that is
 * getting handled here will be sent a message with the relevant data
 * from this event (it may depend on the state information kept when the
 * watch is created by the client application).
 *
 * There may also be internal actions triggered from this handler. They
 * include things that will need to occur to maintain the state of the
 * galaxy watch list, such as:
 *   - Adding a new directory
 *   - Removing an existing watch directory
 *   - Unmounting of a directory
 */
static void *
ihandler_thread(void *arg)
{
	struct inotify_event *event;
	char *dirname;
	int err;
	
	event = (struct inotify_event *)arg;

#ifdef DEBUG_IHANDLER_THREAD
	err_msg("DEBUG[ihandler_thread]: Inotify event handler\n");
	err_msg("  + Event watch descriptor = %d\n", event->wd);
	err_msg("  + Event mask = 0x%x\n", event->mask);
	print_mask(event->mask);
#endif

	pthread_mutex_lock(&inotify_wds_mutex);
	dirname = g_hash_table_lookup(inotify_wds, &(event->wd));
	pthread_mutex_unlock(&inotify_wds_mutex);

	/* Check for NULL'ness. Shouldn't happend. Abort if it occurs. */
	if (dirname == NULL) {
		err_msg("error[ihandler_thread]: Hash table lookup returned NULL.\n");
		return NULL;
	}
#ifdef DEBUG_IHANDLER_THREAD
	err_msg("  + dirname = %s\n", dirname);
#endif

	/* Append dirname + '/' + the event filename (if it exists). */
	char filename[strlen(dirname) + event->len + 2];
	filename[0] = '\0';
	strcat(filename, dirname);
	if (event->len) {
		if (filename[strlen(filename) - 1] != '/')
			strcat(filename, "/");
		strcat(filename, event->name);
	}

#ifdef DEBUG_IHANDLER_THREAD
	err_msg("  + filename = %s\n", filename);
#endif

	/* Handle any internal actions for this event. */
	err = handle_internal_actions(event, filename);
	if (err < 0)
		err_msg("warning[ihandler_thread]: Unable to handle internal actions.\n");

	/* Search list of galaxy watches for matching event(s). */
	find_matching_events(filename, event->mask);

	free(event);

	return NULL;
}

/*
 * Returns the value of pthread_create(3).
 */
int
create_ihandler_thread(struct inotify_event *event)
{
	int err;
	pthread_t handler;

	err = create_detached_thread(&handler, ihandler_thread, event);
	if (err < 0)
		err_create_detached_thread(errno);

	return err;
}
