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

#ifndef __EVENT_QUEUE_H
#define __EVENT_QUEUE_H

struct queue_struct;
typedef struct queue_struct *queue_t;

int queue_empty (queue_t q);
int queue_full (queue_t q);
queue_t queue_create (int num_elements);
void queue_destroy (queue_t q);
void queue_make_empty (queue_t q);
void queue_enqueue (void *d, queue_t q);
void *queue_front (queue_t q);
void queue_dequeue (queue_t q);

#endif
