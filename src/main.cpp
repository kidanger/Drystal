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
#include <cstring>
#include <cerrno>
#include <cstdio>
#ifndef EMSCRIPTEN
#ifdef BUILD_LIVECODING
#include <libgen.h>// dirname()
#include <cstdlib>
#include "livecoding.h"
#endif
#else
#include <emscripten.h>
#include <sys/stat.h>
#include <miniz.h>

#include "util.h"
#endif

#include "engine.hpp"
#include "log.hpp"

log_category("main");

#ifdef EMSCRIPTEN
static Engine* engine = NULL;

static void on_zip_downloaded(void* userdata, void* buffer, int size)
{
	mz_zip_archive za;
	if (!mz_zip_reader_init_mem(&za, buffer, size, 0)) {
		log_error("Cannot unzip game files: invalid archive");
		return;
	}

	for (int i = 0; i < mz_zip_reader_get_num_files(&za); i++) {
		mz_zip_archive_file_stat file_stat;
		if (!mz_zip_reader_file_stat(&za, i, &file_stat)) {
			log_error("Cannot unzip game files");
			break;
		}
		const char* filename = file_stat.m_filename;

		if (!mz_zip_reader_is_file_a_directory(&za, i)) {
			r = mkdir_p(filename);
			if (r < 0) {
				log_error("Cannot unzip game files: %s", strerror(-r));
				break;
			}
			mz_zip_reader_extract_to_file(&za, i, filename, 0);
		}
	}
	mz_zip_reader_end(&za);
	engine->load();
}

static void on_zip_fail(void* userdata)
{
	log_error("Unable to download %s", (char *) userdata);
	engine->load(); // load anyway (as long as old method still work)
}

static void loop()
{
	if (engine->is_loaded()) {
		engine->update();
	}
}

int main(int argc, const char* argv[])
{
	const char* filename = "main.lua";
	const char* zipname = "game.zip";

	int ziplen = strlen("--zip=");
	for (int i = 1; i < argc; i++) {
		if (!strncmp(argv[i], "--zip=", ziplen)) {
			zipname = argv[i] + ziplen;
		} else {
			filename = argv[i];
		}
	}

	Engine e(filename, 60);
	engine = &e;

	emscripten_async_wget_data(zipname, (void*) zipname, on_zip_downloaded, on_zip_fail);
	emscripten_set_main_loop(loop, 0, true);

	return 0;
}
#else
#ifdef BUILD_LIVECODING
static Engine* engine = NULL;

static void reload(void)
{
	assert(engine);

	engine->lua.set_need_to_reload();
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
	r = livecoding_init(reload);
	if (r < 0) {
		log_error("Cannot initialize livecoding: %s", strerror(-r));
		free(filename_dup);
		return -r;
	}

	r = livecoding_watch_directory_recursively(watched_directory);
	if (r < 0) {
		log_error("Cannot watch %s for livecoding: %s", watched_directory, strerror(-r));
		free(filename_dup);
		return -r;
	}

	r = livecoding_start();
	if (r < 0) {
		log_error("Cannot start livecoding: %s", strerror(-r));
		free(filename_dup);
		return -r;
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
int main(int argc, const char* argv[])
{
	const char* filename = "main.lua";
#ifdef BUILD_LIVECODING
	bool livecoding = false;
#endif

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
			help();
			return 0;
#ifdef BUILD_LIVECODING
		} else if (!strcmp(argv[i], "--livecoding") || !strcmp(argv[i], "-l")) {
			livecoding = true;
#endif
		} else {
			filename = argv[i];
		}
	}

	Engine e(filename, 60);
#ifdef BUILD_LIVECODING
	engine = &e;
	if (livecoding) {
		int r = start_livecoding(filename);
		if (r < 0) {
			return r;
		}
	}
#endif

	e.load();
	e.loop();

#ifdef BUILD_LIVECODING
	if (livecoding) {
		livecoding_stop();
	}
#endif

	return 0;
}
#endif

