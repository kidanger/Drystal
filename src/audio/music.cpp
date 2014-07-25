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
#include <cassert>
#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>

#include "log.hpp"
#include "audio.hpp"
#include "music.hpp"

log_category("music");

Music::Music(MusicCallback* clb, ALenum format, int rate) :
	source(NULL),
	callback(clb),
	format(format),
	samplesrate(rate),
	buffersize(rate * 0.4),
	ended(false),
	ref(0)
{
	alGenBuffers(STREAM_NUM_BUFFERS, alBuffers);
	audio_check_error();
}

void Music::play()
{
	if (source != NULL)
		return;

	this->source = get_free_source();
	if (!source)
		return;

	ALushort buff[buffersize];
	unsigned int len;
	for (int i = 0; i < STREAM_NUM_BUFFERS; i++) {
		len = callback->feed_buffer(buff, buffersize);
		alBufferData(alBuffers[i], format, buff, len * sizeof(ALushort), samplesrate);
		audio_check_error();
	}

	alSourceQueueBuffers(source->alSource, STREAM_NUM_BUFFERS, alBuffers);
	alSourcef(source->alSource, AL_GAIN, get_music_volume());
	alSourcePlay(source->alSource);
	audio_check_error();

	ended = false;
	source->isMusic = true;
	source->currentMusic = this;
	source->used = true;
	source->desiredVolume = 1;
}


void Music::stop()
{
	alSourceStop(source->alSource);
	alSourcei(source->alSource, AL_BUFFER, 0);
	source->used = false;
}

void Music::free()
{
	if (source) {
		stop();
	}
	alDeleteBuffers(2, alBuffers);
	delete callback;
	delete this;
}
void Music::stream()
{
	Source* source = this->source;

	ALint processed;
	alGetSourcei(source->alSource, AL_BUFFERS_PROCESSED, &processed);

	while (processed--) {
		ALuint buffer;
		alSourceUnqueueBuffers(source->alSource, 1, &buffer);
		audio_check_error();

		ALushort buff[buffersize];
		unsigned int len = callback->feed_buffer(buff, buffersize);
		alBufferData(buffer, format, buff, len * sizeof(ALushort), samplesrate);
		audio_check_error();

		if (len < buffersize) {
			ended = true;
		}

		alSourceQueueBuffers(source->alSource, 1, &buffer);
		audio_check_error();
	}
}

Music* Music::load(MusicCallback* callback, int samplesrate, int num_channels)
{
	assert(callback);
	if (!initialise_if_needed())
		return NULL;
	ALenum format = num_channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
	Music* music = new Music(callback, format, samplesrate);
	return music;
}

class VorbisMusicCallback : public MusicCallback
{
public:
	stb_vorbis* stream;
	stb_vorbis_info info;

	VorbisMusicCallback(stb_vorbis* stream) :
		stream(stream),
		info(stb_vorbis_get_info(stream))
	{
	}

	~VorbisMusicCallback()
	{
		stb_vorbis_close(stream);
	}

	unsigned int feed_buffer(unsigned short * buffer, unsigned int len)
	{
		int size = stb_vorbis_get_samples_short_interleaved(
		               this->stream, this->info.channels,
		               reinterpret_cast<short*>(buffer), len);
		size *= this->info.channels;
		return size;
	}
private:
	VorbisMusicCallback(const VorbisMusicCallback&);
	VorbisMusicCallback& operator=(const VorbisMusicCallback&);
};

Music* Music::load_from_file(const char* filename)
{
	assert(filename);
	if (!initialise_if_needed())
		return NULL;

	stb_vorbis* stream = stb_vorbis_open_filename((char*) filename, NULL, NULL);
	if (!stream) {
		return NULL;
	}

	VorbisMusicCallback* callback = new VorbisMusicCallback(stream);

	return load(callback, callback->info.sample_rate, callback->info.channels);
}
