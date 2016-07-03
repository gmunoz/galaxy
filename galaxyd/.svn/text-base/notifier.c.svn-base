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

#if HAVE_UNISTD_H
#  include <unistd.h>
#endif

#if HAVE_ERRNO_H
#  include <errno.h>
#endif

#include "notifier.h"
#include "galaxy.h"
#include "watch.h"
#include "galnet.h"
#include "list.h"
#include "error.h"

/*
 * Sends a galaxy event notification to the client machines listed in
 * the list of `matches' that requested a notification on the
 * `filename'.
 *
 * If any notification attempts fail (due to network refusals), then it
 * will be ignored and all remaining matching client matches will be
 * processed.
 *
 * If a single error occurs, an error code will be returned (after all
 * client matches are processed), but the callee will be unable to know
 * which server had failed. The callee will only know that at least one
 * client match had failed to receive its notification.
 *
 * Parameters:
 *   mask: The inotify_event struct mask (specifies the type of event).
 *   filename: The filename that the event occurred on.
 *   matches: A list of the server names that requested a notification
 *     for this event on the given filename.
 *
 * Return Value:
 *   Returns 0 on success.
 *
 * Errors:
 *   NETWORK_ERROR_CLI_CONN: Results when at least one `cli_conn'
 *     function fails.
 */
int
send_notification(uint32_t mask, const char *filename, const list_t *matches)
{
	int fd, err = 0;
	list_node_t *node = NULL;

	/* matches list holds references to items in watches list, so lock. */
/*
	pthread_mutex_lock(&watches_mutex);
	list_foreach(matches, node) {
		char *client_name = (char *)list_key(node);
		fd = cli_conn(client_name);
		if (fd < 0) {
			err = NETWORK_ERROR_CLI_CONN;
			continue;
		}
		net_send_galaxy_event(fd, filename, mask);
		close(fd);
	}
	pthread_mutex_unlock(&watches_mutex);
*/
	return err;
}
