/*
 * liberror - A library of common error messages for UNIX.
 * Copyright (C) 2004  Gabriel Munoz <gabriel@xusia.net>
 *
 * Based on example code in "UNIX Network Programming, Volume 1, 3rd
 * Edition" by W. Richard Stevens, Bill Fenner, and Andrew M. Rudoff.

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

#if HAVE_SYSLOG_H
#  include <syslog.h>
#endif

#include <stdarg.h>
#include <stdio.h>

#include "error.h"

#define MAXLINE    4096  /* Max text line length */

int daemon_proc;  /* set nonzero by daemon_init() */

static void
err_doit(int, int, const char *, va_list);

/* Nonfatal error related to system call
 * Print message and return */
void
err_ret(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	err_doit(1, LOG_INFO, fmt, ap);
	va_end(ap);
	return;
}

/* Fatal error related to system call
 * Print message and terminate */
void
err_sys(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	err_doit(1, LOG_ERR, fmt, ap);
	va_end(ap);
	exit(1);
}

/* Fatal error related to system call
 * Print message, dump core, and terminate */
void
err_dump(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	err_doit(1, LOG_ERR, fmt, ap);
	va_end(ap);
	abort();  /* dump core and terminate */
	exit(1);  /* shouldn't get here */
}

/* Nonfatal error unrelated to system call
 * Print message and return */
void
err_msg(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	err_doit(0, LOG_INFO, fmt, ap);
	va_end(ap);
	return;
}

/* Fatal error unrelated to system call
 * Print message and terminate */
void
err_quit(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	err_doit(0, LOG_ERR, fmt, ap);
	va_end(ap);
	exit(1);
}

/* Print message and return to caller
 * Caller specifies "errnoflag" and "level" */
static void
err_doit(int errnoflag, int level, const char *fmt, va_list ap)
{
	int errno_save, n;
	char buf[MAXLINE + 1];

	memset(buf, 0, MAXLINE + 1);

	errno_save = errno;  /* value caller might want printed */
#ifdef HAVE_VSNPRINTF
	vsnprintf(buf, MAXLINE, fmt, ap);  /* safe */
#else
	vsprintf(buf, fmt, ap);            /* not safe */
#endif
	n = strlen(buf);
	if (errnoflag)
		snprintf(buf + n, MAXLINE - n, ": %s", strerror(errno_save));

	if (daemon_proc) {
		syslog(level, buf);
	} else {
		fflush(stdout);  /* in case stdout and stderr are the same */
		fputs(buf, stderr);
		fflush(stderr);
	}
	return;
}

void
err_accept(int err)
{
	err_msg("error: The accept(2) system call failed.\n");
	switch (err) {
		case ETIME:
			err_msg("       The value ERESTARTSYS may be seen during a trace.\n");
			break;
		case EFAULT:
			err_msg("       The addr argument is not in a writable part of the user\n       address space.\n");
			break;
		case ENOTSOCK:
			err_msg("       The descriptor references a file, not a socket.\n");
			break;
		case EINTR:
			err_msg("       The system call was interrupted by a signal that was\n       caught before a valid connection arrived.\n");
			break;
		case EPERM:
			err_msg("       Firewall rules forbid connection. .PP In addition, network\n       errors for the new socket and as defined for the protocol\n       may be returned. Various Linux kernels can return other\n       errors such as\n");
			break;
		case EPROTO:
			err_msg("       Protocol error. .PP Linux accept () may fail if:\n");
			err_msg("                OR\n");
			err_msg("       \n");
			break;
		case EMFILE:
			err_msg("       The per-process limit of open file descriptors has been\n       reached.\n");
			break;
		case EINVAL:
			err_msg("       Socket is not listening for connections.\n");
			break;
		case EAGAIN:
			err_msg("       The socket is marked non-blocking and no connections are\n       present to be accepted.\n");
			break;
		case EBADF:
			err_msg("       The descriptor is invalid.\n");
			break;
		case EOPNOTSUPP:
			err_msg("       The referenced socket is not of type SOCK_STREAM . .PP\n       accept () may fail if:\n");
			break;
		case ECONNABORTED:
			err_msg("       A connection has been aborted.\n");
			break;
		case ENOSR:
			err_msg("       ESOCKTNOSUPPORT ,\n");
			break;
		case ENFILE:
			err_msg("       The system limit on the total number of open files has\n       been reached.\n");
			break;
		case ENOBUFS:
			err_msg("       Not enough free memory. This often means that the memory\n       allocation is limited by the socket buffer limits, not by\n       the system memory.\n");
			break;
		default:
			err_msg("       Unknown errno %d for the accept(2) system call.\n", err);
			break;
	}
}

void
err_bind(int err)
{
	err_msg("error: The bind(2) system call failed.\n");
	switch (err) {
		case ENAMETOOLONG:
			err_msg("       my_addr is too long.\n");
			break;
		case EFAULT:
			err_msg("       my_addr points outside the user's accessible address\n       space.\n");
			break;
		case ENOTSOCK:
			err_msg("       Argument is a descriptor for a file, not a socket. .PP The\n       following errors are specific to UNIX domain ( AF_UNIX )\n       sockets:\n");
			break;
		case EADDRINUSE:
			err_msg("       The given address is already in use.\n");
			break;
		case ENOTDIR:
			err_msg("       A component of the path prefix is not a directory.\n");
			break;
		case EACCES:
			err_msg("       The address is protected, and the user is not the\n       superuser.\n");
			err_msg("                OR\n");
			err_msg("       Search permission is denied on a component of the path\n       prefix. (See also path_resolution (2).)\n");
			break;
		case EROFS:
			err_msg("       The socket inode would reside on a read-only file system.\n");
			break;
		case ENOMEM:
			err_msg("       Insufficient kernel memory was available.\n");
			break;
		case ENOENT:
			err_msg("       The file does not exist.\n");
			break;
		case EINVAL:
			err_msg("       The socket is already bound to an address.\n");
			err_msg("                OR\n");
			err_msg("       The addrlen is wrong, or the socket was not in the AF_UNIX\n       family.\n");
			break;
		case EBADF:
			err_msg("       sockfd is not a valid descriptor.\n");
			break;
		case ELOOP:
			err_msg("       Too many symbolic links were encountered in resolving\n       my_addr .\n");
			break;
		default:
			err_msg("       Unknown errno %d for the bind(2) system call.\n", err);
			break;
	}
}

