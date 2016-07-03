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

#if HAVE_PTHREAD_H
#  include <pthread.h>
#endif

#if HAVE_ERRNO_H
#  include <errno.h>
#endif

#include "thread.h"
#include "error.h"

/*
 * Return Value:
 *   Returns 0 on success; -1 when pthread_create caused an error
 *   (errno is still set).
 */
int
create_joinable_thread(pthread_t *id, const void *func, void *data)
{
	int err;

	err = pthread_create(id, NULL, func, data);
	if (err < 0)
		return -1;

	return err;
}

/*
 * Return Value:
 *   Returns 0 on success; -2 when pthread_attr_setdetachstate caused an
 *   error (errno is still set); -1 when pthread_create caused an error
 *   (errno is still set).
 */
int
create_detached_thread(pthread_t *id, const void *func, void *data)
{
	int err;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (err != 0)
		return -2;

	err = pthread_create(id, &attr, func, data);
	if (err < 0)
		return -1;

	return err;
}
