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

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#if HAVE_STDLIB_H
#  include <stdlib.h>
#endif

#include "event_queue.h"

struct queue_struct 
{
	int capacity;
	int front;
	int rear;
	int size;
	void **array;
};

int
queue_empty(queue_t q)
{
	return q->size == 0;
}

int
queue_full(queue_t q)
{
	return q->size == q->capacity;
}

queue_t
queue_create(int num_elements)
{
	queue_t q;

	q = malloc (sizeof (struct queue_struct));

	if (q == NULL)
		exit (-1);

	q->array = malloc(sizeof (void *) * num_elements);

	if (q->array == NULL)
		exit (-1);

	q->capacity = num_elements;

	queue_make_empty (q);

	return q;
}

void
queue_destroy(queue_t q)
{
	if (q != NULL)
	{
		if (q->array)
			free (q->array);

		free (q);
	}
}

void
queue_make_empty(queue_t q)
{
	q->size = 0;
	q->front = 1;
	q->rear = 0;
}

static int
next_position(int v, queue_t q)
{
	if (++v == q->capacity) {
		v = 0;
	}

	return v;
}

void
queue_enqueue(void *d, queue_t q)
{
	if (queue_full (q))
	{
		return;
	}

	q->size++;
	q->rear = next_position (q->rear, q);
	q->array[q->rear] = d;
}

void *
queue_front(queue_t q)
{
	if (!queue_empty(q))
		return q->array [q->front];

	return NULL;
}

void
queue_dequeue(queue_t q)
{
	if (!queue_empty (q))
	{
		q->size--;
		q->front = next_position (q->front, q);
	}
}
