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

#include <assert.h>
#include <stdbool.h>

#ifndef EMSCRIPTEN
#include <SDL2/SDL_opengles2.h>
#else
#include <SDL/SDL_opengl.h>
#endif

typedef struct Surface Surface;

enum FilterMode {
	FILTER_NEAREST = GL_NEAREST,
	FILTER_LINEAR = GL_LINEAR,
	FILTER_BILINEAR = GL_LINEAR_MIPMAP_NEAREST,
	FILTER_TRILINEAR = GL_LINEAR_MIPMAP_LINEAR,

	FILTER_DEFAULT = FILTER_LINEAR,
};
typedef enum FilterMode FilterMode;

enum SurfaceFormat {
	FORMAT_LUMINANCE = GL_LUMINANCE,
	FORMAT_LUMINANCE_ALPHA = GL_LUMINANCE_ALPHA,
	FORMAT_RGB = GL_RGB,
	FORMAT_RGBA = GL_RGBA,
};
typedef enum SurfaceFormat SurfaceFormat;

struct Surface {
	char* filename;
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

	unsigned char *pixels;
	bool pixels_valid;
};

Surface *surface_new(unsigned int w,
                     unsigned int h,
                     unsigned int texw,
                     unsigned int texh,
                     SurfaceFormat format,
                     void *pixels,
                     Surface *current_from,
                     Surface *current_on);
void surface_free(Surface *s);
void surface_draw_on(Surface *s);
void surface_draw_from(Surface *s);
void surface_set_filter(Surface *s, FilterMode filter, Surface *current_surface);
void surface_get_pixel(Surface *s, unsigned int x, unsigned int y,
		       int *red, int *green, int *blue, int *alpha, Surface *current_on);

static inline void surface_get_size(const Surface *s, unsigned int *w, unsigned int *h)
{
	assert(w);
	assert(h);
	*w = s->w;
	*h = s->h;
}

int surface_load(const char* filename, Surface **surface, Surface *current_surface);

