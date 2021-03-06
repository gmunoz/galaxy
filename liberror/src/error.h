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

#ifndef LIBERROR_ERROR_H
#define LIBERROR_ERROR_H

#include <errno.h>

/* General error handling functions */
void err_ret(const char *fmt, ...);
void err_sys(const char *fmt, ...);
void err_dump(const char *fmt, ...);
void err_msg(const char *fmt, ...);
void err_quit(const char *fmt, ...);

void err_accept(int err);
void err_bind(int err);
void err_chdir(int err);
void err_chmod(int err);
void err_close(int err);
void err_connect(int err);
void err_dup2(int err);
void err_execve(int err);
void err_fork(int err);
void err_fstat(int err);
void err_gethostname(int err);
void err_getsockname(int err);
void err_kill(int err);
void err_listen(int err);
void err_lstat(int err);
void err_mmap(int err);
void err_open(int err);
void err_pipe(int err);
void err_read(int err);
void err_recvfrom(int err);
void err_recvmsg(int err);
void err_sendmsg(int err);
void err_sendto(int err);
void err_shmat(int err);
void err_shmctl(int err);
void err_shmdt(int err);
void err_shmget(int err);
void err_sigaction(int err);
void err_sigprocmask(int err);
void err_socket(int err);
void err_stat(int err);
void err_wait(int err);
void err_write(int err);
void err_fopen(int err);
void err_freopen(int err);
void err_getaddrinfo(int err);
void err_inet_ntop(int err);
void err_inet_pton(int err);
void err_malloc(int err);
void err_opendir(int err);
void err_sigsetops(int err);
void err_pthread_attr_setdetachstate(int err);
void err_pthread_create(int err);
void err_pthread_sigmask(int err);

#endif
