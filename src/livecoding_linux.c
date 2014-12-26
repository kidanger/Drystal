/**
 * This file is part of Drystal.
 *
 * Drystal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Drystal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Drystal.If not, see <http://www.gnu.org/licenses/>.
 */
#include <sys/inotify.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <tgmath.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <poll.h>
#include <libgen.h>
#include <signal.h>

#include "livecoding.h"
#include "macro.h"
#include "util.h"

#define EVENT_BUFFER_LEN (20 * (sizeof(struct inotify_event) + FILENAME_MAX))

static int fd = -1;
static int wakeup_write_fd = -1;
static int wakeup_read_fd = -1;
static int *wds = NULL;
static size_t wds_nmemb = 0;
static size_t wds_count = 0;
static pthread_t watcher_tid;
static void *callback_arg = NULL;
static void (*reload_callback)(void *arg) = NULL;

static bool is_valid_filename(char *filename)
{
	assert(filename);

	// Ignore backup files
	if (endswith(filename, ".swp") || endswith(filename, "~")) {
		return false;
	}

	// Ignore hidden files
	if (startswith(basename(filename), ".")) {
		return false;
	}

	return true;
}

static void handle_event(struct inotify_event *ievent)
{
	char *event_filename = NULL;

	assert(ievent);

	if (ievent->len) {
		event_filename = ievent->name;
	} else {
		return;
	}

	switch (ievent->mask & (IN_MODIFY | IN_CREATE)) {
		case IN_MODIFY:
			// We don't treat directories modifications
			if (ievent->mask & IN_ISDIR) {
				return;
			}
			if (!is_valid_filename(event_filename))
				return;

			reload_callback(callback_arg);
			break;
		case IN_CREATE:
			// recursively watch new dirs
			if (!(ievent->mask & IN_ISDIR)) {
				return;
			}
			livecoding_watch_directory(event_filename);
			break;
		default:
			break;
	}
}

static int handle_events(void)
{
	union {
		struct inotify_event ev;
		uint8_t raw[EVENT_BUFFER_LEN];
	} buffer;
	struct inotify_event *ievent;
	ssize_t r;
	ssize_t i;
	size_t event_size;
	int count = 0;

	r = read(fd, buffer.raw, EVENT_BUFFER_LEN);
	if (r <= 0) {
		return r;
	}

	if (r < (ssize_t) sizeof(struct inotify_event)) {
		return -EIO;
	}

	i = 0;
	while (i < r) {
		ievent = (struct inotify_event *)(&buffer.ev + i);
		event_size = offsetof(struct inotify_event, name) + ievent->len;
		i += event_size;
		handle_event(ievent);
		count++;
	}

	return count;
}

static void *watcher_loop(_unused_ void *args)
{
	int r = 1;
	bool stop_thread = false;
	struct pollfd fds[2];

	fds[0].fd = fd;
	fds[0].events = POLLIN;
	fds[1].fd = wakeup_read_fd;
	fds[1].events = POLLIN;
	fds[1].revents = 0;

	while (r > 0 && !stop_thread) {
		r = poll(fds, 2, -1);
		stop_thread = fds[1].revents & POLLIN;
		if (r > 0 && !stop_thread) {
			r = handle_events();
		}
	}

	return NULL;
}

int livecoding_watch_directory_recursively(const char *path)
{
	DIR *dir;
	struct dirent *entry;
	int r;

	assert(path);

	dir = opendir(path);
	if (!dir) {
		return -errno;
	}

	r = livecoding_watch_directory(path);
	if (r < 0) {
		return r;
	}

	while ((entry = readdir(dir))) {
		char *new_path;
		const char *d_name = entry->d_name;

		if (!(entry->d_type & DT_DIR)) {
			continue;
		}

		if (startswith(d_name, ".")) {
			continue;
		}

		new_path = strjoin(path, "/", d_name, NULL);
		if (!new_path) {
			continue;
		}

		livecoding_watch_directory_recursively(new_path);

		free(new_path);
	}
	closedir(dir);

	return 0;
}

int livecoding_init(void (*callback)(void *arg), void *arg)
{
	int ret;
	int fildes[2];

	assert(fd < 0);
	assert(callback);

	fd = inotify_init();
	if (fd < 0) {
		ret = -errno;
		goto fail;
	}

	if (pipe(fildes) < 0) {
		ret = -errno;
		goto fail;
	}

	wakeup_read_fd = fildes[0];
	wakeup_write_fd = fildes[1];

	if (!XREALLOC(wds, wds_nmemb, 1)) {
		ret = -ENOMEM;
		goto fail;
	}

	reload_callback = callback;
	callback_arg = arg;

	return 0;

fail:
	close(fd);
	close(wakeup_read_fd);
	close(wakeup_write_fd);

	return ret;
}

int livecoding_watch_directory(const char *directory)
{
	int wd;

	assert(directory);
	assert(wds);

	if (!is_directory(directory)) {
		return -EINVAL;
	}

	wd = inotify_add_watch(fd, directory, IN_MODIFY | IN_CREATE);
	if (wd < 0) {
		return -errno;
	}

	if (!XREALLOC(wds, wds_nmemb, wds_count + 1)) {
		close(wd);
		return -ENOMEM;
	}
	wds[wds_count] = wd;
	wds_count++;

	return wd;
}

int livecoding_stop(void)
{
	size_t i;

	assert(wds);
	assert(fd >= 0);

	// Wakeup poll() to tell the thread to die
	write(wakeup_write_fd, "\0", 1);

	pthread_join(watcher_tid, NULL);

	for (i = 0; i < wds_count; i++) {
		assert(wds[i] >= 0);
		close(wds[i]);
	}

	free(wds);
	close(fd);
	close(wakeup_read_fd);
	close(wakeup_write_fd);

	return 0;
}

int livecoding_start(void)
{
	int r;

	r = pthread_create(&watcher_tid, NULL, watcher_loop, NULL);
	if (r < 0) {
		return -errno;
	}

	return 0;
}

int livecoding_is_running(void)
{
	return pthread_kill(watcher_tid, 0) != ESRCH;
}

