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

#ifndef NETWORK_H
#define NETWORK_H

#include <inttypes.h>
#include <galaxy.h>

#define CLI_PATH            "/var/tmp/"
/* TODO: Following 2 defines only used by net_send_string(). Remove? */
#define NETWORK_INT_T       uint32_t
#define NETWORK_INT_LENGTH  sizeof(NETWORK_INT_T)

/* Error codes for network functions. */
#define NETWORK_ERROR_WRITE          -1
#define NETWORK_ERROR_SOCKET         -2
#define NETWORK_ERROR_BIND           -3
#define NETWORK_ERROR_LISTEN         -4
#define NETWORK_ERROR_ACCEPT         -5
#define NETWORK_ERROR_STAT           -6
#define NETWORK_ERROR_NOT_SOCKET     -7   /* IS_SOCK() is false. */
#define NETWORK_ERROR_NOT_RWX        -8   /* Perms are not rwx------. */
#define NETWORK_ERROR_INODE_TOO_OLD  -9   /* I-node is too old. */
#define NETWORK_ERROR_CHMOD          -10
#define NETWORK_ERROR_CONNECT        -11
#define NETWORK_ERROR_CLI_CONN       -12
#define NETWORK_ERROR_PARTIAL_WRITE  -13  /* Wrote a partial message. */
#define NETWORK_ERROR_SERV_LISTEN    -14
#define NETWORK_ERROR_READ           -15
#define NETWORK_ERROR_PARTIAL_READ   -16
#define NETWORK_ERROR_NET_SEND_UINT32 -17
#define NETWORK_ERROR_NET_SEND_STRING -18
#define NETWORK_ERROR_NET_RECV_UINT32 -19
#define NETWORK_ERROR_NET_RECV_STRING -20
#define NETWORK_ERROR_MALLOC          -21
#define NETOWRK_ERROR_NOT_NOTIFICATION -22
#define NETWORK_ERROR_NET_RECV_CMD    -23
#define NETWORK_ERROR_NET_RECV_MASK   -24

void print_sockname(int fd);
int serv_listen(const char *name);
int serv_accept(int listenfd, uid_t *uidptr);
int cli_conn(const char *name);
int net_send_uint32(int fd, const uint32_t uint);
int net_send_string(int fd, const char *string);
int net_send_galaxy_event(int fd, const char *filename, uint32_t mask);
int net_recv_uint32(int fd, uint32_t *retval);
char *net_recv_string(int fd);
int net_recv_galaxy_event(int fd, struct galaxy_event_t *gevent);

void err_serv_listen(int err);
void err_serv_accept(int err);
void err_cli_conn(int err);
void err_net_send_uint32(int err);
void err_net_send_string(int err);
void err_net_send_galaxy_event(int err);
void err_net_recv_uint32(int err);
void err_net_recv_string(int err);
void err_net_recv_galaxy_event(int err);

#endif
