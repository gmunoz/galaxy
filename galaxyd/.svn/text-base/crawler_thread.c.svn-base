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

#if HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif

#if HAVE_SYS_STAT_H
#  include <sys/stat.h>
#endif

#if HAVE_UNISTD_H
#  include <unistd.h>
#endif

#if HAVE_DIRENT_H
#  include <dirent.h>
#endif

#if HAVE_STRING_H
#  include <string.h>
#endif

#if HAVE_PTHREAD_H
#  include <pthread.h>
#endif

#if HAVE_ERRNO_H
#  include <errno.h>
#endif

#if HAVE_LIBGLIB_2_0
#  include <glib.h>
#endif

#include "crawler_thread.h"
#include "thread.h"
#include "inotify.h"
#include "inotify_utils.h"
#include "error.h"
#include "list.h"

extern pthread_mutex_t inotify_wds_mutex;

static int total = 0;

struct crawl_data_t {
	int fd;
	int recursive;  /* Boolean to specify if we should add recursively. */
	const list_t *dirs;
	const list_t *prune_dirs;
	GHashTable *inotify_wds;
} crawl_data_t;

static int
max_pathname(const char *path)
{
  long max_path;

  errno = 0;
  max_path = pathconf("/", _PC_PATH_MAX);
  if (max_path == -1) {
    if (errno == 0)
      max_path = 4096;  /* guess */
    else
			err_msg("error[max_pathname]: pathconf failed.\n");
	}
	return max_path + 1;
}

static int
recursive_crawl(int fd, const char *dirname, GHashTable *wds,
	const list_t *prune_dirs, const int recursive)
{
	DIR *dir = NULL;
	struct dirent *entry;
	struct stat statbuf;
	int ret, *key;
	list_node_t *node = NULL;

	/* Prune this directory if it is in our list of prunes. */
	list_foreach(prune_dirs, node) {
		if (strcmp(dirname, list_key(node)) == 0)
			return 0;
	}

	dir = opendir(dirname);
	if (dir == NULL) {
		err_opendir(errno);
		return -1;
	}

	while (errno = 0, (entry = readdir(dir)) != NULL && recursive) {
		char path[4097];
		size_t path_len;

		strcpy(path, dirname);
		path_len = strlen(path);

		/* If the directory path doesn't end with a slash, append a slash. */
		if (path[path_len - 1] != '/') {
			path[path_len] = '/';
			path[path_len + 1] = '\0';
			++path_len;
		}

		strncpy(path + path_len, entry->d_name, sizeof(path) - path_len);
		if (lstat(path, &statbuf) == -1) {
			err_lstat(errno);
			err_msg("error[recursive_crawl]: lstat failed on %s\n", entry->d_name);
		}

		if (S_ISDIR(statbuf.st_mode) && strcmp(entry->d_name, ".") != 0 &&
		    strcmp(entry->d_name, "..") != 0) {
			recursive_crawl(fd, path, wds, prune_dirs, recursive);
		}
	}

	if (dir != NULL)
		closedir(dir);

	/* Add the directory to the galaxy watch list. Adding the galaxy watch
	 * here will perform a depth-first search. Keeping this after the
	 * opendir(), closedir(), and recursive calls will minimize the number
	 * of Inotify events raised due to this function crawling the
	 * filesystem.*/
	ret = galaxy_add_watch(dirname, IN_ALL_EVENTS);
	if (ret < 0) {
		err_msg("error[recursive_crawl]: Unable to add a galaxy watch event.\n");
		return -1;
	}
	total++;

	return 0;
}

static void *
crawl(void *arg)
{
	struct crawl_data_t *cdata;

	cdata = (struct crawl_data_t *)arg;
	list_node_t *node = NULL;
	list_foreach(cdata->dirs, node) {
		recursive_crawl(cdata->fd, list_key(node), cdata->inotify_wds,
			cdata->prune_dirs, cdata->recursive);
	}
	free(arg);

	return NULL;
}

int
create_crawler_thread(pthread_t *id, int fd, const list_t *dirs,
	const list_t *prune_dirs, GHashTable *inotify_wds, int recursive)
{
	struct crawl_data_t *cdata;

	cdata = malloc(sizeof(struct crawl_data_t));
	if (cdata == NULL) {
		err_malloc(errno);
		err_msg("error[create_crawler_thread]: Unable to malloc crawl data.\n");
	}

	cdata->fd = fd;
	cdata->recursive = recursive;     /* Specifies to recursively add. */
	cdata->dirs = dirs;               /* Destroyed in galaxyd.c:main(). */
	cdata->prune_dirs = prune_dirs;   /* Destroyed in galaxyd.c:main(). */
	cdata->inotify_wds = inotify_wds; /* Used throughout life of app. */

	return create_joinable_thread(id, crawl, cdata);
}
