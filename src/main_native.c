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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Drystal.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <string.h>
#include <errno.h>
#include <stdio.h>
#ifdef BUILD_LIVECODING
#include <assert.h>
#include <libgen.h>// dirname()
#include <stdlib.h>

#include "livecoding.h"
#include "dlua.h"
#include "macro.h"
#endif

#include "engine.h"
#include "log.h"
#include "util.h"

log_category("main");

#ifdef BUILD_LIVECODING
static void reload(_unused_ void *arg, const char* filename)
{
	if (endswith(filename, ".png")) {
		engine_add_surface_to_reloadqueue(filename);
	} else if (endswith(filename, ".wav")) {
		engine_add_sound_to_reloadqueue(filename);
	} else {
		dlua_set_need_to_reload();
	}
}

static int start_livecoding(const char *filename)
{
	char *filename_dup;
	char *watched_directory;
	int r;

	assert(filename);

	filename_dup = strdup(filename);
	if (!filename_dup) {
		return -ENOMEM;
	}

	watched_directory = dirname(filename_dup);
	r = livecoding_init(reload, NULL);
	if (r < 0) {
		log_error("Cannot initialize livecoding: %s", strerror(-r));
		free(filename_dup);
		return r;
	}

	r = livecoding_watch_directory_recursively(watched_directory);
	if (r < 0) {
		log_error("Cannot watch %s for livecoding: %s", watched_directory, strerror(-r));
		free(filename_dup);
		return r;
	}

	r = livecoding_start();
	if (r < 0) {
		log_error("Cannot start livecoding: %s", strerror(-r));
		free(filename_dup);
		return r;
	}
	free(filename_dup);

	return 0;
}
#endif

static void help(void)
{
	printf("drystal [OPTIONS] <game.lua>\n\n"
	       "OPTIONS\n"
	       "    -h --help       Show this help message and exit\n"
#ifdef BUILD_LIVECODING
	       "    -l --livecoding Enable the livecoding which will reload the lua code when modifications on the files are performed\n"
#endif
	      );
}

int main(int argc, char* argv[])
{
	const char* filename = "main.lua";
#ifdef BUILD_LIVECODING
	bool livecoding = false;
#endif

	for (int i = 1; i < argc; i++) {
		if (streq(argv[i], "--help") || streq(argv[i], "-h")) {
			help();
			return 0;
#ifdef BUILD_LIVECODING
		} else if (streq(argv[i], "--livecoding") || streq(argv[i], "-l")) {
			livecoding = true;
#endif
		} else {
			filename = argv[i];
		}
	}

	engine_init(filename, 60);
#ifdef BUILD_LIVECODING
	if (livecoding) {
		int r = start_livecoding(filename);
		if (r < 0) {
			return r;
		}
	}
#endif

	engine_load();
	engine_loop();

#ifdef BUILD_LIVECODING
	if (livecoding) {
		livecoding_stop();
	}
#endif

	engine_free();

	return 0;
}