void
err_chdir(int err)
{
	err_msg("error: The chdir(2) system call failed.\n");
	switch (err) {
		case ENAMETOOLONG:
			err_msg("       path is too long.\n");
			break;
		case EIO:
			err_msg("       An I/O error occurred.\n");
			break;
		case EFAULT:
			err_msg("       path points outside your accessible address space.\n");
			break;
		case ENOTDIR:
			err_msg("       A component of path is not a directory. .PP The general\n       errors for fchdir () are listed below:\n");
			break;
		case EACCES:
			err_msg("       Search permission is denied for one of the directories in\n       the path prefix of path . (See also path_resolution (2).)\n");
			err_msg("                OR\n");
			err_msg("       Search permission was denied on the directory open on fd .\n");
			break;
		case ENOMEM:
			err_msg("       Insufficient kernel memory was available.\n");
			break;
		case ENOENT:
			err_msg("       The file does not exist.\n");
			break;
		case EBADF:
			err_msg("       fd is not a valid file descriptor.\n");
			break;
		case ELOOP:
			err_msg("       Too many symbolic links were encountered in resolving path\n       .\n");
			break;
		default:
			err_msg("       Unknown errno %d for the chdir(2) system call.\n", err);
			break;
	}
}

void
err_chmod(int err)
{
	err_msg("error: The chmod(2) system call failed.\n");
	switch (err) {
		case ENAMETOOLONG:
			err_msg("       path is too long.\n");
			break;
		case EIO:
			err_msg("       An I/O error occurred.\n");
			err_msg("                OR\n");
			err_msg("       See above.\n");
			break;
		case EFAULT:
			err_msg("       path points outside your accessible address space.\n");
			break;
		case ENOTDIR:
			err_msg("       A component of the path prefix is not a directory.\n");
			break;
		case EPERM:
			err_msg("       The effective UID does not match the owner of the file,\n       and the process is not privileged (Linux: it does not have\n       the CAP_FOWNER capability).\n");
			err_msg("                OR\n");
			err_msg("       See above.\n");
			break;
		case EACCES:
			err_msg("       Search permission is denied on a component of the path\n       prefix. (See also path_resolution (2).)\n");
			break;
		case EROFS:
			err_msg("       The named file resides on a read-only file system. .PP The\n       general errors for fchmod () are listed below:\n");
			err_msg("                OR\n");
			err_msg("       See above.\n");
			break;
		case ENOMEM:
			err_msg("       Insufficient kernel memory was available.\n");
			break;
		case ENOENT:
			err_msg("       The file does not exist.\n");
			break;
		case EBADF:
			err_msg("       The file descriptor fildes is not valid.\n");
			break;
		case ELOOP:
			err_msg("       Too many symbolic links were encountered in resolving path\n       .\n");
			break;
		default:
			err_msg("       Unknown errno %d for the chmod(2) system call.\n", err);
			break;
	}
}

void
err_close(int err)
{
	err_msg("error: The close(2) system call failed.\n");
	switch (err) {
		case EIO:
			err_msg("       An I/O error occurred.\n");
			break;
		case EINTR:
			err_msg("       The close () call was interrupted by a signal.\n");
			break;
		case EBADF:
			err_msg("       fd isn't a valid open file descriptor.\n");
			break;
		default:
			err_msg("       Unknown errno %d for the close(2) system call.\n", err);
			break;
	}
}

void
err_connect(int err)
{
	err_msg("error: The connect(2) system call failed.\n");
	switch (err) {
		case ETIME:
			err_msg("       Timeout while attempting connection. The server may be too\n       busy to accept new connections. Note that for IP sockets\n       the timeout may be very long when syncookies are enabled\n       on the server.\n");
			break;
		case ENOTSOCK:
			err_msg("       The file descriptor is not associated with a socket.\n");
			break;
		case EFAULT:
			err_msg("       The socket structure address is outside the user's address\n       space.\n");
			break;
		case EINTR:
			err_msg("       The system call was interrupted by a signal that was\n       caught.\n");
			break;
		case EADDRINUSE:
			err_msg("       Local address is already in use.\n");
			break;
		case ENETUNREACH:
			err_msg("       Network is unreachable.\n");
			break;
		case ECONNREFUSED:
			err_msg("       No one listening on the remote address.\n");
			break;
		case EACCES:
			err_msg("       For Unix domain sockets, which are identified by pathname:\n       Write permission is denied on the socket file, or search\n       permission is denied for one of the directories in the\n       path prefix. (See also path_resolution (2).)\n");
			err_msg("                OR\n");
			err_msg("       The user tried to connect to a broadcast address without\n       having the socket broadcast flag enabled or the connection\n       request failed because of a local firewall rule.\n");
			break;
		case EAFNOSUPPORT:
			err_msg("       The passed address didn't have the correct address family\n       in its sa_family field.\n");
			break;
		case EISCONN:
			err_msg("       The socket is already connected.\n");
			break;
		case EINPROGRESS:
			err_msg("       The socket is non-blocking and the connection cannot be\n       completed immediately. It is possible to select (2) or\n       poll (2) for completion by selecting the socket for\n       writing. After select (2) indicates writability, use\n       getsockopt (2) to read the SO_ERROR option at level\n       SOL_SOCKET to determine whether connect () completed\n       successfully ( SO_ERROR is zero) or unsuccessfully (\n       SO_ERROR is one of the usual error codes listed here,\n       explaining the reason for the failure).\n");
			break;
		case EAGAIN:
			err_msg("       No more free local ports or insufficient entries in the\n       routing cache. For PF_INET see the\n       net.ipv4.ip_local_port_range sysctl in ip (7) on how to\n       increase the number of local ports.\n");
			break;
		case EBADF:
			err_msg("       The file descriptor is not a valid index in the descriptor\n       table.\n");
			break;
		case EALREADY:
			err_msg("       The socket is non-blocking and a previous connection\n       attempt has not yet been completed.\n");
			break;
		default:
			err_msg("       Unknown errno %d for the connect(2) system call.\n", err);
			break;
	}
}

void
err_dup2(int err)
{
	err_msg("error: The dup2(2) system call failed.\n");
	switch (err) {
		case EINTR:
			err_msg("       The dup2 () call was interrupted by a signal.\n");
			break;
		case EMFILE:
			err_msg("       The process already has the maximum number of file\n       descriptors open and tried to open a new one.\n");
			break;
		case EBADF:
			err_msg("       oldfd isn't an open file descriptor, or newfd is out of\n       the allowed range for file descriptors.\n");
			break;
		case EBUSY:
			err_msg("       (Linux only) This may be returned by dup2 () during a race\n       condition with open () and dup ().\n");
			break;
		default:
			err_msg("       Unknown errno %d for the dup2(2) system call.\n", err);
			break;
	}
}

