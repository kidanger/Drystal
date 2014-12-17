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
#include <stddef.h>
#include <AL/al.h>
#include <AL/alc.h>

#include "macro.h"
#include "music.h"
#include "sound.h"
#include "audio.h"

log_category("audio");

#define NUM_SOURCES 16

static bool initialized = false;
static ALCcontext* context;
static ALCdevice* device;
static float globalSoundVolume = 1.;
static float globalMusicVolume = 1.;

Source sources[NUM_SOURCES];

static void audio_init(void)
{
	device = alcOpenDevice(NULL);
	if (!device) {
		log_error("Cannot open device");
		return;
	}

	context = alcCreateContext(device, NULL);
	if (!context) {
		log_error("Cannot create context");
		return;
	}

	if (!alcMakeContextCurrent(context)) {
		log_error("Cannot make context");
		return;
	}

	for (unsigned i = 0; i < NUM_SOURCES; i++)
		alGenSources(1, &sources[i].alSource);

	initialized = true;
}

bool audio_init_if_needed()
{
	if (!initialized)
		audio_init();

	return initialized;
}

void audio_update(_unused_ float dt)
{
	if (!initialized)
		return;

	ALint status;
	for (unsigned i = 0; i < NUM_SOURCES; i++) {
		Source *source = &sources[i];
		if (!source->used)
			continue;
		alGetSourcei(source->alSource, AL_SOURCE_STATE, &status);
		source->used = status == AL_PLAYING || status == AL_PAUSED;

		if (!source->used && source->type == SOURCE_SOUND) { // ended sound
			Sound *sound = source->currentSound;
			if (sound->free_me) {
				sound_free(sound);
			}
		}
		if (status == AL_PLAYING && source->type == SOURCE_MUSIC) { // still playing music
			Music *music = source->currentMusic;
			music_update(music);
		}
		if (!source->used) {
			// if the source is not playing anymore,
			// remove any buffer attached to it
			alSourcei(source->alSource, AL_BUFFER, 0);
			audio_check_error();
		}
	}
}

void audio_free(void)
{
	if (initialized) {
		for (unsigned i = 0; i < NUM_SOURCES; i++)
			alDeleteSources(1, &sources[i].alSource);

		alcMakeContextCurrent(NULL);
		alcDestroyContext(context);
		alcCloseDevice(device);
	}
}

Source* audio_get_free_source(void)
{
	for (unsigned i = 0; i < NUM_SOURCES; i++) {
		if (!sources[i].used) {
			return &sources[i];
		}
	}
	log_error("No more source available");
	return NULL;
}

void audio_set_music_volume(float volume)
{
	globalMusicVolume = volume;
	if (!initialized)
		return;

	// update current playing musics
	for (unsigned i = 0; i < NUM_SOURCES; i++) {
		Source *source = &sources[i];
		if (source->type == SOURCE_MUSIC) {
			alSourcef(source->alSource, AL_GAIN, source->desiredVolume * volume);
			audio_check_error();
		}
	}
}

void audio_set_sound_volume(float volume)
{
	globalSoundVolume = volume;
	if (!initialized)
		return;

	// update current playing sounds
	for (unsigned i = 0; i < NUM_SOURCES; i++) {
		Source *source = &sources[i];
		if (source->type == SOURCE_SOUND) {
			alSourcef(source->alSource, AL_GAIN, source->desiredVolume * volume);
			audio_check_error();
		}
	}
}

float audio_get_music_volume()
{
	return globalMusicVolume;
}

float audio_get_sound_volume()
{
	return globalSoundVolume;
}

bool audio_try_free_sound(Sound* sound)
{
	bool can_free = true;
	// stop sources which used the sound
	for (unsigned i = 0; i < NUM_SOURCES; i++) {
		Source *source = &sources[i];
		if (source->currentSound == sound) {
			if (source->used) {
				can_free = false;
			} else {
				alSourceStop(source->alSource);
				alSourcei(source->alSource, AL_BUFFER, 0);
			}
		}
	}
	audio_check_error();
	return can_free;
}

