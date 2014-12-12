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
#pragma once

#include <stdbool.h>
#include <AL/al.h>

typedef struct Music Music;
typedef struct MusicCallback MusicCallback;

#include "audio.h"

#define STREAM_NUM_BUFFERS 3

struct MusicCallback {
	unsigned int (*feed_buffer)(MusicCallback *mc, unsigned short *buffer, unsigned int len);
	void (*rewind)(MusicCallback *mc);
	void (*free)(MusicCallback *mc);
};

struct Music {
	Source* source;
	ALuint alBuffers[STREAM_NUM_BUFFERS];
	bool ended;
	bool loop;
	MusicCallback* callback;
	ALenum format;
	int samplesrate;
	unsigned int buffersize;
	int ref;
};

void music_play(Music *m, bool loop);
void music_update(Music *m);
void music_stop(Music *m);
void music_pause(Music *m);
void music_free(Music *m);

Music *music_load(MusicCallback* callback, int samplesrate, int num_channels);
Music *music_load_from_file(const char* filename);

