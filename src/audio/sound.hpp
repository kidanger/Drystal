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

class Sound {
private:
	ALuint alBuffer;
	bool free_me;

public:
	void play(float volume = 1, float x = 0, float y = 0);
	void free();

	bool wants_to_be_free() const { return free_me; }

	static Sound* load_from_file(const char *filepath);
	static Sound* load(unsigned int len, const float* buffer, int samplesrate = DEFAULT_SAMPLES_RATE);
};