void
err_execve(int err)
{
	err_msg("error: The execve(2) system call failed.\n");
	switch (err) {
		case ETXTBSY:
			err_msg("       Executable was open for writing by one or more processes.\n");
			break;
		case ENAMETOOLONG:
			err_msg("       filename is too long.\n");
			break;
		case EIO:
			err_msg("       An I/O error occurred.\n");
			break;
		case EFAULT:
			err_msg("       filename points outside your accessible address space.\n");
			break;
		case ENOEXEC:
			err_msg("       An executable is not in a recognised format, is for the\n       wrong architecture, or has some other format error that\n       means it cannot be executed.\n");
			break;
		case EISDIR:
			err_msg("       An ELF interpreter was a directory.\n");
			break;
		case E2BIG:
			err_msg("       The total number of bytes in the environment ( envp ) and\n       argument list ( argv ) is too large.\n");
			break;
		case ENOTDIR:
			err_msg("       A component of the path prefix of filename or a script or\n       ELF interpreter is not a directory.\n");
			break;
		case EPERM:
			err_msg("       The file system is mounted nosuid , the user is not the\n       superuser, and the file has an SUID or SGID bit set.\n");
			err_msg("                OR\n");
			err_msg("       The process is being traced, the user is not the superuser\n       and the file has an SUID or SGID bit set.\n");
			break;
		case EACCES:
			err_msg("       Search permission is denied on a component of the path\n       prefix of filename or the name of a script interpreter.\n       (See also path_resolution (2).)\n");
			err_msg("                OR\n");
			err_msg("       The file or a script interpreter is not a regular file.\n");
			err_msg("                OR\n");
			err_msg("       Execute permission is denied for the file or a script or\n       ELF interpreter.\n");
			err_msg("                OR\n");
			err_msg("       The file system is mounted noexec .\n");
			break;
		case ENOMEM:
			err_msg("       Insufficient kernel memory was available.\n");
			break;
		case EMFILE:
			err_msg("       The process has the maximum number of files open.\n");
			break;
		case ENOENT:
			err_msg("       The file filename or a script or ELF interpreter does not\n       exist, or a shared library needed for file or interpreter\n       cannot be found.\n");
			break;
		case EINVAL:
			err_msg("       An ELF executable had more than one PT_INTERP segment\n       (i.e., tried to name more than one interpreter).\n");
			break;
		case ENFILE:
			err_msg("       The system limit on the total number of open files has\n       been reached.\n");
			break;
		case ELOOP:
			err_msg("       Too many symbolic links were encountered in resolving\n       filename or the name of a script or ELF interpreter.\n");
			break;
		case ELIBBAD:
			err_msg("       An ELF interpreter was not in a recognised format.\n");
			break;
		default:
			err_msg("       Unknown errno %d for the execve(2) system call.\n", err);
			break;
	}
}

void
err_fork(int err)
{
	err_msg("error: The fork(2) system call failed.\n");
	switch (err) {
		case ENOMEM:
			err_msg("       fork () failed to allocate the necessary kernel structures\n       because memory is tight.\n");
			break;
		case EAGAIN:
			err_msg("       fork () cannot allocate sufficient memory to copy the\n       parent's page tables and allocate a task structure for the\n       child.\n");
			err_msg("                OR\n");
			err_msg("       It was not possible to create a new process because the\n       caller's RLIMIT_NPROC resource limit was encountered. To\n       exceed this limit, the process must have either the\n       CAP_SYS_ADMIN or the CAP_SYS_RESOURCE capability.\n");
			break;
		default:
			err_msg("       Unknown errno %d for the fork(2) system call.\n", err);
			break;
	}
}

void
err_fstat(int err)
{
	err_msg("error: The fstat(2) system call failed.\n");
	switch (err) {
		case ENAMETOOLONG:
			err_msg("       File name too long.\n");
			break;
		case EFAULT:
			err_msg("       Bad address.\n");
			break;
		case ENOTDIR:
			err_msg("       A component of the path is not a directory.\n");
			break;
		case EACCES:
			err_msg("       Search permission is denied for one of the directories in\n       the path prefix of path . (See also path_resolution (2).)\n");
			break;
		case ENOMEM:
			err_msg("       Out of memory (i.e. kernel memory).\n");
			break;
		case ENOENT:
			err_msg("       A component of the path path does not exist, or the path\n       is an empty string.\n");
			break;
		case EBADF:
			err_msg("       filedes is bad.\n");
			break;
		case ELOOP:
			err_msg("       Too many symbolic links encountered while traversing the\n       path.\n");
			break;
		default:
			err_msg("       Unknown errno %d for the fstat(2) system call.\n", err);
			break;
	}
}

void
err_gethostname(int err)
{
	err_msg("error: The gethostname(2) system call failed.\n");
	switch (err) {
		case EFAULT:
			err_msg("       name is an invalid address.\n");
			break;
		case EPERM:
			err_msg("       For sethostname (), the caller did not have the\n       CAP_SYS_ADMIN capability.\n");
			break;
		case EINVAL:
			err_msg("       len is negative or, for sethostname (), len is larger than\n       the maximum allowed size, or, for gethostname () on\n       Linux/i386, len is smaller than the actual size. (In this\n       last case glibc 2.1 uses ENAMETOOLONG.)\n");
			break;
		default:
			err_msg("       Unknown errno %d for the gethostname(2) system call.\n", err);
			break;
	}
}

void
err_getsockname(int err)
{
	err_msg("error: The getsockname(2) system call failed.\n");
	switch (err) {
		case ENOTSOCK:
			err_msg("       The argument s is a file, not a socket.\n");
			break;
		case EFAULT:
			err_msg("       The name parameter points to memory not in a valid part of\n       the process address space.\n");
			break;
		case EBADF:
			err_msg("       The argument s is not a valid descriptor.\n");
			break;
		case ENOBUFS:
			err_msg("       Insufficient resources were available in the system to\n       perform the operation.\n");
			break;
		default:
			err_msg("       Unknown errno %d for the getsockname(2) system call.\n", err);
			break;
	}
}

void
err_kill(int err)
{
	err_msg("error: The kill(2) system call failed.\n");
	switch (err) {
		case ESRCH:
			err_msg("       The pid or process group does not exist. Note that an\n       existing process might be a zombie, a process which\n       already committed termination, but has not yet been\n       waitfP()ed for.\n");
			break;
		case EPERM:
			err_msg("       The process does not have permission to send the signal to\n       any of the target processes.\n");
			break;
		case EINVAL:
			err_msg("       An invalid signal was specified.\n");
			break;
		default:
			err_msg("       Unknown errno %d for the kill(2) system call.\n", err);
			break;
	}
}

