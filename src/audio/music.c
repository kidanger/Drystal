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
#include <assert.h>
#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>

#include "log.h"
#include "audio.h"
#include "music.h"
#include "util.h"
#include "dlua.h"
#include "lua_util.h"

log_category("music");

static Music *music_new(MusicCallback* clb, ALenum format, int rate)
{
	Music *m;

	assert(clb);

	m = new0(Music, 1);

	m->callback = clb;
	m->format = format;
	m->samplesrate = rate;
	m->buffersize = rate * 0.4;
	m->pitch = 1.0;
	m->volume = 1.0;
	alGenBuffers(STREAM_NUM_BUFFERS, m->alBuffers);
	audio_check_error();

	return m;
}

void music_play(Music *m, bool loop, int onend_clb)
{
	ALushort *buff;
	unsigned int len;
	unsigned int i;
	Source *source;

	assert(m);
	if (m->onend_clb)
		luaL_unref(dlua_get_lua_state(), LUA_REGISTRYINDEX, m->onend_clb);
	m->onend_clb = onend_clb;

	if (m->source) {
		if (m->source->paused) {
			m->source->paused = false;
			alSourcePlay(m->source->alSource);
		}
		return;
	}

	source = audio_get_free_source();
	if (!source)
		return;

	buff = newa(ALushort, m->buffersize);
	for (i = 0; i < STREAM_NUM_BUFFERS; i++) {
		len = m->callback->feed_buffer(m->callback, buff, m->buffersize);
		alBufferData(m->alBuffers[i], m->format, buff, len * sizeof(ALushort), m->samplesrate);
		audio_check_error();
	}

	alSourceQueueBuffers(source->alSource, STREAM_NUM_BUFFERS, m->alBuffers);
	audio_check_error();
	alSourcef(source->alSource, AL_GAIN, m->volume * audio_get_music_volume());
	audio_check_error();
	alSourcef(source->alSource, AL_PITCH, m->pitch);
	audio_check_error();
	alSourcePlay(source->alSource);
	audio_check_error();

	m->ended = false;
	m->loop = loop;
	source->type = SOURCE_MUSIC;
	source->currentMusic = m;
	source->used = true;
	source->desiredVolume = 1;
	m->source = source;
}

void music_pause(Music *m)
{
	assert(m);

	if (!m->source)
		return;

	alSourcePause(m->source->alSource);
	audio_check_error();
	m->source->paused= true;
}

void music_stop(Music *m)
{
	assert(m);

	if (m->source == NULL)
		return;

	alSourceStop(m->source->alSource);
	alSourcei(m->source->alSource, AL_BUFFER, 0);
	m->callback->rewind(m->callback);
	m->source->used = false;
	m->source = NULL;
}

void music_set_volume(Music *m, float volume)
{
	assert(m);

	m->volume = volume;

	if (!m->source)
		return;

	alSourcef(m->source->alSource, AL_GAIN, volume * audio_get_music_volume());
	audio_check_error();
}

void music_set_pitch(Music *m, float pitch)
{
	assert(m);

	m->pitch = pitch;

	if (!m->source)
		return;

	alSourcef(m->source->alSource, AL_PITCH, pitch);
	audio_check_error();
}

void music_free(Music *m)
{
	if (!m)
		return;

	if (m->source) {
		music_stop(m);
	}

	alDeleteBuffers(STREAM_NUM_BUFFERS, m->alBuffers);
	m->callback->free(m->callback);
	free(m);
}

static void music_stream(Music *m)
{
	Source *source;
	ALint processed;
	ALushort *buff;

	assert(m);

	source = m->source;
	buff = newa(ALushort, m->buffersize);

	alGetSourcei(source->alSource, AL_BUFFERS_PROCESSED, &processed);
	audio_check_error();

	while (processed--) {
		ALuint buffer;
		unsigned int len;

		alSourceUnqueueBuffers(source->alSource, 1, &buffer);
		audio_check_error();

		len = m->callback->feed_buffer(m->callback, buff, m->buffersize);
		if (len == 0 && !m->loop)
			break;

		alBufferData(buffer, m->format, buff, len * sizeof(ALushort), m->samplesrate);
		audio_check_error();

		if (len < m->buffersize) {
			if (m->loop) {
				unsigned int rest;

				m->callback->rewind(m->callback);
				// fill the rest of the buffer
				rest = m->callback->feed_buffer(m->callback, buff + len, m->buffersize - len);
				alBufferData(buffer, m->format, buff, (len + rest) * sizeof(ALushort), m->samplesrate);
				audio_check_error();
			}
		}

		alSourceQueueBuffers(source->alSource, 1, &buffer);
		audio_check_error();
	}

	int nb_queued;
	alGetSourcei(source->alSource, AL_BUFFERS_QUEUED, &nb_queued);
	m->ended = nb_queued == 0;
}

Music* music_load(MusicCallback* callback, int samplesrate, int num_channels)
{
	assert(callback);
	assert(num_channels == 1 || num_channels == 2);

	if (!audio_init_if_needed())
		return NULL;

	return music_new(callback, num_channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16 , samplesrate);
}

typedef struct VorbisMusicCallback VorbisMusicCallback;
struct VorbisMusicCallback {
	MusicCallback base;

	stb_vorbis *stream;
	stb_vorbis_info info;
};

static void vmc_free(MusicCallback *mc)
{
	VorbisMusicCallback *vmc = (VorbisMusicCallback *) mc;

	if (!vmc)
		return;

	stb_vorbis_close(vmc->stream);
	free(vmc);
}

static unsigned int vmc_feed_buffer(MusicCallback *mc, unsigned short *buffer, unsigned int len)
{
	VorbisMusicCallback *vmc = (VorbisMusicCallback *) mc;
	int size;

	assert(vmc);

	size = stb_vorbis_get_samples_short_interleaved(
	           vmc->stream, vmc->info.channels,
	           (short*) buffer, len);
	size *= vmc->info.channels;
	return size;
}

static void vmc_rewind(MusicCallback *mc)
{
	VorbisMusicCallback *vmc = (VorbisMusicCallback *) mc;

	assert(vmc);

	stb_vorbis_seek_start(vmc->stream);
}

static VorbisMusicCallback *vmc_new(stb_vorbis *stream)
{
	VorbisMusicCallback *vmc;

	assert(stream);

	vmc = new(VorbisMusicCallback, 1);

	vmc->stream = stream;
	vmc->info = stb_vorbis_get_info(stream);
	vmc->base.free = vmc_free;
	vmc->base.rewind = vmc_rewind;
	vmc->base.feed_buffer = vmc_feed_buffer;

	return vmc;
}

Music* music_load_from_file(const char* filename)
{
	assert(filename);

	if (!audio_init_if_needed())
		return NULL;

	stb_vorbis* stream = stb_vorbis_open_filename(filename, NULL, NULL);
	if (!stream) {
		return NULL;
	}

	VorbisMusicCallback *callback = vmc_new(stream);

	return music_load((MusicCallback*) callback, callback->info.sample_rate, callback->info.channels);
}

void music_update(Music *m)
{
	assert(m);

	if (m->source) {
		music_stream(m);

		if (m->ended) {
			music_stop(m);
			if (m->onend_clb) {
				lua_State* L = dlua_get_lua_state();
				lua_rawgeti(L, LUA_REGISTRYINDEX, m->onend_clb);
				call_lua_function(L, 0, 0);
			}
		}
	}
}

