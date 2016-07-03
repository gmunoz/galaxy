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

#if HAVE_ERRNO_H
#  include <errno.h>
#endif

#if HAVE_PCRE_H
#  include <pcre.h>
#endif

#if HAVE_PTHREAD_H
#  include <pthread.h>
#endif

#if HAVE_LIBGLIB_2_0
#  include <glib.h>
#endif

#define OVECCOUNT 30    /* Needs to be a multiple of 3 */

#include "watch.h"
#include "list.h"
#include "inotify.h"
#include "error.h"

static GHashTable *client_watches;
static pthread_mutex_t client_watches_mutex = PTHREAD_MUTEX_INITIALIZER;

struct watch_t {
	uint32_t mask;
	pcre *re;
	pcre_extra *extra;
} watch_t;

struct client_watch_t {
	uint32_t ignore_mask;
	list_t *ignore_watches;
	list_t *watches;
} client_watch_t;

struct internal_event_t {
	uint32_t mask;
	const char *filename;
} internal_event_t;

/*
 * Used to de-allocate a struct watch_t structure. May be used as the
 * destroy function when a list (or any other ADT) is created for these
 * structures.
 */
void
destroy_watch(void *ptr)
{
	struct watch_t *watch;

	watch = (struct watch_t *)ptr;
	if (watch->re != NULL)
		free(watch->re);
	if (watch->extra != NULL)
		free(watch->extra);
	free(watch);
}

/*
 * Creates a new watch structure, and fill its contents with the PCRE.
 */
struct watch_t *
create_watch(uint32_t mask, const char *pattern)
{
	struct watch_t *watch;
	const char *error;
	int erroffset;

	watch = malloc(sizeof(struct watch_t));
	if (watch == NULL) {
		err_malloc(errno);
		err_msg("error[create_watch]: Unable to malloc memory for a watch_t.\n");
		return NULL;
	}

	watch->mask = mask;

	/* Convert regexp to a PCRE. */
	watch->re = pcre_compile(pattern, 0, &error, &erroffset, NULL);
	if (watch->re == NULL) {
		err_msg("error[create_watch]: %s\n", error);
		err_msg("      Unable to make PCRE '%s'.\n", pattern);
		free(watch);
		return NULL;
	}

	watch->extra = pcre_study(watch->re, 0, &error);  /* Ignore retval. */

	return watch;
}

void
destroy_client_watch(void *ptr)
{
	struct client_watch_t *client_watch;

	client_watch = (struct client_watch_t *)ptr;
	list_destroy(client_watch->ignore_watches);
	list_destroy(client_watch->watches);
}

struct client_watch_t *
create_client_watch(void)
{
	struct client_watch_t *client_watch;

	client_watch = malloc(sizeof(struct client_watch_t));
	if (client_watch == NULL) {
		err_malloc(errno);
		err_msg("error[create_client_watch]: Unable to malloc client_watch_t.\n");
		return NULL;
	}
	client_watch->ignore_mask = 0;
	client_watch->ignore_watches = list_create(destroy_watch);
	if (client_watch->ignore_watches == NULL) {
		err_msg("error[create_client_watch]: Unable to create ignore_watches list.\n");
		free(client_watch);
		return NULL;
	}
	client_watch->watches = list_create(destroy_watch);
	if (client_watch->watches == NULL) {
		err_msg("error[create_client_watch]: Unable to create watches list.\n");
		list_destroy(client_watch->ignore_watches);
		free(client_watch);
		return NULL;
	}

	return client_watch;
}

/*
 * Initialize the client watches list.
 */
int
init_client_watches_container(void)
{
	int err = 0;

	pthread_mutex_init(&client_watches_mutex, NULL);
	client_watches = g_hash_table_new_full(g_str_hash, g_str_equal, free,
		destroy_client_watch);
	if (client_watches == NULL)
		err = -1;

	return err;
}

static void
destroy_watches(gpointer key, gpointer value, gpointer user_data)
{
	free(key);
	destroy_client_watch(value);
}

