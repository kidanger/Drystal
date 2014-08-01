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

#define GL_GLEXT_PROTOTYPES
#ifndef EMSCRIPTEN
#include <SDL2/SDL_opengles2.h>
#else
#include <SDL/SDL_opengl.h>
#endif

extern const char* SHADER_PREFIX;
extern const size_t SHADER_PREFIX_LEN;
extern const char* DEFAULT_VERTEX_SHADER;
extern const char* DEFAULT_FRAGMENT_SHADER_COLOR;
extern const char* DEFAULT_FRAGMENT_SHADER_TEX;

const GLuint ATTR_POSITION_INDEX = 0; // WebGL wants 0 as an attribute, so here it is
const GLuint ATTR_COLOR_INDEX = 1;
const GLuint ATTR_TEXCOORD_INDEX = 2;
const GLuint ATTR_POINTSIZE_INDEX = 3;

enum VarLocationIndex {
	COLOR,
	TEX,
};

struct Shader {
	GLuint prog_color;
	GLuint prog_tex;
	GLuint vert;
	GLuint frag_color;
	GLuint frag_tex;

	struct {
		GLuint dxLocation;
		GLuint dyLocation;
		GLuint zoomLocation;
		GLuint rotationMatrixLocation;
		GLuint destinationSizeLocation;
	} vars[2];
	int ref;

	Shader(GLuint prog_color, GLuint prog_tex, GLuint vert, GLuint frag_color, GLuint frag_tex);
	~Shader();

	void feed(const char* name, float value);
};

