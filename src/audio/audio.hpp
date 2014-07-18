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

#include <AL/al.h>
#include <AL/alc.h>

struct Sound;
struct Music;

struct Source {
	ALuint alSource;
	bool used;
	bool isMusic;
	union {
		Sound* currentSound;
		Music* currentMusic;
	};
	float desiredVolume;
};

struct Sound {
	ALuint alBuffer;
	bool free_me;
};

class MusicCallback
{
public:
	virtual ~MusicCallback() {}
	virtual unsigned int feed_buffer(unsigned short * buffer, unsigned int len) = 0;
};

#define STREAM_NUM_BUFFERS 3
struct Music {
	Source* source;
	ALuint alBuffers[STREAM_NUM_BUFFERS];
	MusicCallback* callback;
	ALenum format;
	int samplesrate;
	unsigned int buffersize;
	bool ended;
};

#define DEFAULT_SAMPLES_RATE 44100

class Audio
{
public:
	Audio();
	~Audio();

	void update(float dt);

	Sound* load_sound(const char *filepath);
	Sound* create_sound(unsigned int len, const float* buffer, int samplesrate = DEFAULT_SAMPLES_RATE);
	void play_sound(Sound* sound, float volume = 1, float x = 0, float y = 0);
	void free_sound(Sound* sound);

	Music* load_music(MusicCallback* callback, int samplesrate = DEFAULT_SAMPLES_RATE, int num_channels = 1);
	Music* load_music_from_file(const char* filename);
	void play_music(Music* music);
	void stop_music(Music* music);
	void free_music(Music* music);

	void set_music_volume(float volume);
	void set_sound_volume(float volume);

private:
	bool initialized;
	ALCcontext* context;
	ALCdevice* device;
	float globalSoundVolume;
	float globalMusicVolume;

	bool init();
	void stream_music(Music* music);
	Audio(const Audio&);
	Audio& operator=(const Audio&);
};
