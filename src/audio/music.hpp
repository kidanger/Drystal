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

#include "audio.hpp"

#define STREAM_NUM_BUFFERS 3

class MusicCallback
{
public:
	virtual ~MusicCallback() {}
	virtual unsigned int feed_buffer(unsigned short * buffer, unsigned int len) = 0;
};

struct Source;

class Music
{
private:
	Source* source;
	ALuint alBuffers[STREAM_NUM_BUFFERS];
	MusicCallback* callback;
	ALenum format;
	int samplesrate;
	unsigned int buffersize;
	bool ended;

	Music(MusicCallback* clb, ALenum format, int rate);

public:
	void play();
	void stream();
	void stop();
	void free();

	bool is_ended() const
	{
		return ended;
	}

	static Music* load(MusicCallback* callback, int samplesrate = DEFAULT_SAMPLES_RATE, int num_channels = 1);
	static Music* load_from_file(const char* filename);
};

