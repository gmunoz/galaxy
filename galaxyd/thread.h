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

#ifndef THREAD_H
#define THREAD_H

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#if HAVE_PTHREAD_H
#  include <pthread.h>
#endif

int create_joinable_thread(pthread_t *id, const void *func, void *data);
int create_detached_thread(pthread_t *id, const void *func, void *data);

/* Synonymous with create_joinable_thread(). */
#define create_thread(id,func,data) create_joinable_thread(id,func,data)

/* Error handlers. */
#define err_create_thread(__err) err_create_joinable_thread(__err)
#define err_create_joinable_thread(__err) \
	{ \
	if (__err == -1) { \
		err_pthread_create(__err); \
		err_msg("error: Unable to create detached thread.\n"); \
	} \
	}
#define err_create_detached_thread(__err) \
	{ \
	if (__err == -1) { \
		err_pthread_create(__err); \
		err_msg("error: Unable to create detached thread.\n"); \
	} else if (__err == -2) { \
		err_pthread_attr_setdetachstate(__err); \
		err_msg("error: Unable to set thread detach state.\n"); \
	} \
	}

#endif
