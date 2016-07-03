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

#if HAVE_STDDEF_H
#  include <stddef.h>
#endif

#if HAVE_UNISTD_H
#  include <unistd.h>
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

#if HAVE_SYS_SOCKET_H
#  include <sys/socket.h>
#endif

#if HAVE_SYS_UN_H
#  include <sys/un.h>
#endif

#if HAVE_INTTYPES_H
#  include <inttypes.h>
#endif

#if HAVE_ERRNO_H
#  include <errno.h>
#endif

#include <time.h>

#include "galnet.h"
#include "error.h"

#define QLEN      10
#define STALE     30
#define CLI_PERM  S_IRWXU

#define GALAXY_NOTIFICATION  1121

void
print_sockname(int fd)
{
	socklen_t len = 0;
	struct sockaddr_un addr;

	memset(&addr, 0, sizeof(struct sockaddr_un));
	getsockname(fd, (struct sockaddr *)&addr, &len);
#ifdef DEBUG_PRINT_SOCKET
	err_msg("  + bound name = %s, returned len = %d\n", addr.sun_path, len);
#endif
}

/*
 * Create a server endpoint of a connection.
 *
 * Return Value:
 *   Returns a socket file descriptor that is set to listen for
 *   incomming connections on the given socket name. On error a negative
 *   int is returned.
 *
 * Errors:
 *   NETWORK_ERROR_SOCKET
 *     The system call socket(2) has failed. errno will retain the
 *     error code of socket(2).
 *   NETWORK_ERROR_BIND
 *     The system call bind(2) has failed. errno will retain the error
 *     code of bind(2).
 *   NETWORK_ERROR_LISTEN
 *     The system call listen(2) has failed. errno will retain the
 *     error code of listen(2).
 */
int
serv_listen(const char *name)
{
	int fd, len, err, rval;
	struct sockaddr_un un;

	/* Create a UNIX domain stream socket. */
	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		err_socket(errno);
		err_msg("error[serv_listen]: Unable to create UNIX domain socket.\n");
		return NETWORK_ERROR_SOCKET;
	}

	unlink(name);  /* In case it already exists. */

	/* Fill in socket address structure. */
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	strcpy(un.sun_path, name);
	len = offsetof(struct sockaddr_un, sun_path) + strlen(name);

	/* Bind the name to the descriptor. */
	err = bind(fd, (struct sockaddr *)&un, len);
	if (err < 0) {
		err_bind(errno);
		err_msg("error[serv_listen]: Unable to bind name to descriptor.\n");
		rval = NETWORK_ERROR_BIND;
		goto errout;
	}

	err = listen(fd, QLEN);
	if (err < 0) {  /* Tell the kernel we're a server. */
		err_listen(errno);
		err_msg("error[serv_listen]: Unable to listen on socket.\n");
		rval = NETWORK_ERROR_LISTEN;
		goto errout;
	}

	return fd;

errout:
	err = errno;
	close(fd);
	errno = err;
	return rval;
}

/*
 * Wait for a client connection to arrive and accept it. We also obtain
 * the client's user ID from the pathname that it must bind before
 * calling us.
 *
 * Return Value:
 *   Returns a new socket file descriptor that can be used to read/write
 *   messages to/from. On error, a negative int is returned.
 *
 * Errors:
 *   NETWORK_ERROR_ACCEPT
 *     The system call accept(2) has failed. errno will retain the error
 *     code of accept(2).
 *   NETWORK_ERROR_STAT
 *     The system call stat(2) has failed. errno will retain the error
 *     code of stat(2).
 *   NETWORK_ERROR_NOT_SOCKET
 *     The file is not a valid socket.
 *   NETWORK_ERROR_NOT_RWX
 *     The file does not contain permissions rwx------.
 *   NETWORK_ERROR_INODE_TOO_OLD
 *     The i-node is too old.
 */
