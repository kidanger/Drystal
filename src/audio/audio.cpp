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
#include <cstddef>
#include <cstdio>
#include <AL/al.h>
#include <AL/alc.h>

#include "macro.hpp"
#include "music.hpp"
#include "sound.hpp"
#include "audio.hpp"

#define NUM_SOURCES 16

static bool initialized = false;
static ALCcontext* context;
static ALCdevice* device;
static float globalSoundVolume = 1.;
static float globalMusicVolume = 1.;

Source sources[NUM_SOURCES];

static bool init_audio()
{
	device = alcOpenDevice(NULL);
	if (!device) {
		fprintf(stderr, "cannot open device\n");
		return false;
	}

	context = alcCreateContext(device, NULL);
	if (!context) {
		fprintf(stderr, "cannot create context\n");
		return false;
	}

	if (!alcMakeContextCurrent(context)) {
		fprintf(stderr, "cannot make context\n");
		return false;
	}

	for (int i = 0; i < NUM_SOURCES; i++)
		alGenSources(1, &sources[i].alSource);

	initialized = true;
	return true;
}

bool initialise_if_needed()
{
	if (!initialized)
		init_audio();
	if (!initialized)
		return false;
	return true;
}

void update_audio(_unused_ float dt)
{
	if (!initialized)
		return;

	ALint status;
	for (int i = 0; i < NUM_SOURCES; i++) {
		Source& source = sources[i];
		if (!source.used)
			continue;
		alGetSourcei(source.alSource, AL_SOURCE_STATE, &status);
		source.used = status == AL_PLAYING;

		if (!source.used && !source.isMusic) { // ended sound
			Sound* sound = source.currentSound;
			if (sound->wants_to_be_free()) {
				sound->free();
			}
		}
		if (source.used && source.isMusic) { // still playing music
			Music* music = source.currentMusic;
			if (!music->is_ended()) {
				music->stream();
			}
		}
	}
}

void destroy_audio()
{
	if (initialized) {
		for (int i = 0; i < NUM_SOURCES; i++)
			alDeleteSources(1, &sources[i].alSource);

		alcMakeContextCurrent(NULL);
		alcDestroyContext(context);
		alcCloseDevice(device);
	}
}

Source* get_free_source()
{
	for (int i = 0; i < NUM_SOURCES; i++) {
		if (!sources[i].used) {
			return &sources[i];
		}
	}
	fprintf(stderr, "no more source available\n");
	return NULL;
}

const char* getAlError(ALint error)
{
#define casereturn(x) case x: return #x
	switch (error) {
			casereturn(AL_INVALID_NAME);
			casereturn(AL_INVALID_ENUM);
			casereturn(AL_INVALID_VALUE);
			casereturn(AL_INVALID_OPERATION);
			casereturn(AL_OUT_OF_MEMORY);
		default:
			casereturn(AL_NO_ERROR);
	}
#undef casereturn
	return "";
}

void set_music_volume(float volume)
{
	globalMusicVolume = volume;
	if (!initialized)
		return;

	// update current playing musics
	for (int i = 0; i < NUM_SOURCES; i++) {
		Source& source = sources[i];
		if (source.isMusic) {
			alSourcef(source.alSource, AL_GAIN, source.desiredVolume * volume);
			audio_check_error();
		}
	}
}

void set_sound_volume(float volume)
{
	globalSoundVolume = volume;
	if (!initialized)
		return;

	// update current playing sounds
	for (int i = 0; i < NUM_SOURCES; i++) {
		Source& source = sources[i];
		if (!source.isMusic) {
			alSourcef(source.alSource, AL_GAIN, source.desiredVolume * volume);
			audio_check_error();
		}
	}
}

float get_music_volume()
{
	return globalMusicVolume;
}

float get_sound_volume()
{
	return globalSoundVolume;
}

bool try_free_sound(Sound* sound)
{
	bool can_free = true;
	// stop sources which used the sound
	for (int i = 0; i < NUM_SOURCES; i++) {
		Source& source = sources[i];
		if (source.currentSound == sound) {
			if (source.used) {
				can_free = false;
			} else {
				alSourceStop(source.alSource);
				alSourcei(source.alSource, AL_BUFFER, 0);
			}
		}
	}
	audio_check_error();
	return can_free;
}
