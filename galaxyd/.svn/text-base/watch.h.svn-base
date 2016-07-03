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

#ifndef WATCH_H
#define WATCH_H

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#if HAVE_INTTYPES_H
#  include <inttypes.h>
#endif

/* Initialization and destruction routines -- called once on
 * startup/shutdown. */
int init_client_watches_container(void);
void destroy_client_watches_container(void);

/* Functions to manipulate the various watch lists. */
int set_galaxy_ignore_mask(const char *client_name, uint32_t ignore_mask);
int add_galaxy_watch(const char *client_name, uint32_t mask,
	const char *pattern);
int add_galaxy_ignore_watch(const char *client_name, uint32_t mask,
	const char *pattern);

/* Functions to manipulate all entries for a client. */
void find_matching_events(const char *filename, uint32_t mask);
int remove_galaxy_watches(const char *client_name);

#endif