int
serv_accept(int listenfd, uid_t *uidptr)
{
	int clifd, err, rval;
	time_t staletime;
	struct sockaddr_un un;
	socklen_t len;
	struct stat statbuf;

	len = sizeof(un);
	clifd = accept(listenfd, (struct sockaddr *)&un, &len);
	if (clifd < 0) {
		err_accept(errno);
		err_msg("error[serv_accept]: Unable to accept connection.\n");
		return NETWORK_ERROR_ACCEPT;
	}

	/* Obtain the client's uid from its calling address. */
	len -= offsetof(struct sockaddr_un, sun_path); /* len of pathname */
	un.sun_path[len] = 0;  /* NULL terminate */

	err = stat(un.sun_path, &statbuf);
	if (err < 0) {
		err_stat(errno);
		err_msg("error[serv_accept]: Unable to stat UNIX domain socket '%s'.\n",
			un.sun_path);
		rval = NETWORK_ERROR_STAT;
		goto errout;
	}

	if (S_ISSOCK(statbuf.st_mode) == 0) {
		err_msg("error[serv_accept]: File is not a socket.\n");
		rval = NETWORK_ERROR_NOT_SOCKET;     /* Not a socket. */
		goto errout;
	}

	if ((statbuf.st_mode & (S_IRWXG | S_IRWXO)) ||
	    (statbuf.st_mode & S_IRWXU) != S_IRWXU) {
		err_msg("error[serv_accept]: Socket mode is not rwx------.\n");
		rval = NETWORK_ERROR_NOT_RWX;        /* Perms are not rwx------. */
		goto errout;
	}

	staletime = time(NULL) - STALE;
	if (statbuf.st_atime < staletime ||
	    statbuf.st_ctime < staletime ||
	    statbuf.st_mtime < staletime) {
		rval = NETWORK_ERROR_INODE_TOO_OLD;  /* i-node is too old. */
		goto errout;
	}

	if (uidptr != NULL)
		*uidptr = statbuf.st_uid;  /* Return uid of caller. */
	//unlink(un.sun_path);  /* We're done with pathname now. */

	return clifd;

errout:
	err = errno;
	close(clifd);
	errno = err;
	return rval;
}

/*
 * Create a client endpoint and connect to a server.
 *
 * Return Value:
 *   Returns a new socket file descriptor that represents the client
 *   connection to the given socket name. This socket may be ready for
 *   reading and writing. On error, a negative int is returned.
 *
 * Errors:
 *   NETWORK_ERROR_SOCKET
 *     The system call socket(2) has failed. errno will retain the
 *     error code of socket(2).
 *   NETWORK_ERROR_BIND
 *     The system call bind(2) has failed. errno will retain the error
 *     code of bind(2).
 *   NETWORK_ERROR_CHMOD
 *     The system call chmod(2) has failed. errno will retain the error
 *     code of chmod(2).
 *   NETWORK_ERROR_CONNECT
 *     The system call connect(2) has failed. errno will retain the
 *     error code of connect(2).
 */
int
cli_conn(const char *name)
{
	int fd, len, err, rval;
	struct sockaddr_un un;

	/* Create a UNIX domain stream socket. */
	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		err_socket(errno);
		err_msg("error[cli_conn]: Unable to create UNIX domain socket.\n");
		return NETWORK_ERROR_SOCKET;
	}

	/* Fill socket address structure with out address. */
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	sprintf(un.sun_path, "%s%05d", CLI_PATH, getpid());
	len = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);

	unlink(un.sun_path);  /* In case it already exists. */
	err = bind(fd, (struct sockaddr *)&un, len);
	if (err < 0) {
		err_bind(errno);
		err_msg("error[cli_conn]: Unable to bind name to descriptor.\n");
		rval = NETWORK_ERROR_BIND;
		goto errout;
	}
	err = chmod(un.sun_path, CLI_PERM);
	if (err < 0) {
		err_chmod(errno);
		err_msg("error[cli_conn]: Unable to chmod socket.\n");
		rval = NETWORK_ERROR_CHMOD;
		goto errout;
	}

	/* Fill socket address structure with server's address. */
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	strcpy(un.sun_path, name);
	len = offsetof(struct sockaddr_un, sun_path) + strlen(name);
	err = connect(fd, (struct sockaddr *)&un, len);
	if (err < 0) {
		err_connect(errno);
		err_msg("error[cli_conn]: Unable to connect to socket.\n");
		rval = NETWORK_ERROR_CONNECT;
		goto errout;
	}

	return fd;

errout:
	err = errno;
	close(fd);
	errno = err;
	return rval;
}

/*
 * Sends an unsigned int across a socket connection.
 *
 * Return Value:
 *   On success, zero is returned. On error, a negative int is returned.
 *
 * Errors:
 *   NETWORK_ERROR_WRITE
 *     The system call write(2) has failed. errno will retain the error
 *     code of write(2).
 *   NETWORK_ERROR_PARTIAL_WRITE
 *     The system call write(2) only wrote a fragment of the uint32_t.
 */
