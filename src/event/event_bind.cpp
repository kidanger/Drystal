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
#ifndef EMSCRIPTEN
#include <SDL2/SDL.h>
#else
#include <SDL/SDL.h>
#endif

#include "engine.hpp"
#include "event_bind.hpp"

int mlua_set_relative_mode(lua_State* L)
{
	assert(L);

	bool relative = lua_toboolean(L, 1);
#ifdef EMSCRIPTEN
	Engine& engine = get_engine();
	engine.display.show_cursor(not relative);
	// NOT IMPLEMENTED
#else
	SDL_SetRelativeMouseMode(relative ? SDL_TRUE : SDL_FALSE);
#endif
	return 0;
}

int mlua_start_text(lua_State*)
{
	SDL_StartTextInput();
	return 0;
}

int mlua_stop_text(lua_State*)
{
	SDL_StopTextInput();
	return 0;
}

