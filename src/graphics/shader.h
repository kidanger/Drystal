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

typedef struct Shader Shader;

const char* SHADER_PREFIX;
const char* DEFAULT_VERTEX_SHADER;
const char* DEFAULT_FRAGMENT_SHADER_COLOR;
const char* DEFAULT_FRAGMENT_SHADER_TEX;

const char* DEFAULT_FRAGMENT_SHADER_TEXPOINT;

typedef enum AttrLocationIndex {
	// WebGL wants 0 as an attribute, so here it is
	ATTR_LOCATION_POSITION = 0,
	ATTR_LOCATION_COLOR,
	ATTR_LOCATION_TEXCOORD,
	ATTR_LOCATION_POINTSIZE,
} AttrLocationIndex;

enum VarLocationIndex {
	VAR_LOCATION_COLOR,
	VAR_LOCATION_TEX,
};
typedef enum VarLocationIndex VarLocationIndex;

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

};
Shader *shader_new(GLuint prog_color, GLuint prog_tex, GLuint vert, GLuint frag_color, GLuint frag_tex);
void shader_free(Shader *s);

void shader_feed(Shader *s, const char* name, float value);