void
err_listen(int err)
{
	err_msg("error: The listen(2) system call failed.\n");
	switch (err) {
		case ENOTSOCK:
			err_msg("       The argument sockfd is not a socket.\n");
			break;
		case EADDRINUSE:
			err_msg("       Another socket is already listening on the same port.\n");
			break;
		case EBADF:
			err_msg("       The argument sockfd is not a valid descriptor.\n");
			break;
		case EOPNOTSUPP:
			err_msg("       The socket is not of a type that supports the listen ()\n       operation.\n");
			break;
		default:
			err_msg("       Unknown errno %d for the listen(2) system call.\n", err);
			break;
	}
}

void
err_lstat(int err)
{
	err_msg("error: The lstat(2) system call failed.\n");
	switch (err) {
		case ENAMETOOLONG:
			err_msg("       File name too long.\n");
			break;
		case EFAULT:
			err_msg("       Bad address.\n");
			break;
		case ENOTDIR:
			err_msg("       A component of the path is not a directory.\n");
			break;
		case EACCES:
			err_msg("       Search permission is denied for one of the directories in\n       the path prefix of path . (See also path_resolution (2).)\n");
			break;
		case ENOMEM:
			err_msg("       Out of memory (i.e. kernel memory).\n");
			break;
		case ENOENT:
			err_msg("       A component of the path path does not exist, or the path\n       is an empty string.\n");
			break;
		case EBADF:
			err_msg("       filedes is bad.\n");
			break;
		case ELOOP:
			err_msg("       Too many symbolic links encountered while traversing the\n       path.\n");
			break;
		default:
			err_msg("       Unknown errno %d for the lstat(2) system call.\n", err);
			break;
	}
}

void
err_mmap(int err)
{
	err_msg("error: The mmap(2) system call failed.\n");
	switch (err) {
		case ETXTBSY:
			err_msg("       MAP_DENYWRITE was set but the object specified by fd is\n       open for writing. .LP Use of a mapped region can result in\n       these signals:\n");
			break;
		case EPERM:
			err_msg("       The prot argument asks for PROT_EXEC but the mapped area\n       belongs to a file on a filesystem that was mounted\n       no-exec.\n");
			break;
		case EACCES:
			err_msg("       A file descriptor refers to a non-regular file. Or\n       MAP_PRIVATE was requested, but fd is not open for reading.\n       Or MAP_SHARED was requested and PROT_WRITE is set, but fd\n       is not open in read/write (O_RDWR) mode. Or PROT_WRITE is\n       set, but the file is append-only.\n");
			break;
		case ENOMEM:
			err_msg("       No memory is available, or the process's maximum number of\n       mappings would have been exceeded.\n");
			break;
		case ENODEV:
			err_msg("       The underlying filesystem of the specified file does not\n       support memory mapping.\n");
			break;
		case EINVAL:
			err_msg("       We don't like start or length or offset . (E.g., they are\n       too large, or not aligned on a PAGESIZE boundary.)\n");
			break;
		case EAGAIN:
			err_msg("       The file has been locked, or too much memory has been\n       locked (see setrlimit (2)).\n");
			break;
		case EBADF:
			err_msg("       fd is not a valid file descriptor (and MAP_ANONYMOUS was\n       not set).\n");
			break;
		case ENFILE:
			err_msg("       The system limit on the total number of open files has\n       been reached.\n");
			break;
		default:
			err_msg("       Unknown errno %d for the mmap(2) system call.\n", err);
			break;
	}
}

void
err_open(int err)
{
	err_msg("error: The open(2) system call failed.\n");
	switch (err) {
		case ETXTBSY:
			err_msg("       pathname refers to an executable image which is currently\n       being executed and write access was requested.\n");
			break;
		case EOVERFLOW:
			err_msg("       pathname refers to a regular file, too large to be opened;\n       see O_LARGEFILE above.\n");
			break;
		case ENOSPC:
			err_msg("       pathname was to be created but the device containing\n       pathname has no room for the new file.\n");
			break;
		case ENAMETOOLONG:
			err_msg("       pathname was too long.\n");
			break;
		case EFAULT:
			err_msg("       pathname points outside your accessible address space.\n");
			break;
		case EISDIR:
			err_msg("       pathname refers to a directory and the access requested\n       involved writing (that is, O_WRONLY or O_RDWR is set).\n");
			break;
		case ENOTDIR:
			err_msg("       A component used as a directory in pathname is not, in\n       fact, a directory, or O_DIRECTORY was specified and\n       pathname was not a directory.\n");
			break;
		case EPERM:
			err_msg("       The O_NOATIME flag was specified, but the effective user\n       ID of the caller did not match the owner of the file and\n       the caller was not privileged ( CAP_FOWNER ).\n");
			break;
		case ENXIO:
			err_msg("       O_NONBLOCK | O_WRONLY is set, the named file is a FIFO and\n       no process has the file open for reading. Or, the file is\n       a device special file and no corresponding device exists.\n");
			break;
		case EACCES:
			err_msg("       The requested access to the file is not allowed, or search\n       permission is denied for one of the directories in the\n       path prefix of pathname , or the file did not exist yet\n       and write access to the parent directory is not allowed.\n       (See also path_resolution (2).)\n");
			break;
		case EROFS:
			err_msg("       pathname refers to a file on a read-only filesystem and\n       write access was requested.\n");
			break;
		case ENOMEM:
			err_msg("       Insufficient kernel memory was available.\n");
			break;
		case ENODEV:
			err_msg("       pathname refers to a device special file and no\n       corresponding device exists. (This is a Linux kernel bug;\n       in this situation ENXIO must be returned.)\n");
			break;
		case EMFILE:
			err_msg("       The process already has the maximum number of files open.\n");
			break;
		case ENOENT:
			err_msg("       O_CREAT is not set and the named file does not exist. Or,\n       a directory component in pathname does not exist or is a\n       dangling symbolic link.\n");
			break;
		case EWOULDBLOCK:
			err_msg("       The O_NONBLOCK flag was specified, and an incompatible\n       lease was held on the file (see fcntl (2)).\n");
			break;
		case EEXIST:
			err_msg("       pathname already exists and O_CREAT and O_EXCL were used.\n");
			break;
		case ENFILE:
			err_msg("       The system limit on the total number of open files has\n       been reached.\n");
			break;
		case ELOOP:
			err_msg("       Too many symbolic links were encountered in resolving\n       pathname , or O_NOFOLLOW was specified but pathname was a\n       symbolic link.\n");
			break;
		default:
			err_msg("       Unknown errno %d for the open(2) system call.\n", err);
			break;
	}
}

