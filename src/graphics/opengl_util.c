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
#ifndef EMSCRIPTEN
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>
#else
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#endif

#include "opengl_util.h"
#include "log.h"

void check_opengl_oom(void)
{
	GLenum e;

	e = glGetError();
	if (e == GL_OUT_OF_MEMORY)
		log_oom_and_exit();
}

#ifndef NDEBUG
const char* getGLError(GLenum error)
{
#define casereturn(x) case x: return #x
	switch (error) {
			casereturn(GL_INVALID_ENUM);
			casereturn(GL_INVALID_VALUE);
			casereturn(GL_INVALID_OPERATION);
			casereturn(GL_INVALID_FRAMEBUFFER_OPERATION);
			casereturn(GL_OUT_OF_MEMORY);
		default:
		case GL_NO_ERROR:
			return "";
	}
#undef casereturn
}
#endif

