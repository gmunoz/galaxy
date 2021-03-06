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

#include <signal.h>

#if HAVE_PTHREAD_H
#  include <pthread.h>
#endif

#if HAVE_ERRNO_H
#  include <errno.h>
#endif

#include "signal_thread.h"
#include "thread.h"
#include "error.h"

extern pthread_t crawler, watcher, signaler, server;

static sigset_t mask;

static void *
signal_handler(void *arg)
{
	int err, signo;

	while (1) {
		err = sigwait(&mask, &signo);

		switch (signo) {
			case SIGINT:
#ifdef DEBUG_SIGNAL_HANDLER
				err_msg("DEBUG[signal_handler]: SIGINT caught.\n");
#endif
				pthread_cancel(crawler);
				pthread_cancel(watcher);
				pthread_cancel(server);
				return NULL;
				break;
			case SIGQUIT:
#ifdef DEBUG_SIGNAL_HANDLER
				err_msg("DEBUG[signal_handler]: SIGQUIT caught.\n");
#endif
				break;
			default:
				err_msg("warning[signal_handler]: unexpected signal %d.\n", signo);
				break;
		}
	}
}

int
create_signal_thread(pthread_t *id)
{
	int err;
	sigset_t oldmask;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGQUIT);

	err = pthread_sigmask(SIG_BLOCK, &mask, &oldmask);
	if (err != 0) {
		err_pthread_sigmask(errno);
		return err;
	}

	err = create_joinable_thread(id, signal_handler, 0);
	if (err < 0) {
		err_create_joinable_thread(errno);
		return err;
	}

	return err;
}
