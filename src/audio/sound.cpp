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
#include <cassert>

#include <wavloader.c>

#include "log.hpp"
#include "audio.hpp"
#include "sound.hpp"

log_category("sound");

Sound::Sound(ALushort* buffer, unsigned int length, int samplesrate) :
	alBuffer(0),
	free_me(false),
	ref(0)
{
	alGenBuffers(1, &alBuffer);
	alBufferData(alBuffer, AL_FORMAT_MONO16,
	             buffer, length * sizeof(ALushort), samplesrate);

	audio_check_error();
}

Sound* Sound::load_from_file(const char *filepath)
{
	assert(filepath);
	if (!initialise_if_needed())
		return NULL;

	void* buffer;
	int length;
	int format;
	int channels;
	int samplesrate;
	int err = load_wav(filepath, &buffer, &length, &format, &channels, &samplesrate);
	if (err) {
		return NULL;
	}

	Sound* sound = new Sound(static_cast<ALushort*>(buffer), length, samplesrate);
	return sound;
}

Sound* Sound::load(unsigned int len, const float* buffer, int samplesrate)
{
	assert(buffer);
	if (!initialise_if_needed())
		return NULL;
	ALushort converted_buffer[len]; // 16bits per sample
	for (unsigned int i = 0; i < len; i++) {
		converted_buffer[i] = static_cast<ALushort>(buffer[i] * 65535 / 2 + 65535 / 2);
	}

	Sound* sound = new Sound(converted_buffer, len, samplesrate);
	return sound;
}

void Sound::free()
{
	// if there's no more source playing the sound, free it
	if (try_free_sound(this)) {
		alDeleteBuffers(1, &alBuffer);
		audio_check_error();
		delete this;
	} else {
		// otherwise, just delay the deletion
		free_me = true;
	}
}

void Sound::play(float volume, float x, float y)
{
	Source* source = get_free_source();
	if (!source)
		return;

	audio_check_error();
	alSourcei(source->alSource, AL_BUFFER, alBuffer);
	audio_check_error();
	alSource3f(source->alSource, AL_POSITION, x, y, 0.);
	audio_check_error();
	alSourcef(source->alSource, AL_GAIN, volume * get_sound_volume());
	audio_check_error();
	alSourcePlay(source->alSource);
	audio_check_error();

	source->isMusic = false;
	source->currentSound = this;
	source->used = true;
	source->desiredVolume = volume;
}

