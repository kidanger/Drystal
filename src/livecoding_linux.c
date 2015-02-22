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
static char **dirnames = NULL;
static pthread_t watcher_tid;
static bool running = false;
static void *callback_arg = NULL;
static void (*reload_callback)(void *arg, const char* filename) = NULL;

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
	char *event_filename;
	char *dirname = NULL;
	char *fullname;

	assert(ievent);

	if (ievent->len) {
		event_filename = ievent->name;
	} else {
		return;
	}

	for (size_t i = 0; i < wds_count; i++) {
		if (wds[i] == ievent->wd) {
			dirname = dirnames[i];
			break;
		}
	}
	assert(dirname);
	fullname = strjoin(dirname, "/", event_filename, NULL);

	switch (ievent->mask & (IN_CLOSE_WRITE | IN_CREATE)) {
		case IN_CLOSE_WRITE:
			// We don't treat directories modifications
			if (ievent->mask & IN_ISDIR) {
				break;
			}
			if (!is_valid_filename(event_filename)) {
				break;
			}

			reload_callback(callback_arg, fullname);
			break;
		case IN_CREATE:
			// recursively watch new dirs
			if (!(ievent->mask & IN_ISDIR)) {
				break;
			}

			livecoding_watch_directory(fullname);
			break;
		default:
			break;
	}

	free(fullname);
}

static int handle_events(void)
{
	union {
		struct inotify_event ev;
		uint8_t raw[EVENT_BUFFER_LEN];
	} buffer;
	struct inotify_event *ievent;
	ssize_t r;
	int count = 0;

	r = read(fd, buffer.raw, EVENT_BUFFER_LEN);
	if (r <= 0) {
		return r;
	}

	if (r < (ssize_t) sizeof(struct inotify_event)) {
		return -EIO;
	}

	for (ievent = &buffer.ev; (uint8_t *) ievent < (uint8_t *) buffer.raw + r; ievent = (struct inotify_event *) (ievent + sizeof(struct inotify_event) + ievent->len)) {
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
		closedir(dir);
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

int livecoding_init(void (*callback)(void *arg, const char* filename), void *arg)
{
	int ret;
	int r;
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

	size_t dirnames_nb = wds_nmemb;
	XREALLOC(wds, wds_nmemb, 1);
	XREALLOC(dirnames, dirnames_nb, 1);

	reload_callback = callback;
	callback_arg = arg;

	r = pthread_create(&watcher_tid, NULL, watcher_loop, NULL);
	if (r < 0) {
		ret = -errno;
		goto fail;
	}

	running = true;

	return 0;

fail:
	close(fd);
	close(wakeup_read_fd);
	close(wakeup_write_fd);
	free(wds);
	free(dirnames);
	fd = -1;
	wakeup_read_fd = -1;
	wakeup_write_fd = -1;
	wds = NULL;
	dirnames = NULL;

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

	wd = inotify_add_watch(fd, directory, IN_CLOSE_WRITE | IN_CREATE);
	if (wd < 0) {
		return -errno;
	}

	size_t dirnames_nb = wds_nmemb;
	XREALLOC(wds, wds_nmemb, wds_count + 1);
	XREALLOC(dirnames, dirnames_nb, wds_count + 1);
	wds[wds_count] = wd;
	dirnames[wds_count] = xstrdup(directory);
	wds_count++;

	return wd;
}

int livecoding_quit(void)
{
	size_t i;

	if (!running)
		return 0;

	assert(wds);
	assert(fd >= 0);

	// Wakeup poll() to tell the thread to die
	write(wakeup_write_fd, "\0", 1);

	pthread_join(watcher_tid, NULL);

	for (i = 0; i < wds_count; i++) {
		assert(wds[i] >= 0);
		close(wds[i]);
		free(dirnames[i]);
	}

	free(wds);
	free(dirnames);
	close(fd);
	close(wakeup_read_fd);
	close(wakeup_write_fd);

	wds = NULL;
	dirnames = NULL;
	fd = -1;
	wakeup_read_fd = -1;
	wakeup_write_fd = -1;
	running = false;
	wds_count = 0;
	wds_nmemb = 0;
	return 0;
}

int livecoding_is_running(void)
{
	return running;
}