void
err_pipe(int err)
{
	err_msg("error: The pipe(2) system call failed.\n");
	switch (err) {
		case EFAULT:
			err_msg("       filedes is not valid.\n");
			break;
		case EMFILE:
			err_msg("       Too many file descriptors are in use by the process.\n");
			break;
		case ENFILE:
			err_msg("       The system limit on the total number of open files has\n       been reached.\n");
			break;
		default:
			err_msg("       Unknown errno %d for the pipe(2) system call.\n", err);
			break;
	}
}

void
err_read(int err)
{
	err_msg("error: The read(2) system call failed.\n");
	switch (err) {
		case EIO:
			err_msg("       I/O error. This will happen for example when the process\n       is in a background process group, tries to read from its\n       controlling tty, and either it is ignoring or blocking\n       SIGTTIN or its process group is orphaned. It may also\n       occur when there is a low-level I/O error while reading\n       from a disk or tape.\n");
			break;
		case EFAULT:
			err_msg("       buf is outside your accessible address space.\n");
			break;
		case EISDIR:
			err_msg("       fd refers to a directory. .PP Other errors may occur,\n       depending on the object connected to fd . POSIX allows a\n       read () that is interrupted after reading some data to\n       return -1 (with errno set to EINTR) or to return the\n       number of bytes already read.\n");
			break;
		case EINTR:
			err_msg("       The call was interrupted by a signal before any data was\n       read.\n");
			break;
		case EINVAL:
			err_msg("       fd is attached to an object which is unsuitable for\n       reading; or the file was opened with the O_DIRECT flag,\n       and either the address specified in buf , the value\n       specified in count , or the current file offset is not\n       suitably aligned.\n");
			break;
		case EAGAIN:
			err_msg("       Non-blocking I/O has been selected using O_NONBLOCK and no\n       data was immediately available for reading.\n");
			break;
		case EBADF:
			err_msg("       fd is not a valid file descriptor or is not open for\n       reading.\n");
			break;
		default:
			err_msg("       Unknown errno %d for the read(2) system call.\n", err);
			break;
	}
}

void
err_recvfrom(int err)
{
	err_msg("error: The recvfrom(2) system call failed.\n");
	switch (err) {
		case ENOTSOCK:
			err_msg("       The argument s does not refer to a socket.\n");
			break;
		case EFAULT:
			err_msg("       The receive buffer pointer(s) point outside the process's\n       address space.\n");
			break;
		case EINTR:
			err_msg("       The receive was interrupted by delivery of a signal before\n       any data were available.\n");
			break;
		case ECONNREFUSED:
			err_msg("       A remote host refused to allow the network connection\n       (typically because it is not running the requested\n       service).\n");
			break;
		case ENOMEM:
			err_msg("       Could not allocate memory for recvmsg ().\n");
			break;
		case EINVAL:
			err_msg("       Invalid argument passed.\n");
			break;
		case EAGAIN:
			err_msg("       The socket is marked non-blocking and the receive\n       operation would block, or a receive timeout had been set\n       and the timeout expired before data was received.\n");
			break;
		case EBADF:
			err_msg("       The argument s is an invalid descriptor.\n");
			break;
		case ENOTCONN:
			err_msg("       The socket is associated with a connection-oriented\n       protocol and has not been connected (see connect (2) and\n       accept (2)).\n");
			break;
		default:
			err_msg("       Unknown errno %d for the recvfrom(2) system call.\n", err);
			break;
	}
}

void
err_recvmsg(int err)
{
	err_msg("error: The recvmsg(2) system call failed.\n");
	switch (err) {
		case ENOTSOCK:
			err_msg("       The argument s does not refer to a socket.\n");
			break;
		case EFAULT:
			err_msg("       The receive buffer pointer(s) point outside the process's\n       address space.\n");
			break;
		case EINTR:
			err_msg("       The receive was interrupted by delivery of a signal before\n       any data were available.\n");
			break;
		case ECONNREFUSED:
			err_msg("       A remote host refused to allow the network connection\n       (typically because it is not running the requested\n       service).\n");
			break;
		case ENOMEM:
			err_msg("       Could not allocate memory for recvmsg ().\n");
			break;
		case EINVAL:
			err_msg("       Invalid argument passed.\n");
			break;
		case EAGAIN:
			err_msg("       The socket is marked non-blocking and the receive\n       operation would block, or a receive timeout had been set\n       and the timeout expired before data was received.\n");
			break;
		case EBADF:
			err_msg("       The argument s is an invalid descriptor.\n");
			break;
		case ENOTCONN:
			err_msg("       The socket is associated with a connection-oriented\n       protocol and has not been connected (see connect (2) and\n       accept (2)).\n");
			break;
		default:
			err_msg("       Unknown errno %d for the recvmsg(2) system call.\n", err);
			break;
	}
}

void
err_sendmsg(int err)
{
	err_msg("error: The sendmsg(2) system call failed.\n");
	switch (err) {
		case ENOTSOCK:
			err_msg("       The argument s is not a socket.\n");
			break;
		case EMSGSIZE:
			err_msg("       The socket type requires that message be sent atomically,\n       and the size of the message to be sent made this\n       impossible.\n");
			break;
		case EFAULT:
			err_msg("       An invalid user space address was specified for a\n       parameter.\n");
			break;
		case EPIPE:
			err_msg("       The local end has been shut down on a connection oriented\n       socket. In this case the process will also receive a\n       SIGPIPE unless MSG_NOSIGNAL is set.\n");
			break;
		case EINTR:
			err_msg("       A signal occurred before any data was transmitted.\n");
			break;
		case ECONNRESET:
			err_msg("       Connection reset by peer.\n");
			break;
		case EACCES:
			err_msg("       (For Unix domain sockets, which are identified by\n       pathname) Write permission is denied on the destination\n       socket file, or search permission is denied for one of the\n       directories the path prefix. (See path_resolution (2).)\n");
			break;
		case ENOMEM:
			err_msg("       No memory available.\n");
			break;
		case EISCONN:
			err_msg("       The connection-mode socket was connected already but a\n       recipient was specified. (Now either this error is\n       returned, or the recipient specification is ignored.)\n");
			break;
		case EINVAL:
			err_msg("       Invalid argument passed.\n");
			break;
		case EAGAIN:
			err_msg("       The socket is marked non-blocking and the requested\n       operation would block.\n");
			break;
		case EBADF:
			err_msg("       An invalid descriptor was specified.\n");
			break;
		case EOPNOTSUPP:
			err_msg("       Some bit in the flags argument is inappropriate for the\n       socket type.\n");
			break;
		case ENOTCONN:
			err_msg("       The socket is not connected, and no target has been given.\n");
			break;
		case ENOBUFS:
			err_msg("       The output queue for a network interface was full. This\n       generally indicates that the interface has stopped\n       sending, but may be caused by transient congestion.\n       (Normally, this does not occur in Linux. Packets are just\n       silently dropped when a device queue overflows.)\n");
			break;
		case EDESTADDRREQ:
			err_msg("       The socket is not connection-mode, and no peer address is\n       set.\n");
			break;
		default:
			err_msg("       Unknown errno %d for the sendmsg(2) system call.\n", err);
			break;
	}
}

