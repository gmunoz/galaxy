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

#if HAVE_UNISTD_H
#  include <unistd.h>
#endif

#if HAVE_SYS_SELECT_H
#  include <sys/select.h>
#endif

#if HAVE_SYS_TIME_H
#  include <sys/time.h>
#endif

#if HAVE_PTHREAD_H
#  include <pthread.h>
#endif

#if HAVE_ERRNO_H
#  include <errno.h>
#endif

#if HAVE_SYS_SOCKET_H
#  include <sys/socket.h>
#endif

#if HAVE_SYS_UN_H
#  include <sys/un.h>
#endif

#include "server_thread.h"
#include "galaxy.h"
#include "galnet.h"
#include "thread.h"
#include "watch.h"
#include "error.h"

struct client_data_t {
	int listenfd;
	char *cliservname;
} client_data_t;

static void *
client_request(void *arg)
{
	struct client_data_t *cdata;

	cdata = (struct client_data_t *)arg;

	while (1) {
		uint32_t cmd, mask;
		int connfd, err;

#ifdef DEBUG_CLIENT_REQUEST
		err_msg("DEBUG[client_request]: Accepting on socket...\n");
		print_sockname(cdata->listenfd);
#endif

		connfd = serv_accept(cdata->listenfd, NULL);
		if (connfd < 0) {
			err_msg("error[client_request]: Accepting server request failed.\n");
			continue;
		}
#ifdef DEBUG_CLIENT_REQUEST
		err_msg("DEBUG[client_request]: Received a command from client.\n");
#endif

		/* TODO: The command receive should be abstracted into a function
		 * galaxy_recv_server_command() into libgalaxy/libgalaxy.c. It
		 * should mirror galaxy_send_server_command(). */

		/* Receive command number. */
#ifdef DEBUG_CLIENT_REQUEST
		err_msg("  => DEBUG[client_request]: Waiting to receive a command...\n");
#endif
		err = net_recv_uint32(connfd, &cmd);
		if (err < 0) {
			close(connfd);
			continue;
		}
#ifdef DEBUG_CLIENT_REQUEST
		err_msg("     + Received command\n");
		err_msg("     + sizeof(cmd) = %d\n", sizeof(cmd));
		switch (cmd) {
			case GALAXY_WATCH:
				err_msg("     + command type = GALAXY_WATCH\n");
				break;
			case GALAXY_EXIT:
				err_msg("     + command type = GALAXY_EXIT\n");
				break;
			case GALAXY_IGNORE_MASK:
				err_msg("     + command type = GALAXY_IGNORE_MASK\n");
				break;
			case GALAXY_IGNORE_WATCH:
				err_msg("     + command type = GALAXY_IGNORE_WATCH\n");
				break;
			default:
				err_msg("     + unrecognized command = %d (see galaxy.h)\n", cmd);
				break;
		}
#endif

		/* Check for an exit command before waiting for data payload. */
		if (cmd == GALAXY_EXIT) {
			remove_galaxy_watches(cdata->cliservname);
#ifdef DEBUG_CLIENT_REQUEST
			err_msg("  => DEBUG[client_request]: Exiting client server.\n");
#endif
			close(connfd);
			break;
		}
		
		/* Receive the Inotify mask, used to filter out event matches. */
#ifdef DEBUG_CLIENT_REQUEST
		err_msg("  => DEBUG[client_request]: Receiving the Inotify mask...\n");
#endif
		err = net_recv_uint32(connfd, &mask);
		if (err < 0) {
			close(connfd);
			continue;
		}
#ifdef DEBUG_CLIENT_REQUEST
		err_msg("     + Received the inotify mask\n");
		err_msg("     + Inotify mask = 0x%x\n", mask);
#endif

		/* Take action according to the received galaxy command. */
		switch (cmd) {
			char *regexp;
			case GALAXY_WATCH:
#ifdef DEBUG_CLIENT_REQUEST
				err_msg("  => DEBUG[client_request]: Receiving payload...\n");
#endif
				regexp = net_recv_string(connfd);
#ifdef DEBUG_CLIENT_REQUEST
				err_msg("     + Received payload data: '%s'\n", regexp);
#endif
				add_galaxy_watch(cdata->cliservname, mask, regexp);
				break;
			case GALAXY_IGNORE_WATCH:
#ifdef DEBUG_CLIENT_REQUEST
				err_msg("  => DEBUG[client_request]: Receiving payload...\n");
#endif
				regexp = net_recv_string(connfd);
#ifdef DEBUG_CLIENT_REQUEST
				err_msg("     + Received payload data: '%s'\n", regexp);
#endif
				add_galaxy_ignore_watch(cdata->cliservname, mask, regexp);
				break;
			case GALAXY_IGNORE_MASK:
				set_galaxy_ignore_mask(cdata->cliservname, mask);
				break;
			default:
				err_msg("warning[client_request]: Unrecognized galaxy command. Ignoring this command.\n");
				break;
		}

		close(connfd);
	}

	close(cdata->listenfd);
	free(((struct client_data_t *)arg)->cliservname);
	free(arg);

	return NULL;
}