int
net_send_uint32(int fd, const uint32_t uint)
{
	int len, err, save;
	ssize_t bytes;

	/* Send command number. */
	len = sizeof(uint32_t);
	bytes = write(fd, &uint, len);
	if (bytes != len)
		goto errout;

	return 0;

errout:
	save = errno;
	if (bytes == -1) {          /* write(2) syscall error. */
		err_write(errno);
		err_msg("error[net_send_uint32]: Unable to write to server socket.\n");
		err = NETWORK_ERROR_WRITE;
	} else if (bytes != len) {  /* Didn't write entire string. */
		err_msg("error[net_send_uint32]: Didn't write entire message to server.\n");
		err_msg("       Wrote %d bytes of %d total bytes.\n", bytes, len);
		err = NETWORK_ERROR_PARTIAL_WRITE;
	}
	errno = save;
	return err;
}

/*
 * Sends a character string across a socket connection.
 *
 * Return Value:
 *   On success, zero is returned. On error, a negative int is returned.
 *
 * Errors:
 *   NETWORK_ERROR_WRITE
 *     The system call write(2) has failed. errno will retain the error
 *     code of write(2).
 *   NETWORK_ERROR_PARTIAL_WRITE
 *     The system call write(2) only wrote a partial fragment of the
 *     character string.
 */
int
net_send_string(int fd, const char *string)
{
	int len, err, save;
	ssize_t bytes;
	NETWORK_INT_T string_len;

	/* Send length of string. */
	len = NETWORK_INT_LENGTH;
	string_len  = strlen(string) + 1; /* Includes the terminating '\0'. */
	bytes = write(fd, &string_len, len);
	if (bytes != len)
		goto errout;

	/* Send the string. */
	len = string_len;
	bytes = write(fd, string, len);
	if (bytes != len)
		goto errout;

	return 0;

errout:
	save = errno;
	if (bytes == -1) {          /* write(2) syscall error. */
		err_write(errno);
		err_msg("error[net_send_string]: Unable to write to server socket.\n");
		err = NETWORK_ERROR_WRITE;
	} else if (bytes != len) {  /* Didn't write entire string. */
		err_msg("error[net_send_string]: Didn't write entire message to server.\n");
		err_msg("       Wrote %d bytes of %d total bytes.\n", bytes, len);
		err = NETWORK_ERROR_PARTIAL_WRITE;
	}
	errno = save;
	return err;
}

/*
 * Sends the internals of a galaxy event across a network connection.
 *
 * Return Value:
 *   On success, zero is returned. On error, a negative int is returned.
 *
 * Errors:
 *   NETWORK_ERROR_NET_SEND_UINT32
 *     The network function net_send_uint32() failed.
 *   NETWORK_ERROR_NET_SEND_STRING
 *     The network function net_send_string() failed.
 */
int
net_send_galaxy_event(int fd, const char *filename, uint32_t mask)
{
	int err;

	err = net_send_uint32(fd, GALAXY_NOTIFICATION);
	if (err < 0) {
		err_net_send_uint32(err);
		err_msg("error[net_send_galaxy_event]: Unable to send notification command.\n");
		return NETWORK_ERROR_NET_SEND_UINT32;
	}

	err = net_send_uint32(fd, mask);
	if (err < 0) {
		err_net_send_uint32(err);
		err_msg("error[net_send_galaxy_event]: Unable to send string length.\n");
		return NETWORK_ERROR_NET_SEND_UINT32;
	}

	err = net_send_string(fd, filename);
	if (err < 0) {
		err_net_send_string(err);
		err_msg("error[net_send_galaxy_event]: Unable to send string.\n");
		return NETWORK_ERROR_NET_SEND_STRING;
	}

	return 0;
}

/*
 * Reveives an unsigned int from a socket connection.
 *
 * Return Value:
 *   On success, zero is returned. On error, a negative int is returned.
 *
 * Errors:
 *   NETWORK_ERROR_READ
 *     The system call read(2) has failed. errno will retain the error
 *     code of write(2).
 *   NETWORK_ERROR_PARTIAL_READ
 *     The system call read(2) only wrote a fragment of the uint32_t.
 */
int
net_recv_uint32(int fd, uint32_t *retval)
{
	int len, err, save;
	ssize_t bytes;

	len = sizeof(uint32_t);
	bytes = read(fd, retval, len);
	if (bytes != len)
		goto errout;

	return 0;

errout:
	save = errno;
	if (bytes == -1) {          /* read(2) syscall error. */
		err_read(errno);
		err_msg("error[net_recv_uint32]: Unable to read from server socket.\n");
		err = NETWORK_ERROR_READ;
	} else if (bytes != len) {  /* Didn't write entire string. */
		err_msg("error[net_recv_uint32]: Didn't read entire message from server.\n");
		err_msg("       Read %d bytes of %d total bytes.\n", bytes, len);
		err = NETWORK_ERROR_PARTIAL_READ;
	}
	errno = save;
	return err;
}

