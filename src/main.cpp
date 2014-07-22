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
#ifndef EMSCRIPTEN
#include <signal.h>
#else
#include <string>
#include <cstdio>
#include <emscripten.h>
#include <sys/stat.h>
#include <miniz.h>
#endif

#include "engine.hpp"

Engine* engine;

#ifdef EMSCRIPTEN
static void mkpath(const char* path)
{
	char filepath[strlen(path) + 1];
	strcpy(filepath, path);
	for (char* p = strchr(filepath + 1, '/'); p; p = strchr(p + 1, '/')) {
		*p = '\0';
		mkdir(filepath, 0777);
		*p = '/';
	}
}
static void on_zip_downloaded(void* userdata, void* buffer, int size)
{
	mz_zip_archive za;
	if (!mz_zip_reader_init_mem(&za, buffer, size, 0)) {
		printf("mz_zip_reader_init_file() failed!\n");
		return;
	}

	for (int i = 0; i < mz_zip_reader_get_num_files(&za); i++) {
		mz_zip_archive_file_stat file_stat;
		mz_zip_reader_file_stat(&za, i, &file_stat);
		const char* filename = file_stat.m_filename;

		if (!mz_zip_reader_is_file_a_directory(&za, i)) {
			mkpath(filename);
			mz_zip_reader_extract_to_file(&za, i, filename, 0);
		}
	}
	mz_zip_reader_end(&za);
	engine->load();
}

static void on_zip_fail(void* userdata)
{
	printf("Unable to download %s.\n", userdata);
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

	Engine e(filename, 60, false);
	engine = &e;

	emscripten_async_wget_data(zipname, (void*) zipname, on_zip_downloaded, on_zip_fail);
	emscripten_set_main_loop(loop, 0, true);

	return 0;
}
#else
static void reload(int)
{
	engine->lua.reload_code();
}

int main(int argc, const char* argv[])
{
	const char* filename = "main.lua";
	bool server_mode = false;

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--server") || !strcmp(argv[i], "-s")) {
			server_mode = true;
		} else {
			filename = argv[i];
		}
	}

	Engine e(filename, 60, server_mode);
	engine = &e;
	signal(SIGUSR1, reload);

	e.load();
	e.loop();

	return 0;
}
#endif