/*
 * Destroy's the watches container and all of its internal elements.
 * This will de-allocate all allocated memory from the
 * init_client_watches_container() routine and any add_* functions that
 * were called on this container.
 *
 * This function should only be used once, at the final tear-down of the
 * entire application that is using the watches container. A single
 * instance is kept for the instance of the program that is using it.
 */
void
destroy_client_watches_container(void)
{
	g_hash_table_foreach(client_watches, destroy_watches, NULL);
	g_hash_table_destroy(client_watches);
}

/*
 * Looks for an existing clent watch entry for the given server name
 * key. If no entry is found, a new client watch is created and inserted
 * into the hash table.
 *
 * Think of this function as a single design pattern around the client
 * watch structure for a client. There will be at most one client watch
 * structure for a single client. This function makes sure of that.
 *
 * Return Value:
 *   A pointer to the client_watch_t structure that was found in the
 *   hash table lookup, or that was created and inserted into the hash
 *   table.
 */
static struct client_watch_t *
get_client_watch(GHashTable *client_watches, char *client_name)
{
	struct client_watch_t *client_watch;

	/* Look for an existing entry using the server name as the key. */
	pthread_mutex_lock(&client_watches_mutex);
	client_watch = g_hash_table_lookup(client_watches, client_name);
	pthread_mutex_unlock(&client_watches_mutex);

	/* If no server name entry exists, create a new client_watch_t and
	 * insert into the client_watches hash table. */
	if (client_watch == NULL) {
		client_watch = create_client_watch();
		if (client_watch == NULL) {
			err_msg("error[get_client_watch]: Unable to create client watch.\n");
			return NULL;
		}
		pthread_mutex_lock(&client_watches_mutex);
		g_hash_table_insert(client_watches, client_name, client_watch);
		pthread_mutex_unlock(&client_watches_mutex);
	}

	return client_watch;
}

int
set_galaxy_ignore_mask(const char *client_name, uint32_t ignore_mask)
{
	struct client_watch_t *client_watch;

	/* Look for an existing entry using the server name as the key. */
	client_watch = get_client_watch(client_watches, (char *)client_name);
	if (client_watch == NULL) {
		err_msg("error[set_galaxy_ignore_mask]: Unable to lookup client watch.\n");
		return -1;
	}

	pthread_mutex_lock(&client_watches_mutex);
	client_watch->ignore_mask = ignore_mask;
	pthread_mutex_unlock(&client_watches_mutex);

	return 0;
}

/*
 * Converts the given pattern into a PCRE and stores it in a global list
 * of galaxy watches.
 *
 * Return Value:
 *   Returns -1 if this function was unable to lookup a client watch
 *   structure. Returns -2 if this function was unable to create a new
 *   watch structure.
 */
int
add_galaxy_watch(const char *client_name, uint32_t mask, const char *pattern)
{
	struct client_watch_t *client_watch;
	struct watch_t *watch;

	/* Look for an existing entry using the server name as the key. */
	client_watch = get_client_watch(client_watches, (char *)client_name);
	if (client_watch == NULL) {
		err_msg("error[add_galaxy_watch]: Unable to lookup client watch.\n");
		return -1;
	}

	/* Add a new watch_t into the watches list of the client watch. */
	watch = create_watch(mask, pattern);
	if (watch == NULL) {
		err_msg("error[add_galaxy_watch]: Unable to create a watch structure.\n");
		return -2;
	}
	pthread_mutex_lock(&client_watches_mutex);
	list_push(client_watch->watches, watch);
	pthread_mutex_unlock(&client_watches_mutex);

	return 0;
}

int
add_galaxy_ignore_watch(const char *client_name, uint32_t mask,
	const char *pattern)
{
	struct client_watch_t *client_watch;
	struct watch_t *watch;

	/* Look for an existing entry using the server name as the key. */
	client_watch = get_client_watch(client_watches, (char *)client_name);
	if (client_watch == NULL) {
		err_msg("error[add_galaxy_ignore_watch]: Unable to lookup client watch.\n");
		return -1;
	}

	/* Add a new watch_t into the watches list of the client watch. */
	watch = create_watch(mask, pattern);
	if (watch == NULL) {
		err_msg("error[add_galaxy_ignore_watch]: Unable to create a watch structure.\n");
		return -2;
	}
	pthread_mutex_lock(&client_watches_mutex);
	list_push(client_watch->ignore_watches, watch);
	pthread_mutex_unlock(&client_watches_mutex);

	return 0;
}