/*
 * Receives a character string from a socket connection. Memory for the
 * returned string is obtained with malloc(3), and must be deallocated
 * with free(3) by the callee.
 *
 * Return Value:
 *   On success, returns a pointer to the received string. On error,
 *   returns NULL, and errno is set appropriately.
 *
 * Errors:
 *   NETWORK_ERROR_NET_RECV_UINT32
 *     The network function net_recv_uint32() failed.
 *   NETWORK_ERROR_MALLOC
 *     The library call malloc(3) failed. errno will retain the error
 *     code of malloc(3).
 *   NETWORK_ERROR_PARTIAL_READ
 *     The system call read(2) only wrote a fragment of the uint32_t.
 */
char *
net_recv_string(int fd)
{
	int len, err;
	ssize_t bytes;
	char *string;

	err = net_recv_uint32(fd, &len);
	if (err < 0) {
		err_net_recv_uint32(err);
		err_msg("error[net_recv_string]: Unable to receive string length.\n");
		errno = NETWORK_ERROR_NET_RECV_UINT32;
		return NULL;
	}

	string = malloc(len);
	if (string == NULL) {
		err_malloc(errno);
		err_msg("error[net_recv_string]: Unable to allocate string of %d bytes.\n",
			len);
		errno = NETWORK_ERROR_MALLOC;
		return NULL;
	}

	bytes = read(fd, string, len);
	if (bytes != len) {
		err_read(errno);
		err_msg("error[net_recv_string]: Read a partial string.\n");
		goto errout;
	}

	return string;

errout:
	if (bytes == -1) {          /* read(2) syscall error. */
		err_read(errno);
		err_msg("error[net_recv_string]: Unable to read from server socket.\n");
		errno = NETWORK_ERROR_READ;
	} else if (bytes != len) {  /* Didn't write entire string. */
		err_msg("error[net_recv_string]: Didn't read entire message from server.\n");
		err_msg("       Read %d bytes of %d total bytes.\n", bytes, len);
		errno = NETWORK_ERROR_PARTIAL_READ;
	}
	return NULL;
}

/*
 * Receives a galaxy event from a socket connection.
 *
 * Return Value:
 *   On success, zero is returned. On error, a negative int is returned.
 *
 * Errors:
 *   NETWORK_ERROR_NET_RECV_CMD
 *     The network function net_recv_uint32() failed to receive command.
 *   NETOWRK_ERROR_NOT_NOTIFICATION
 *     The command is not a galaxy notification command.
 *   NETWORK_ERROR_NET_RECV_MASK
 *     The network function net_recv_uint32() failed to receive mask.
 *   NETWORK_ERROR_NET_RECV_STRING
 *     The network function net_recv_string failed.
 */
int
net_recv_galaxy_event(int fd, struct galaxy_event_t *gevent)
{
	int cmd, err;

	err = net_recv_uint32(fd, &cmd);
	if (err < 0) {
		err_net_recv_uint32(err);
		err_msg("error[net_recv_galaxy_event]: Unable to receive command.\n");
		return NETWORK_ERROR_NET_RECV_CMD;
	}

	if (cmd != GALAXY_NOTIFICATION) {
		err_msg("error[net_recv_galaxy_event]: Command is not a galaxy notification.\n");
		return NETOWRK_ERROR_NOT_NOTIFICATION;
	}

	err = net_recv_uint32(fd, &gevent->mask);
	if (err < 0) {
		err_net_recv_uint32(err);
		err_msg("error[net_recv_galaxy_event]: Unable to receive uint32.\n");
		return NETWORK_ERROR_NET_RECV_MASK;
	}

	gevent->name = net_recv_string(fd);
	if (gevent->name == NULL) {
		err_net_recv_string(errno);
		err_msg("error[net_recv_galaxy_event]: Unable to receive string.\n");
		return NETWORK_ERROR_NET_RECV_STRING;
	}

	gevent->timestamp = time(NULL);

	return 0;
}

void err_serv_listen(int err)
{
	err_msg("error: serv_listen() failed.\n");
	switch (err) {
		case NETWORK_ERROR_SOCKET:
			err_msg("       The system call socket(2) has failed.\n");
			break;
		case NETWORK_ERROR_BIND:
			err_msg("       The system call bind(2) has failed.\n");
			break;
		case NETWORK_ERROR_LISTEN:
			err_msg("       The system call listen(2) has failed.\n");
			break;
	}
	fflush(stderr);
}

