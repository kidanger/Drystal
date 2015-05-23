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

#ifndef EMSCRIPTEN
#include <SDL2/SDL.h>
#else
#include <SDL/SDL.h>
#endif

enum Button {
	BUTTON_LEFT        = SDL_BUTTON_LEFT,
	BUTTON_MIDDLE      = SDL_BUTTON_MIDDLE,
	BUTTON_RIGHT       = SDL_BUTTON_RIGHT,
	BUTTON_EXTRA_LEFT  = SDL_BUTTON_X1,
	BUTTON_EXTRA_RIGHT = SDL_BUTTON_X2,
	// There is no more WHEEL_UP constant in SDL2 so we create ours
	WHEEL_UP = 4,
	WHEEL_DOWN = 5,
};
typedef enum Button Button;

void event_update(void);
void event_small_update(void);
void event_destroy(void);
int event_init(void);

void event_set_relative_mode(bool relative);