void
err_sendto(int err)
{
	err_msg("error: The sendto(2) system call failed.\n");
	switch (err) {
		case ENOTSOCK:
			err_msg("       The argument s is not a socket.\n");
			break;
		case EMSGSIZE:
			err_msg("       The socket type requires that message be sent atomically,\n       and the size of the message to be sent made this\n       impossible.\n");
			break;
		case EFAULT:
			err_msg("       An invalid user space address was specified for a\n       parameter.\n");
			break;
		case EPIPE:
			err_msg("       The local end has been shut down on a connection oriented\n       socket. In this case the process will also receive a\n       SIGPIPE unless MSG_NOSIGNAL is set.\n");
			break;
		case EINTR:
			err_msg("       A signal occurred before any data was transmitted.\n");
			break;
		case ECONNRESET:
			err_msg("       Connection reset by peer.\n");
			break;
		case EACCES:
			err_msg("       (For Unix domain sockets, which are identified by\n       pathname) Write permission is denied on the destination\n       socket file, or search permission is denied for one of the\n       directories the path prefix. (See path_resolution (2).)\n");
			break;
		case ENOMEM:
			err_msg("       No memory available.\n");
			break;
		case EISCONN:
			err_msg("       The connection-mode socket was connected already but a\n       recipient was specified. (Now either this error is\n       returned, or the recipient specification is ignored.)\n");
			break;
		case EINVAL:
			err_msg("       Invalid argument passed.\n");
			break;
		case EAGAIN:
			err_msg("       The socket is marked non-blocking and the requested\n       operation would block.\n");
			break;
		case EBADF:
			err_msg("       An invalid descriptor was specified.\n");
			break;
		case EOPNOTSUPP:
			err_msg("       Some bit in the flags argument is inappropriate for the\n       socket type.\n");
			break;
		case ENOTCONN:
			err_msg("       The socket is not connected, and no target has been given.\n");
			break;
		case ENOBUFS:
			err_msg("       The output queue for a network interface was full. This\n       generally indicates that the interface has stopped\n       sending, but may be caused by transient congestion.\n       (Normally, this does not occur in Linux. Packets are just\n       silently dropped when a device queue overflows.)\n");
			break;
		case EDESTADDRREQ:
			err_msg("       The socket is not connection-mode, and no peer address is\n       set.\n");
			break;
		default:
			err_msg("       Unknown errno %d for the sendto(2) system call.\n", err);
			break;
	}
}

void
err_shmat(int err)
{
	err_msg("error: The shmat(2) system call failed.\n");
	switch (err) {
		case EACCES:
			err_msg("       The calling process does not have the required permissions\n       for the requested attach type, and does not have the\n       CAP_IPC_OWNER capability.\n");
			break;
		case ENOMEM:
			err_msg("       Could not allocate memory for the descriptor or for the\n       page tables. .PP shmdt () can fail only if there is no\n       shared memory segment attached at shmaddr ; in this case,\n       errno will be set to\n");
			break;
		case EINVAL:
			err_msg("       Invalid shmid value, unaligned (i.e., not page-aligned and\n       SHM_RNDfP was not specified) or invalid shmaddr value, or\n       failing attach at brk (), or SHM_REMAP was specified and\n       shmaddr was NULL.\n");
			err_msg("                OR\n");
			err_msg("       \n");
			break;
		default:
			err_msg("       Unknown errno %d for the shmat(2) system call.\n", err);
			break;
	}
}

void
err_shmctl(int err)
{
	err_msg("error: The shmctl(2) system call failed.\n");
	switch (err) {
		case EOVERFLOW:
			err_msg("       IPC_STATfP is attempted, and the GID or UID value is too\n       large to be stored in the structure pointed to by buf .\n");
			break;
		case EFAULT:
			err_msg("       The argument cmd has value IPC_SET or IPC_STAT but the\n       address pointed to by buf isn't accessible.\n");
			break;
		case EPERM:
			err_msg("       IPC_SETfP or IPC_RMIDfP is attempted, and the effective\n       user ID of the calling process is not that of the creator\n       (found in shm_perm.cuid ), or the owner (found in\n       shm_perm.uid ), and the process was not privileged (Linux:\n       did not have the CAP_SYS_ADMIN capability). Or (in kernels\n       before 2.6.9), SHM_LOCK or SHM_UNLOCK was specified, but\n       the process was not privileged (Linux: did not have the\n       CAP_IPC_LOCK capability). (Since Linux 2.6.9, this error\n       can also occur if the RLIMIT_MEMLOCK is 0 and the caller\n       is not privileged.)\n");
			break;
		case EACCES:
			err_msg("       IPC_STATfP or SHM_STATfP is requested and\n       fIshm_perm.modefP does not allow read access for shmid ,\n       and the calling process does not have the CAP_IPC_OWNER\n       capability.\n");
			break;
		case ENOMEM:
			err_msg("       (In kernels since 2.6.9), SHM_LOCK was specified and the\n       size of the to-be-locked segment would mean that the total\n       bytes in locked shared memory segments would exceed the\n       limit for the real user ID of the calling process. This\n       limit is defined by the RLIMIT_MEMLOCK soft resource limit\n       (see setrlimit (2)).\n");
			break;
		case EINVAL:
			err_msg("       fIshmidfP is not a valid identifier, or fIcmdfP is not a\n       valid command. Or: for a SHM_STAT operation, the index\n       value specified in shmid referred to an array slot that is\n       currently unused.\n");
			break;
		case EIDRM:
			err_msg("       fIshmidfP points to a removed identifier.\n");
			break;
		default:
			err_msg("       Unknown errno %d for the shmctl(2) system call.\n", err);
			break;
	}
}

void
err_shmdt(int err)
{
	err_msg("error: The shmdt(2) system call failed.\n");
	switch (err) {
		case EACCES:
			err_msg("       The calling process does not have the required permissions\n       for the requested attach type, and does not have the\n       CAP_IPC_OWNER capability.\n");
			break;
		case ENOMEM:
			err_msg("       Could not allocate memory for the descriptor or for the\n       page tables. .PP shmdt () can fail only if there is no\n       shared memory segment attached at shmaddr ; in this case,\n       errno will be set to\n");
			break;
		case EINVAL:
			err_msg("       Invalid shmid value, unaligned (i.e., not page-aligned and\n       SHM_RNDfP was not specified) or invalid shmaddr value, or\n       failing attach at brk (), or SHM_REMAP was specified and\n       shmaddr was NULL.\n");
			err_msg("                OR\n");
			err_msg("       \n");
			break;
		default:
			err_msg("       Unknown errno %d for the shmdt(2) system call.\n", err);
			break;
	}
}

