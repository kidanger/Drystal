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

#include <cassert>

#ifndef EMSCRIPTEN
#include <SDL2/SDL_opengles2.h>
#else
#include <SDL/SDL_opengl.h>
#endif

enum FilterMode {
	NEAREST = GL_NEAREST,
	LINEAR = GL_LINEAR,
	BILINEAR = GL_LINEAR_MIPMAP_NEAREST,
	TRILINEAR = GL_LINEAR_MIPMAP_LINEAR,
};

class Surface
{
public:
	Surface(unsigned int w, unsigned int h, unsigned int texw, unsigned int texh, unsigned char *pixels, Surface *current_from, Surface *current_on);
	~Surface();
	void draw_on();
	void draw_from();
	void set_filter(FilterMode filter, Surface *current_surface);
	inline void get_size(unsigned int *w, unsigned int *h) const
	{
		assert(w);
		assert(h);
		*w = this->w;
		*h = this->h;
	}

	static int load(const char* filename, Surface **surface, Surface *current_surface);

	unsigned int w;
	unsigned int h;
	unsigned int texw;
	unsigned int texh;
	FilterMode filter;
	bool has_fbo;
	bool has_mipmap;
	bool npot;
	int ref;

private:
	GLuint tex;
	GLuint fbo;

	void create_fbo();
};

