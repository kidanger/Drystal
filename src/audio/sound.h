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

typedef struct Sound Sound;

struct Sound {
	ALuint alBuffer;
	bool free_me;
	int ref;
};

void sound_play(Sound *sound, float volume, float x, float y, float pitch);
void sound_free(Sound *sound);

Sound *sound_load_from_file(const char *filepath);
Sound *sound_load(unsigned int len, const float* buffer, int samplesrate);