void err_serv_accept(int err)
{
	err_msg("error: serv_accept() failed.\n");
	switch (err) {
		case NETWORK_ERROR_ACCEPT:
			err_msg("       The system call accept(2) has failed.\n");
			break;
		case NETWORK_ERROR_STAT:
			err_msg("       The system call stat(2) has failed.\n");
			break;
		case NETWORK_ERROR_NOT_SOCKET:
			err_msg("       The file is not a valid socket.\n");
			break;
		case NETWORK_ERROR_NOT_RWX:
			err_msg("       The file does not contain permissions rwx------.\n");
			break;
		case NETWORK_ERROR_INODE_TOO_OLD:
			err_msg("       The i-node is too old.\n");
			break;
	}
	fflush(stderr);
}

void err_cli_conn(int err)
{
	err_msg("error: cli_conn() failed.\n");
	switch (err) {
		case NETWORK_ERROR_SOCKET:
			err_msg("       The system call socket(2) has failed.\n");
			break;
		case NETWORK_ERROR_BIND:
			err_msg("       The system call bind(2) has failed.\n");
			break;
		case NETWORK_ERROR_CHMOD:
			err_msg("       The system call chmod(2) has failed.\n");
			break;
		case NETWORK_ERROR_CONNECT:
			err_msg("       The system call connect(2) has failed.\n");
			break;
	}
	fflush(stderr);
}

void err_net_send_uint32(int err)
{
	err_msg("error: The network function net_send_uint32() failed.\n");
	switch (err) {
		case NETWORK_ERROR_WRITE:
			err_msg("       The system call write(2) has failed.\n");
			break;
		case NETWORK_ERROR_PARTIAL_WRITE:
			err_msg("       The system call write(2) only wrote a fragment of the uint32_t.\n");
			break;
	}
	fflush(stderr);
}

void err_net_send_string(int err)
{
	err_msg("error: The network function net_send_string() failed.\n");
	switch (err) {
		case NETWORK_ERROR_WRITE:
			err_msg("       The system call write(2) has failed.\n");
			break;
		case NETWORK_ERROR_PARTIAL_WRITE:
			err_msg("       The system call write(2) only wrote a fragment of the character string.\n");
			break;
	}
	fflush(stderr);
}

void err_net_send_galaxy_event(int err)
{
	err_msg("error: The network function net_send_galaxy_event() failed.\n");
	switch (err) {
		case NETWORK_ERROR_NET_SEND_UINT32:
			err_msg("       The network function net_send_uint32() failed.\n");
			break;
		case NETWORK_ERROR_NET_SEND_STRING:
			err_msg("       The network function net_send_string() failed.\n");
			break;
	}
	fflush(stderr);
}

void err_net_recv_uint32(int err)
{
	err_msg("error: The network function net_recv_uint32() failed.\n");
	switch (err) {
		case NETWORK_ERROR_READ:
			err_msg("       The system call read(2) has failed.\n");
			break;
		case NETWORK_ERROR_PARTIAL_READ:
			err_msg("       The system call read(2) only wrote a fragment of the uint32_t.\n");
			break;
	}
	fflush(stderr);
}

void err_net_recv_string(int err)
{
	err_msg("error: The network function net_recv_string() failed.\n");
	switch (err) {
		case NETWORK_ERROR_NET_RECV_UINT32:
			err_msg("       The network function net_recv_uint32() failed.\n");
			break;
		case NETWORK_ERROR_MALLOC:
			err_msg("       The library call malloc(3) failed.\n");
			break;
		case NETWORK_ERROR_PARTIAL_READ:
			err_msg("       The system call read(2) only wrote a fragment of the uint32_t.\n");
			break;
	}
	fflush(stderr);
}

void err_net_recv_galaxy_event(int err)
{
	err_msg("error: The network function net_recv_galaxy_event() failed.\n");
	switch (err) {
		case NETWORK_ERROR_NET_RECV_CMD:
			err_msg("       The network function net_recv_uint32() failed to receive command.\n");
			break;
		case NETOWRK_ERROR_NOT_NOTIFICATION:
			err_msg("       The command is not a galaxy notification command.\n");
			break;
		case NETWORK_ERROR_NET_RECV_MASK:
			err_msg("       The network function net_recv_uint32() failed to receive mask.\n");
			break;
		case NETWORK_ERROR_NET_RECV_STRING:
			err_msg("       The network function net_recv_string failed.\n");
			break;
	}
	fflush(stderr);
}