static void *
server_thread(void *arg)
{
	int err, listenfd, connfd;
	struct client_data_t *cdata;

	listenfd = *((int *)arg);
	free(arg);

	while (1) {
		connfd = serv_accept(listenfd, NULL);

		/* Read the unique pid and id from client to form another
		 * server-side listening socket. */
		uint32_t pid, id;
		char name[4096];  /* FIXME: Use maxpath. */

		cdata = malloc(sizeof(struct client_data_t));
		if (cdata == NULL) {
			err_malloc(errno);
			err_msg("error[server_thread]: Unable to malloc struct client_data_t.\n");
			close(connfd);
			continue;
		}

		/* Read socket name (string) of the client-side server. */
		cdata->cliservname = net_recv_string(connfd);
		if (cdata->cliservname == NULL) {
			err_msg("error[server_thread]: Didn't read client-side socket name.\n");
			free(cdata);
			close(connfd);
			continue;
		}

#ifdef DEBUG_SERVER_THREAD
		err_msg("DEBUG[server_thread]: client-side socket name = %s\n",
			cdata->cliservname);
#endif

		/* Read the pid of the client process--used for unique name. */
		err = net_recv_uint32(connfd, &pid);
		if (err < 0) {
			err_msg("error[server_thread]: Unable to receive client PID.\n");
			free(cdata->cliservname);
			free(cdata);
			close(connfd);
			continue;
		}

		/* Read client specific unique id. */
		err = net_recv_uint32(connfd, &id);
		if (err < 0) {
			err_msg("error[server_thread]: Unable to receive client unique ID.\n");
			free(cdata->cliservname);
			free(cdata);
			close(connfd);
			continue;
		}

		/* Append PID and client unique id to form our filename. */
		sprintf(name, "%s%05d.%d", CLI_PATH, pid, id);
#ifdef DEBUG_SERVER_THREAD
		err_msg("server path = %s\n", name);
#endif

		/* Create server socket end-point. */
		cdata->listenfd = serv_listen(name);
		if (cdata->listenfd < 0) {
			err_msg("error[server_thread]: Unable to create listener socket on:\n");
			err_msg("      '%s'\n", name);
			net_send_uint32(connfd, ACK_FAIL);
			free(cdata->cliservname);
			free(cdata);
			close(connfd);
			continue;
		}

		/* Send ACK of new server end-point. */
		err = net_send_uint32(connfd, ACK_SUCCESS);
		if (err < 0) {
			err_msg("error[server_thread]: Unable to send ACK to client.\n");
			free(cdata->cliservname);
			free(cdata);
			close(connfd);
			continue;
		}

		/* Spawn a thread to handle client request. */
		pthread_t handler;
		err = create_detached_thread(&handler, client_request, cdata);
		if (err < 0)
			err_create_detached_thread(errno);

		close(connfd);
	}

	return NULL;
}

int
create_server_thread(pthread_t *id, int listenfd)
{
	int err, *fd;

	fd = malloc(sizeof(int));
	if (fd == NULL) {
		err_malloc(errno);
		err_msg("error[create_server_thread]: Unable to malloc for int.\n");
		return -1;
	}
	*fd = listenfd;

	err = create_joinable_thread(id, server_thread, fd);
	if (err < 0)
		err_create_joinable_thread(errno);

	return err;
}
