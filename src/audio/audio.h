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

typedef struct Source Source;
typedef enum SourceType {
	SOURCE_MUSIC,
	SOURCE_SOUND
} SourceType;

#include "sound.h"
#include "music.h"
#include "log.h"

#define DEFAULT_SAMPLES_RATE 44100

struct Source {
	ALuint alSource;
	bool used;
	bool paused;
	union {
		Sound* currentSound;
		Music* currentMusic;
	};
	SourceType type;
	float desiredVolume;
};

#ifdef DODEBUG
#define audio_check_error() do { \
		ALint error; \
		while ((error = alGetError()) != AL_NO_ERROR) { \
			log_debug("[ALerr] %s", alGetString(error)); \
		} \
	} while (false)
#else
#define audio_check_error()
#endif

bool initialise_if_needed();
void update_audio(float dt);
void destroy_audio();

void set_music_volume(float volume);
void set_sound_volume(float volume);
float get_music_volume();
float get_sound_volume();
bool try_free_sound(Sound* sound);

Source* get_free_source();

