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
#include <emscripten.h>
#include <sys/stat.h>
#include <miniz.h>
#include <stdlib.h>

#include "util.h"
#include "macro.h"
#include "engine.h"
#include "log.h"

log_category("main");

static void on_zip_downloaded(_unused_ void* userdata, void* buffer, int size)
{
	mz_zip_archive za;
	if (!mz_zip_reader_init_mem(&za, buffer, size, 0)) {
		log_error("Cannot unzip game files: invalid archive");
		return;
	}

	for (mz_uint i = 0; i < mz_zip_reader_get_num_files(&za); i++) {
		mz_zip_archive_file_stat file_stat;
		if (!mz_zip_reader_file_stat(&za, i, &file_stat)) {
			log_error("Cannot unzip game files");
			break;
		}
		const char* filename = file_stat.m_filename;

		int r = mkdir_p(filename);
		if (r < 0) {
			log_error("Cannot unzip game files: %s", strerror(-r));
			break;
		}
		if (!mz_zip_reader_is_file_a_directory(&za, i)) {
			mz_zip_reader_extract_to_file(&za, i, filename, 0);
		}
	}
	mz_zip_reader_end(&za);
	engine_load();
}

static void on_zip_fail(void* userdata)
{
	log_error("Unable to download %s", (char *) userdata);
	engine_load(); // load anyway (as long as old method still work)
}

static void loop()
{
	if (engine_is_loaded()) {
		engine_update();
	}
}

int main(int argc, char* argv[])
{
	const char* filename = "main.lua";
	const char* zipname = "game.zip";
	int r;

	int ziplen = strlen("--zip=");
	for (int i = 1; i < argc; i++) {
		if (!strncmp(argv[i], "--zip=", ziplen)) {
			zipname = argv[i] + ziplen;
		} else {
			filename = argv[i];
		}
	}

	r = engine_init(filename, 60);
	if (r < 0) {
		return EXIT_FAILURE;
	}

	emscripten_async_wget_data(zipname, (void*) zipname, on_zip_downloaded, on_zip_fail);
	emscripten_set_main_loop(loop, 0, true);

	return 0;
}

