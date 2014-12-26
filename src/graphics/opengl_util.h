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

#ifndef EMSCRIPTEN
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>
#else
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#endif

#include <stdlib.h>

#include "log.h"

void check_opengl_oom(void);

#ifdef DODEBUG
const char* getGLError(GLenum error);

#define GLDEBUG(x) \
	x; \
	{ \
		GLenum e; \
		while((e = glGetError()) != GL_NO_ERROR) \
		{ \
			log_debug("%s for call %s", getGLError(e), #x); \
			exit(1); \
		} \
	}
#else
#define GLDEBUG(x) \
	x;
#endif