static void
send_notifications(gpointer key, gpointer value, gpointer user_data)
{
	int mask;
	char *client_name;
	struct client_watch_t *client_watch;
	struct internal_event_t *ievent;

	client_name = (char *)key;
	client_watch = (struct client_watch_t *)value;
	ievent = (struct internal_event_t *)user_data;
	list_node_t *node;

#ifdef DEBUG_SEND_NOTIFICATIONS
	err_msg("     => DEBUG[send_notifications]: Searching client '%s'...\n", client_name);
#endif

	/* Ignore this event if any single inotify event is marked to be
	 * ignored by the clients ignore mask. */
#ifdef DEBUG_SEND_NOTIFICATIONS
	err_msg("        => Check client ignore mask...\n");
#endif
	if ((ievent->mask & IN_ACCESS && client_watch->ignore_mask & IN_ACCESS) ||
			(ievent->mask & IN_MODIFY && client_watch->ignore_mask & IN_MODIFY) ||
			(ievent->mask & IN_ATTRIB && client_watch->ignore_mask & IN_ATTRIB) ||
			(ievent->mask & IN_CLOSE_WRITE && client_watch->ignore_mask & IN_CLOSE_WRITE) ||
			(ievent->mask & IN_CLOSE_NOWRITE && client_watch->ignore_mask & IN_CLOSE_NOWRITE) ||
			(ievent->mask & IN_OPEN && client_watch->ignore_mask & IN_OPEN) ||
			(ievent->mask & IN_MOVED_FROM && client_watch->ignore_mask & IN_MOVED_FROM) ||
			(ievent->mask & IN_MOVED_TO && client_watch->ignore_mask & IN_MOVED_TO) ||
			(ievent->mask & IN_CREATE && client_watch->ignore_mask & IN_CREATE) ||
			(ievent->mask & IN_DELETE && client_watch->ignore_mask & IN_DELETE) ||
			(ievent->mask & IN_DELETE_SELF && client_watch->ignore_mask & IN_DELETE_SELF) ||
			(ievent->mask & IN_UNMOUNT && client_watch->ignore_mask & IN_UNMOUNT) ||
			(ievent->mask & IN_Q_OVERFLOW && client_watch->ignore_mask & IN_Q_OVERFLOW) ||
			(ievent->mask & IN_IGNORED && client_watch->ignore_mask & IN_IGNORED)) {
#ifdef DEBUG_SEND_NOTIFICATIONS
		err_msg("        + Ignoring event based on ignore mask: 0x%x\n",
			client_watch->ignore_mask);
#endif
		return;
	}

	int ovector[OVECCOUNT];

	/* Check the clients ignore watches for any matches. If any matches
	 * are found, then no notification will be sent to the client for
	 * this event. */
#ifdef DEBUG_SEND_NOTIFICATIONS
	err_msg("        => Check client ignore list...\n");
#endif
	node = NULL;
	list_foreach(client_watch->ignore_watches, node) {
		int err;
		struct watch_t * w = (struct watch_t *)list_key(node);
		err = pcre_exec(w->re, w->extra, ievent->filename, 28, 0, 0,
			ovector, OVECCOUNT);
		if (err >= 0) {
#ifdef DEBUG_SEND_NOTIFICATIONS
			err_msg("           + Ignoring event based on an ignore watches regexp.\n");
#endif
			return;
		}
	}

	/* Check the clients watch list for any matches. If any matches are
	 * found, then a notification will be sent to the client for this
	 * event. */
#ifdef DEBUG_SEND_NOTIFICATIONS
	err_msg("        => Check client watch list...\n");
#endif
	node = NULL;
	list_foreach(client_watch->watches, node) {
		int err;
		struct watch_t * w = (struct watch_t *)list_key(node);
		/* Check if any of the mask bits are set for this watch. Only if at
		 * least one of them is set do we attempt to use the regular
		 * expression to match the given filename for this event. */
		if ((ievent->mask & IN_ACCESS && w->mask & IN_ACCESS) ||
				(ievent->mask & IN_MODIFY && w->mask & IN_MODIFY) ||
				(ievent->mask & IN_ATTRIB && w->mask & IN_ATTRIB) ||
				(ievent->mask & IN_CLOSE_WRITE && w->mask & IN_CLOSE_WRITE) ||
				(ievent->mask & IN_CLOSE_NOWRITE && w->mask & IN_CLOSE_NOWRITE) ||
				(ievent->mask & IN_OPEN && w->mask & IN_OPEN) ||
				(ievent->mask & IN_MOVED_FROM && w->mask & IN_MOVED_FROM) ||
				(ievent->mask & IN_MOVED_TO && w->mask & IN_MOVED_TO) ||
				(ievent->mask & IN_CREATE && w->mask & IN_CREATE) ||
				(ievent->mask & IN_DELETE && w->mask & IN_DELETE) ||
				(ievent->mask & IN_DELETE_SELF && w->mask & IN_DELETE_SELF) ||
				(ievent->mask & IN_UNMOUNT && w->mask & IN_UNMOUNT) ||
				(ievent->mask & IN_Q_OVERFLOW && w->mask & IN_Q_OVERFLOW) ||
				(ievent->mask & IN_IGNORED && w->mask & IN_IGNORED)) {
			/* At least one mask has matched. Try to match the regexp. */
#ifdef DEBUG_SEND_NOTIFICATIONS
			err_msg("           + At least one mask has matched.\n");
			err_msg("           => Checking if regexp matches this event filename...\n");
#endif
			int fd;
			err = pcre_exec(w->re, w->extra, ievent->filename, 28, 0, 0,
				ovector, OVECCOUNT);
			if (err >= 0) { /* Is a match! Ignore error or mismatch. */
#ifdef DEBUG_SEND_NOTIFICATIONS
				err_msg("              + Matched a regexp watch to this event!\n");
#endif
				fd = cli_conn(client_name);
				if (fd < 0) {
#ifdef DEBUG_SEND_NOTIFICATIONS
					err_msg("warning[send_notifications]: Unable to obtain client connection to:\n");
					err_msg("       '%s'. No notification(s) will be sent to this client.\n", client_name);
#endif
					return;
				}
#ifdef DEBUG_SEND_NOTIFICATIONS
				err_msg("              => Sending galaxy event to client...\n");
#endif
				net_send_galaxy_event(fd, ievent->filename, ievent->mask);
				close(fd);
#ifdef DEBUG_SEND_NOTIFICATIONS
				err_msg("                 + Finished sending galaxy event to client.\n");
#endif
			}
		}
	}
}

void
find_matching_events(const char *filename, uint32_t mask)
{
	struct internal_event_t ievent;

	ievent.mask = mask;
	ievent.filename = filename;

#ifdef DEBUG_FIND_MATCHING_EVENTS
	err_msg("  => DEBUG[find_matching_events]: Searching for matching events...\n");
#endif
	pthread_mutex_lock(&client_watches_mutex);
	g_hash_table_foreach(client_watches, send_notifications, &ievent);
	pthread_mutex_unlock(&client_watches_mutex);

	return;
}

/*
 * Removes every watch that is associated with the given server name.
 */
int
remove_galaxy_watches(const char *client_name)
{
	/* NOTE: Calling g_hash_table_remove() will automatically call the
	 * destructors for the hash key and hash value that was used to
	 * initialize the hash table since we used g_hash_table_new_full()
	 * function to create this hash table. */
	pthread_mutex_lock(&client_watches_mutex);
	g_hash_table_remove(client_watches, client_name);
	pthread_mutex_unlock(&client_watches_mutex);

	return 0;
}
