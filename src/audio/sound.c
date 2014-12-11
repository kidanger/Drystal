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
#include <assert.h>
#include <stdlib.h>

#define WAVLOADER_HEADER_ONLY
#include <wavloader.c>

#include "log.h"
#include "audio.h"
#include "sound.h"
#include "util.h"

log_category("sound");

static Sound *sound_new(ALushort* buffer, unsigned int length, int samplesrate, unsigned bits_per_sample, unsigned num_channels)
{
	Sound *s;
	ALenum format = AL_FORMAT_MONO8;

	s = new(Sound, 1);
	if (!s)
		return NULL;

	s->alBuffer = 0;
	s->free_me = false;
	s->ref = 0;

	alGenBuffers(1, &s->alBuffer);

	if (bits_per_sample == 8) {
		if (num_channels == 1) {
			format = AL_FORMAT_MONO8;
		} else if (num_channels == 2) {
			format = AL_FORMAT_STEREO8;
		}
	} else if (bits_per_sample == 16) {
		if (num_channels == 1) {
			format = AL_FORMAT_MONO16;
		} else if (num_channels == 2) {
			format = AL_FORMAT_STEREO16;
		}
	}
	alBufferData(s->alBuffer, format, buffer, length, samplesrate);

	audio_check_error();

	return s;
}

Sound *sound_load_from_file(const char *filepath)
{
	void *buffer = NULL;
	struct wave_header wave_header;
	int r;

	assert(filepath);

	if (!initialise_if_needed())
		return NULL;

	r = load_wav(filepath, &wave_header, &buffer);
	if (r < 0) {
		return NULL;
	}
	if (wave_header.bits_per_sample != 8 && wave_header.bits_per_sample != 16) {
		free(buffer);
		return NULL;
	}
	if (wave_header.num_channels != 1 && wave_header.num_channels != 2) {
		free(buffer);
		return NULL;
	}
	if (wave_header.sample_rate != 44100) {
		free(buffer);
		return NULL;
	}

	Sound* sound = sound_new((ALushort *) buffer, wave_header.data_size,
	                         wave_header.sample_rate, wave_header.bits_per_sample,
	                         wave_header.num_channels);
	free(buffer);
	return sound;
}

Sound* sound_load(unsigned int len, const float* buffer, int samplesrate)
{
	ALushort converted_buffer[len]; // 16bits per sample

	assert(buffer);

	if (!initialise_if_needed())
		return NULL;

	for (unsigned int i = 0; i < len; i++) {
		converted_buffer[i] = (ALushort)(buffer[i] * 65535 / 2 + 65535 / 2);
	}

	Sound* sound = sound_new(converted_buffer, len * sizeof(ALushort), samplesrate, 16, 1);
	return sound;
}

void sound_free(Sound *s)
{
	if (!s)
		return;

	// if there's no more source playing the sound, free it
	if (try_free_sound(s)) {
		alDeleteBuffers(1, &s->alBuffer);
		audio_check_error();
		free(s);
	} else {
		// otherwise, just delay the deletion
		s->free_me = true;
	}
}

void sound_play(Sound *sound, float volume, float x, float y)
{
	assert(sound);

	Source* source = get_free_source();
	if (!source)
		return;

	audio_check_error();
	alSourcei(source->alSource, AL_BUFFER, sound->alBuffer);
	audio_check_error();
	alSource3f(source->alSource, AL_POSITION, x, y, 0.);
	audio_check_error();
	alSourcef(source->alSource, AL_GAIN, volume * get_sound_volume());
	audio_check_error();
	alSourcePlay(source->alSource);
	audio_check_error();

	source->isMusic = false;
	source->currentSound = sound;
	source->used = true;
	source->desiredVolume = volume;
}

