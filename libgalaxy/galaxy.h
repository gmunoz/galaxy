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

#ifndef LIBGALAXY_H
#define LIBGALAXY_H

#include <inttypes.h>
#include <time.h>
#include <sys/inotify.h>

#define GALAXY_MAJOR     0
#define GALAXY_MINOR     1
#define GALAXY_RELEASE   0

#define GALAXY_SOCKET       "/tmp/galaxy.socket"

#define galaxy_cmd_t         uint32_t
#define GALAXY_COMMAND_LEN   (sizeof(galaxy_cmd_t))

/* Galaxy server commands. */
#define GALAXY_WATCH         1
#define GALAXY_IGNORE_MASK   2
#define GALAXY_IGNORE_WATCH  3
#define GALAXY_EXIT          4

#define ACK_LENGTH       4
#define ACK_SUCCESS      1
#define ACK_FAIL         2

/* the following are legal, implemented events that user-space can watch for */
#define GAL_ACCESS   0x00000001  /* File was accessed */
#define GAL_MODIFY   0x00000002  /* File was modified */
#define GAL_ATTRIB   0x00000004  /* Metadata changed */
#define GAL_CLOSE_WRITE    0x00000008  /* Writtable file was closed */
#define GAL_CLOSE_NOWRITE  0x00000010  /* Unwrittable file closed */
#define GAL_OPEN     0x00000020  /* File was opened */
#define GAL_MOVED_FROM   0x00000040  /* File was moved from X */
#define GAL_MOVED_TO   0x00000080  /* File was moved to Y */
#define GAL_CREATE   0x00000100  /* Subfile was created */
#define GAL_DELETE   0x00000200  /* Subfile was deleted */
#define GAL_DELETE_SELF    0x00000400  /* Self was deleted */

/* the following are legal events.  they are sent as needed to any watch */
#define GAL_UNMOUNT    0x00002000  /* Backing fs was unmounted */
#define GAL_Q_OVERFLOW   0x00004000  /* Event queued overflowed */
#define GAL_IGNORED    0x00008000  /* File was ignored */

/* helper events */
#define GAL_CLOSE    (GAL_CLOSE_WRITE | GAL_CLOSE_NOWRITE) /* close */
#define GAL_MOVE     (GAL_MOVED_FROM | GAL_MOVED_TO) /* moves */

/* special flags */
#define GAL_ISDIR    0x40000000  /* event occurred against dir */
#define GAL_ONESHOT    0x80000000  /* only send event once */

/*
 * All of the events - we build the list by hand so that we can add flags in
 * the future and not break backward compatibility.  Apps will get only the
 * events that they originally wanted.  Be sure to add new events here!
 */
#define GAL_ALL_EVENTS (GAL_ACCESS | GAL_MODIFY | GAL_ATTRIB | \
		GAL_CLOSE_WRITE | GAL_CLOSE_NOWRITE | GAL_OPEN | GAL_MOVED_FROM | \
		GAL_MOVED_TO | GAL_DELETE | GAL_CREATE | GAL_DELETE_SELF)

struct galaxy_t {
	int fd;            /* Client side server end-point. */
	char sname[4096];  /* Socket name. */
};

struct galaxy_event_t {
	uint32_t mask;
	time_t timestamp;
	char *name;
};

/* Galaxy event creation/destroy functions. */
struct galaxy_event_t *create_galaxy_event(void);
void destroy_galaxy_event(void *ptr);

/* Init and destroy functions. */
int galaxy_connect(struct galaxy_t *galaxy);
int galaxy_close(struct galaxy_t *galaxy);

/* Inotify-related functions. Watch for file(s) and receive events when
 * those files are triggered by inotify. */
int galaxy_send_server_command(const struct galaxy_t *galaxy,
	galaxy_cmd_t command, uint32_t mask, const char *regexp);
struct galaxy_event_t *galaxy_receive(struct galaxy_t *galaxy);

#define galaxy_watch(galaxy, mask, regexp) \
	galaxy_send_server_command(galaxy, GALAXY_WATCH, mask, regexp)
#define galaxy_ignore_mask(galaxy, mask) \
	galaxy_send_server_command(galaxy, GALAXY_IGNORE_MASK, mask, NULL)
#define galaxy_ignore_watch(galaxy, mask, regexp) \
	galaxy_send_server_command(galaxy, GALAXY_IGNORE_WATCH, mask, regexp)

#endif