void
err_shmget(int err)
{
	err_msg("error: The shmget(2) system call failed.\n");
	switch (err) {
		case ENOSPC:
			err_msg("       All possible shared memory IDs have been taken ( SHMMNI ),\n       or allocating a segment of the requested size would cause\n       the system to exceed the system-wide limit on shared\n       memory ( SHMALL ).\n");
			break;
		case EPERM:
			err_msg("       The SHM_HUGETLB flag was specified, but the caller was not\n       privileged (did not have the CAP_IPC_LOCK capability).\n");
			break;
		case EACCES:
			err_msg("       The user does not have permission to access the shared\n       memory segment, and does not have the CAP_IPC_OWNER\n       capability.\n");
			break;
		case ENOMEM:
			err_msg("       No memory could be allocated for segment overhead.\n");
			break;
		case ENOENT:
			err_msg("       No segment exists for the given fIkeyfP, and IPC_CREAT was\n       not specified.\n");
			break;
		case EINVAL:
			err_msg("       A new segment was to be created and fIsizefP < SHMMINfP or\n       fIsizefP > SHMMAXfP, or no new segment was to be created,\n       a segment with given key existed, but fIsizefP is greater\n       than the size of that segment.\n");
			break;
		case EEXIST:
			err_msg("       IPC_CREAT | IPC_EXCL was specified and the segment exists.\n");
			break;
		case ENFILE:
			err_msg("       The system limit on the total number of open files has\n       been reached.\n");
			break;
		default:
			err_msg("       Unknown errno %d for the shmget(2) system call.\n", err);
			break;
	}
}

void
err_sigaction(int err)
{
	err_msg("error: The sigaction(2) system call failed.\n");
	switch (err) {
		case EFAULT:
			err_msg("       act or oldact points to memory which is not a valid part\n       of the process address space.\n");
			break;
		case EINVAL:
			err_msg("       An invalid signal was specified. This will also be\n       generated if an attempt is made to change the action for\n       SIGKILL or SIGSTOP , which cannot be caught or ignored.\n");
			break;
		default:
			err_msg("       Unknown errno %d for the sigaction(2) system call.\n", err);
			break;
	}
}

void
err_sigprocmask(int err)
{
	err_msg("error: The sigprocmask(2) system call failed.\n");
	switch (err) {
		case EINVAL:
			err_msg("       The value specified in how was invalid.\n");
			break;
		default:
			err_msg("       Unknown errno %d for the sigprocmask(2) system call.\n", err);
			break;
	}
}

void
err_socket(int err)
{
	err_msg("error: The socket(2) system call failed.\n");
	switch (err) {
		case EPROTO:
			err_msg("       The protocol type or the specified protocol is not\n       supported within this domain. .PP Other errors may be\n       generated by the underlying protocol modules.\n");
			break;
		case EACCES:
			err_msg("       Permission to create a socket of the specified type and/or\n       protocol is denied.\n");
			break;
		case EMFILE:
			err_msg("       Process file table overflow.\n");
			break;
		case EAFNOSUPPORT:
			err_msg("       The implementation does not support the specified address\n       family.\n");
			break;
		case EINVAL:
			err_msg("       Unknown protocol, or protocol family not available.\n");
			break;
		case ENFILE:
			err_msg("       The system limit on the total number of open files has\n       been reached.\n");
			break;
		case ENOBUFS:
			err_msg("       Insufficient memory is available. The socket cannot be\n       created until sufficient resources are freed.\n");
			break;
		default:
			err_msg("       Unknown errno %d for the socket(2) system call.\n", err);
			break;
	}
}

void
err_stat(int err)
{
	err_msg("error: The stat(2) system call failed.\n");
	switch (err) {
		case ENAMETOOLONG:
			err_msg("       File name too long.\n");
			break;
		case EFAULT:
			err_msg("       Bad address.\n");
			break;
		case ENOTDIR:
			err_msg("       A component of the path is not a directory.\n");
			break;
		case EACCES:
			err_msg("       Search permission is denied for one of the directories in\n       the path prefix of path . (See also path_resolution (2).)\n");
			break;
		case ENOMEM:
			err_msg("       Out of memory (i.e. kernel memory).\n");
			break;
		case ENOENT:
			err_msg("       A component of the path path does not exist, or the path\n       is an empty string.\n");
			break;
		case EBADF:
			err_msg("       filedes is bad.\n");
			break;
		case ELOOP:
			err_msg("       Too many symbolic links encountered while traversing the\n       path.\n");
			break;
		default:
			err_msg("       Unknown errno %d for the stat(2) system call.\n", err);
			break;
	}
}

void
err_wait(int err)
{
	err_msg("error: The wait(2) system call failed.\n");
	switch (err) {
		case EINTR:
			err_msg("       WNOHANG was not set and an unblocked signal or a SIGCHLD\n       was caught.\n");
			break;
		case EINVAL:
			err_msg("       The options argument was invalid.\n");
			break;
		case ECHILD:
			err_msg("       (for wait ()) The calling process does not have any\n       unwaited-for children.\n");
			err_msg("                OR\n");
			err_msg("       (for waitpid () or waitid ()) The process specified by pid\n       ( waitpid ()) or idtype and id ( waitid ()) does not exist\n       or is not a child of the calling process. (This can happen\n       for one's own child if the action for SIGCHLD is set to\n       SIG_IGN. See also the LINUX NOTES section about threads.)\n");
			break;
		default:
			err_msg("       Unknown errno %d for the wait(2) system call.\n", err);
			break;
	}
}

