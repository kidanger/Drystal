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
#include <cstring>
#include <cmath>
#include "stb_image.h"

#include "surface.hpp"
#include "log.hpp"

log_category("graphics");

Surface::Surface(unsigned int w, unsigned int h, unsigned int texw, unsigned int texh, unsigned char *pixels, Surface *current_from, Surface *current_on) :
	w(w),
	h(h),
	texw(texw),
	texh(texh),
	filter(LINEAR),
	has_fbo(false),
	has_mipmap(false),
	npot(false),
	ref(0),
	tex(0),
	fbo(0)
{
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texw, texh, 0,
	             GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, current_from ? current_from->tex : 0);

	if (!pixels) {
		// we'll need a FBO anyway
		create_fbo();
		// We do not need to bind the fbo since it is done in create_fbo

		// clear the surface
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, current_on ? current_on->fbo : 0);
	}
}

Surface::~Surface()
{
	glDeleteTextures(1, &tex);
	if (has_fbo) {
		glDeleteFramebuffers(1, &fbo);
	}
}

void Surface::create_fbo()
{
	assert(!has_fbo);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
	                       GL_TEXTURE_2D, tex, 0);
	assert(fbo != 0);

#ifndef NDEBUG
	GLenum status;
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	assert(status == GL_FRAMEBUFFER_COMPLETE);
#endif

	has_fbo = true;
}

void Surface::draw_on()
{
	has_mipmap = false;
	if (!has_fbo) {
		create_fbo();
		// We do not need to bind the fbo since it is done in create_fbo
	} else {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	}
}

void Surface::draw_from()
{
	glBindTexture(GL_TEXTURE_2D, tex);

	if (!has_mipmap && filter >= BILINEAR && !npot) {
		glGenerateMipmap(GL_TEXTURE_2D);
		has_mipmap = true;
	}
}

void Surface::set_filter(FilterMode new_filter, Surface *current_surface)
{
	if (filter == new_filter) {
		return;
	}

	filter = new_filter;

	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter > LINEAR ? LINEAR : filter);

	if (!has_mipmap && filter >= BILINEAR && !npot) {
		glGenerateMipmap(GL_TEXTURE_2D);
		has_mipmap = true;
	}

	if (this != current_surface) {
		glBindTexture(GL_TEXTURE_2D, current_surface ? current_surface->tex : 0);
	}
}

#define RGBA_SIZE 4
Surface* Surface::load(const char *filename, Surface *current_surface)
{
	assert(filename);

	int w, h;
	int n;
	unsigned char *data = stbi_load(filename, &w, &h, &n, RGBA_SIZE);

	if (!data)
		return NULL;

	int potw = pow(2, ceil(log(w) / log(2)));
	int poth = pow(2, ceil(log(h) / log(2)));

	Surface* surface = NULL;

	if (potw != w || poth != h) {
		unsigned char *pixels = new unsigned char[potw * poth * RGBA_SIZE];
		if (!pixels) {
			return NULL;
		}
		memset(pixels, 0, potw * poth * RGBA_SIZE);
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				int is = (x + y * w) * RGBA_SIZE;
				int id = (x + y * potw) * RGBA_SIZE;
				memcpy(pixels + id, data + is, RGBA_SIZE);
			}
		}
		surface = new Surface(w, h, potw, poth, pixels, current_surface, NULL);
		delete[] pixels;
	} else {
		surface = new Surface(w, h, w, h, data, current_surface, NULL);
	}

	stbi_image_free(data);

	return surface;
}

