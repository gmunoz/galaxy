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

#include <stdio.h>

#if HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif

#if HAVE_UNISTD_H
#  include <unistd.h>
#endif

#if HAVE_STRING_H
#  include <string.h>
#endif

#if HAVE_ERRNO_H
#  include <errno.h>
#endif

#include "galaxy.h"
#include "galnet.h"
#include "error.h"

static uint32_t uniqueid = 0;

struct galaxy_event_t *
create_galaxy_event(void)
{
	struct galaxy_event_t *gevent;

	gevent = malloc(sizeof(struct galaxy_event_t));
	if (gevent == NULL) {
		err_malloc(errno);
		err_msg("error[create_galaxy_event]: Unable to malloc galaxy event structure.\n");
		return NULL;
	}

	gevent->mask = 0;
	gevent->name = NULL;

	return gevent;
}

void
destroy_galaxy_event(void *ptr)
{
	struct galaxy_event_t *gevent;

	gevent = (struct galaxy_event_t *)ptr;

	if (gevent->name != NULL)
		free(gevent->name);
	free(gevent);
}

/*
 * This will connect to the galaxy daemon (galaxyd) and negotiate the
 * name of the server-side listening socket (used on the client-side for
 * sending various client command--e.g. galaxy_watch(), etc.). This will
 * also initiate a client-side listening socket and communicate its
 * socket name to galaxy daemon, so the daemon can send responses back.
 * All of this info is stored (in an abstract way) inside the struct
 * galaxy_t structure. This structure is used for all communications to
 * the galaxy daemon.
 *
 * This function will not return until the server communicates that the
 * server end-point has been instantiated. This will ensure that we can
 * connect to the client immediately after this function returns.
 *
 * Return Value:
 *   Returns negaive value on error or 0 on success.
 */
int
galaxy_connect(struct galaxy_t *galaxy)
{
	int connfd, err;
	uint32_t pid, ack;
	char cliname[4096];  /* FIXME: Use maxpath. */

	/* Create client-side socket end-point and communicate its socket name
	 * with the galaxy daemon. */

	pid = getpid();

	sprintf(cliname, "%s%05d.%d", CLI_PATH, pid, uniqueid++);
#ifdef DEBUG_GALAXY_CONNECT
	err_msg("DEBUG[galaxy_connect]: client-side server name = %s\n", cliname);
#endif
	galaxy->fd = serv_listen(cliname);
	if (galaxy->fd < 0) {
		err_msg("error[galaxy_connect]: Unable to create client-side socket end-point:\n");
		err_msg("       '%s'\n", cliname);
		return NETWORK_ERROR_SERV_LISTEN;
	}

	/* Get connection to primary galaxy server socket. */
	connfd = cli_conn(GALAXY_SOCKET);

	/* Communicate the client-side socket end-point socket name with the
	 * galaxy daemon. */
	err = net_send_string(connfd, cliname);
	if (err < 0)
		goto end;

	/* Communicate the unique server name to create an end-point on the
	 * galaxy daemon server. Unique name take the format of:
	 *   <pid>.<uniqueid> */

	/* Write pid to server. This is the 1st part of unique name. */
	err = net_send_uint32(connfd, pid);
	if (err < 0)
		goto end;

	/* Write internal ID to server. This is 2nd part of unique name. */
	err = net_send_uint32(connfd, uniqueid);
	if (err < 0)
		goto end;

	/* Read ACK from server so we know when to get a client connection. */
	err = net_recv_uint32(connfd, &ack);
	if (err < 0)  /* FIXME: What should I do on this error? */
		goto end;

	/* If ACK failed, then server side socket creation failed--err out. */
	if (ack == ACK_FAIL) {
		err_msg("error[galaxy_connect]: Server failed to create listener socket for\n");
		err_msg("       '%s'\n", galaxy->sname);
		return -3;
	}

	close(connfd);

	/* Store the unique server socket path name into the return value. */
	sprintf(galaxy->sname, "%s%05d.%d", CLI_PATH, pid, uniqueid);
	err_msg("DEBUG[galaxy_connect]: name = %s\n", galaxy->sname);

	uniqueid++;  /* Must increment after final sprintf(). */

	return 0;

end:
	close(connfd);
	return err;
}

int
galaxy_close(struct galaxy_t *galaxy)
{
	int connfd, err;

	close(galaxy->fd);

	connfd = cli_conn(galaxy->sname);
	if (connfd < 0) {
		err_msg("error[galaxy_close]: Unable to obtain client connection.\n");
		return NETWORK_ERROR_CLI_CONN;
	}

	err = net_send_uint32(connfd, GALAXY_EXIT);

	close(connfd);

	return err;
}

struct galaxy_event_t *
galaxy_receive(struct galaxy_t *galaxy)
{
	int connfd, err;
	struct galaxy_event_t *gevent;

	connfd = serv_accept(galaxy->fd, NULL);

	gevent = create_galaxy_event();
	if (gevent == NULL)
		goto end;

	err = net_recv_galaxy_event(connfd, gevent);
	if (err < 0) {
		err_msg("error[galaxy_receive]: Unable to receive event.\n");
		destroy_galaxy_event(gevent);
		gevent = NULL;
	}

end:
	close(connfd);
	return gevent;
}

/*
 * This function will notify the galaxy daemon to watch for the given
 * files/directories as specified by the regular expression.
 *
 * The command and mask will always be sent. If the regular expression
 * string is NULL, then the network send will be skipped.
 *
 * Parameters:
 *   galaxy: The galaxy_t descriptor that must be created using
 *     galaxy_connect(). It describes the galaxy daemon that a client
 *     application will communicate with.
 *   command: The galaxy command to send to the server.
 *   mask: A bitwise inclusive OR of the Inotify events that will be
 *     tested for matching the given regular expression.
 *   regexp: A regular expression that will be used to match a galaxy
 *     event with the filename that the event is associated with. If
 *     this value is NULL, then the regexp will not be sent.
 *
 * TODO: Clarify what the error return codes are for this, since they
 * are actually getting propogated from the net_send_* functions.
 *
 * Return Value:
 *   Returns -1 when write(2) system call fails. Returns -2 when the
 *   entire message wasn't written to the socket connection. Returns 0
 *   when finishes successfully.
 */
int
galaxy_send_server_command(const struct galaxy_t *galaxy,
	galaxy_cmd_t command, uint32_t mask, const char *regexp)
{
	int connfd, err;

	/* Get a client connection to the given galaxy socket name. */
	connfd = cli_conn(galaxy->sname);
	if (connfd < 0) {
		err_msg("error[galaxy_send_server_command]: Unable to obtain client connection.\n");
		return NETWORK_ERROR_CLI_CONN;
	}

	/* Send command number. */
	err = net_send_uint32(connfd, command);
	if (err < 0)
		goto end;

	/* Send the Inotify mask. */
	err = net_send_uint32(connfd, mask);
	if (err < 0)
		goto end;

	/* Send the string data. It is possible to have a NULL value for the
	 * string to send. Check for this explicitly and skip if it is NULL. */
	if (regexp) {
		err = net_send_string(connfd, regexp);
		if (err < 0)
			goto end;
	}

end:
	close(connfd);
	return err;
}