void
err_write(int err)
{
	err_msg("error: The write(2) system call failed.\n");
	switch (err) {
		case ENOSPC:
			err_msg("       The device containing the file referred to by fd has no\n       room for the data.\n");
			break;
		case EIO:
			err_msg("       A low-level I/O error occurred while modifying the inode.\n");
			break;
		case EFAULT:
			err_msg("       buf is outside your accessible address space.\n");
			break;
		case EPIPE:
			err_msg("       fd is connected to a pipe or socket whose reading end is\n       closed. When this happens the writing process will also\n       receive a SIGPIPE signal. (Thus, the write return value is\n       seen only if the program catches, blocks or ignores this\n       signal.) .PP Other errors may occur, depending on the\n       object connected to fd .\n");
			break;
		case EINTR:
			err_msg("       The call was interrupted by a signal before any data was\n       written.\n");
			break;
		case EINVAL:
			err_msg("       fd is attached to an object which is unsuitable for\n       writing; or the file was opened with the O_DIRECT flag,\n       and either the address specified in buf , the value\n       specified in count , or the current file offset is not\n       suitably aligned.\n");
			break;
		case EAGAIN:
			err_msg("       Non-blocking I/O has been selected using O_NONBLOCK and\n       the write would block.\n");
			break;
		case EBADF:
			err_msg("       fd is not a valid file descriptor or is not open for\n       writing.\n");
			break;
		case EFBIG:
			err_msg("       An attempt was made to write a file that exceeds the\n       implementation-defined maximum file size or the process'\n       file size limit, or to write at a position past the\n       maximum allowed offset.\n");
			break;
		default:
			err_msg("       Unknown errno %d for the write(2) system call.\n", err);
			break;
	}
}

void
err_fopen(int err)
{
	err_msg("error: The fopen(3) function call failed.\n");
	switch (err) {
		case EINVAL:
			err_msg("       The mode provided to fopen (), fdopen (), or freopen ()\n       was invalid. .PP The fopen (), fdopen () and freopen ()\n       functions may also fail and set errno for any of the\n       errors specified for the routine malloc (3). .PP The fopen\n       () function may also fail and set errno for any of the\n       errors specified for the routine open (2). .PP The fdopen\n       () function may also fail and set errno for any of the\n       errors specified for the routine fcntl (2). .PP The\n       freopen () function may also fail and set errno for any of\n       the errors specified for the routines open (2), fclose (3)\n       and fflush (3).\n");
			break;
		default:
			err_msg("       Unknown errno %d for the fopen(3) function call.\n", err);
			break;
	}
}

void
err_freopen(int err)
{
	err_msg("error: The freopen(3) function call failed.\n");
	switch (err) {
		case EINVAL:
			err_msg("       The mode provided to fopen (), fdopen (), or freopen ()\n       was invalid. .PP The fopen (), fdopen () and freopen ()\n       functions may also fail and set errno for any of the\n       errors specified for the routine malloc (3). .PP The fopen\n       () function may also fail and set errno for any of the\n       errors specified for the routine open (2). .PP The fdopen\n       () function may also fail and set errno for any of the\n       errors specified for the routine fcntl (2). .PP The\n       freopen () function may also fail and set errno for any of\n       the errors specified for the routines open (2), fclose (3)\n       and fflush (3).\n");
			break;
		default:
			err_msg("       Unknown errno %d for the freopen(3) function call.\n", err);
			break;
	}
}

void
err_getaddrinfo(int err)
{
	err_msg("error: The getaddrinfo(3) function call failed.\n");
	switch (err) {
		default:
			err_msg("       Unknown errno %d for the getaddrinfo(3) function call.\n", err);
			break;
	}
}

void
err_inet_ntop(int err)
{
	err_msg("error: The inet_ntop(3) function call failed.\n");
	switch (err) {
		default:
			err_msg("       Unknown errno %d for the inet_ntop(3) function call.\n", err);
			break;
	}
}

void
err_inet_pton(int err)
{
	err_msg("error: The inet_pton(3) function call failed.\n");
	switch (err) {
		default:
			err_msg("       Unknown errno %d for the inet_pton(3) function call.\n", err);
			break;
	}
}

void
err_malloc(int err)
{
	err_msg("error: The malloc(3) function call failed.\n");
	switch (err) {
		default:
			err_msg("       Unknown errno %d for the malloc(3) function call.\n", err);
			break;
	}
}

void
err_opendir(int err)
{
	err_msg("error: The opendir(3) function call failed.\n");
	switch (err) {
		case ENOTDIR:
			err_msg("       name is not a directory.\n");
			break;
		case EACCES:
			err_msg("       Permission denied.\n");
			break;
		case ENOMEM:
			err_msg("       Insufficient memory to complete the operation.\n");
			break;
		case EMFILE:
			err_msg("       Too many file descriptors in use by process.\n");
			break;
		case ENOENT:
			err_msg("       Directory does not exist, or name is an empty string.\n");
			break;
		case ENFILE:
			err_msg("       Too many files are currently open in the system.\n");
			break;
		default:
			err_msg("       Unknown errno %d for the opendir(3) function call.\n", err);
			break;
	}
}

void
err_sigsetops(int err)
{
	err_msg("error: The sigsetops(3) function call failed.\n");
	switch (err) {
		case EINVAL:
			err_msg("       sig is not a valid signal.\n");
			break;
		default:
			err_msg("       Unknown errno %d for the sigsetops(3) function call.\n", err);
			break;
	}
}

void
err_pthread_attr_setdetachstate(int err)
{
	err_msg("error: The pthread_attr_setdetachstate(3) function call failed.\n");
	switch (err) {
		case EINVAL:
			err_msg("       The value of fIdetachstatefP was not valid .sp .LP These\n       functions shall not return an error code of [EINTR]. .LP\n       fIThe following sections are informative.fP\n");
			break;
		default:
			err_msg("       Unknown errno %d for the pthread_attr_setdetachstate(3) function call.\n", err);
			break;
	}
}

void
err_pthread_create(int err)
{
	err_msg("error: The pthread_create(3) function call failed.\n");
	switch (err) {
		case EPERM:
			err_msg("       The caller does not have appropriate permission to set the\n       required scheduling parameters or scheduling policy. .sp\n       .LP The fIpthread_createfP() function shall not return an\n       error code of [EINTR]. .LP fIThe following sections are\n       informative.fP\n");
			break;
		case EINVAL:
			err_msg("       The value specified by fIattrfP is invalid.\n");
			break;
		case EAGAIN:
			err_msg("       The system lacked the necessary resources to create\n       another thread, or the system-imposed limit on the total\n       number of threads in a process {PTHREAD_THREADS_MAX} would\n       be exceeded.\n");
			break;
		default:
			err_msg("       Unknown errno %d for the pthread_create(3) function call.\n", err);
			break;
	}
}

void
err_pthread_sigmask(int err)
{
	err_msg("error: The pthread_sigmask(3) function call failed.\n");
	switch (err) {
		case EINVAL:
			err_msg("       The value of the fIhowfP argument is not equal to one of\n       the defined values. .sp .LP The fIpthread_sigmaskfP()\n       function shall not return an error code of [EINTR]. .LP\n       fIThe following sections are informative.fP\n");
			break;
		default:
			err_msg("       Unknown errno %d for the pthread_sigmask(3) function call.\n", err);
			break;
	}
}
