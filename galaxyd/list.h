/*
 * Galaxy - A filesystem monitoring tool.
 * Copyright (C) 2005  Gabriel Munoz <gabriel@xusia.net>
 *
 * Based on algorithms and code from Introduction to Algorithms, Second
 * Edition by Thomas H. Cormen, et al, 2001; and Mastering Algorithms
 * with C by Kyle Loudon, 1999.
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

#ifndef LIST_H
#define LIST_H

typedef struct list_node_t {
	struct list_node_t *next;
	struct list_node_t *prev;
	void *key;
} list_node_t;

typedef struct list_t {
	unsigned long size;
	list_node_t *nil;
	void (*destroy)(void *key);
} list_t;

/* Allocates, initializes, and returns a list container */
list_t *list_create(void (*destroy)(void *key));
/* Destroys all elements in the list */
void list_destroy(list_t *list);
/* Inserts a node into head of the the given list */
int list_insert(list_t *list, void *key);
/* Inserts a node into the tail of the given list */
int list_push(list_t *list, void *key);
/* Removes the node from a list and returns the deleted node */
list_node_t *list_delete(list_t *list, list_node_t *node);
/* Removes the node from a list and returns the key of the node */
void *list_remove(list_t *list, list_node_t *node);

/* Inserts a node (created by key) into the head of the list */
#define list_unshift(list, key)  list_insert(list, key)
/* Removes the head of the list and returns a pointer to the key */
#define list_shift(list)   list_remove(list, list_head(list))
/* Removes the tail of the list and returns a pointer to the key */
#define list_pop(list)     list_remove(list, list_tail(list))
/* Iterate over a list that is NOT safe for node removal */
#define list_foreach(list, n)  \
	for (n = (list)->nil->next; n != (list)->nil; n = n->next)
/* Iterate over a list that is safe against node removal */
#define list_foreach_safe(list, n, lahead)  \
	for (n = (list)->nil->next, lahead = n->next;  \
	     n != (list)->nil; n = lahead, lahead = n->next)

#define list_size(list)    ((list)->size)
#define list_head(list)    ((list)->nil->next)
#define list_tail(list)    ((list)->nil->prev)
#define list_nil(list)     ((list)->nil)
#define list_peek(list)    ((list)->nil->next->key)
#define list_next(node)    ((node)->next)
#define list_prev(node)    ((node)->prev)
#define list_key(node)     ((node)->key)
#define list_is_nil(node)  ((node)->key == NULL)

#endif
