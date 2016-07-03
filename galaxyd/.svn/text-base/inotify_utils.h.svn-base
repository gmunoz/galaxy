/*
 * Galaxy - A filesystem monitoring tool.
 * Copyright (C) 2005  Gabriel Munoz <gabriel@xusia.net>
 * 
 * Based on inotify-utils by Robert Love:
 *   http://www.kernel.org/pub/linux/kernel/people/rml/inotify/utils/
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

#ifndef __INOTIFY_UTILS_H
#define __INOTIFY_UTILS_H

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#if HAVE_INTTYPES_H
#  include <inttypes.h>
#endif

#include "inotify.h"
#include "event_queue.h"

int galaxy_add_watch(const char *dirname, uint32_t mask);
int galaxy_remove_watch(__u32 wd);

void print_event (struct inotify_event *event);
int read_event (int fd, struct inotify_event *event);
int read_events (queue_t q, int fd);
int event_check (int fd);
int read_and_print_events (queue_t q, int fd);
int dev_stats (int fd);
int dev_setdebug (int fd, int debug);
int close_dev (int fd);
int open_dev ();

void err_inotify_init(int err);
void err_inotify_add_watch(int err);
void err_inotify_rm_watch(int err);

#endif
