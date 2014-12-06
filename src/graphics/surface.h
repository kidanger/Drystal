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

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdbool.h>

#ifndef EMSCRIPTEN
#include <SDL2/SDL_opengles2.h>
#else
#include <SDL/SDL_opengl.h>
#endif

typedef struct Surface Surface;

enum FilterMode {
	NEAREST = GL_NEAREST,
	LINEAR = GL_LINEAR,
	BILINEAR = GL_LINEAR_MIPMAP_NEAREST,
	TRILINEAR = GL_LINEAR_MIPMAP_LINEAR,
};
typedef enum FilterMode FilterMode;

struct Surface {
	unsigned int w;
	unsigned int h;
	unsigned int texw;
	unsigned int texh;
	FilterMode filter;
	bool has_fbo;
	bool has_mipmap;
	bool npot;
	int ref;

	GLuint tex;
	GLuint fbo;
};

Surface *surface_new(unsigned int w, unsigned int h, unsigned int texw, unsigned int texh, unsigned char *pixels, Surface *current_from, Surface *current_on);
void surface_free(Surface *s);
void surface_draw_on(Surface *s);
void surface_draw_from(Surface *s);
void surface_set_filter(Surface *s, FilterMode filter, Surface *current_surface);

static inline void surface_get_size(const Surface *s, unsigned int *w, unsigned int *h)
{
	assert(w);
	assert(h);
	*w = s->w;
	*h = s->h;
}

int surface_load(const char* filename, Surface **surface, Surface *current_surface);

#ifdef __cplusplus
}
#endif
