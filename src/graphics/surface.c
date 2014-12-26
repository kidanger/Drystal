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
#include <assert.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#define STBI_ONLY_PNG
#include "stb_image.h"

#include "surface.h"
#include "log.h"
#include "util.h"

log_category("graphics");

static void surface_create_fbo(Surface *s)
{
	assert(s);
	assert(!s->has_fbo);

	glGenFramebuffers(1, &(s->fbo));
	glBindFramebuffer(GL_FRAMEBUFFER, s->fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
	                       GL_TEXTURE_2D, s->tex, 0);
	assert(s->fbo);

#ifndef NDEBUG
	GLenum status;
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	assert(status == GL_FRAMEBUFFER_COMPLETE);
#endif

	s->has_fbo = true;
}

Surface *surface_new(unsigned int w, unsigned int h, unsigned int texw, unsigned int texh, void *pixels, Surface *current_from, Surface *current_on)
{
	Surface *s = new(Surface, 1);
	s->filename = NULL;
	s->w = w;
	s->h = h;
	s->texw = texw;
	s->texh = texh;
	s->filter = FILTER_LINEAR;
	s->has_fbo = false;
	s->has_mipmap = false;
	s->npot = false;
	s->ref = 0;
	s->tex = 0;
	s->fbo = 0;

	glGenTextures(1, &(s->tex));
	glBindTexture(GL_TEXTURE_2D, s->tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, s->texw, s->texh, 0,
	             GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, FILTER_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, FILTER_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, current_from ? current_from->tex : 0);

	if (!pixels) {
		// we'll need a FBO anyway
		surface_create_fbo(s);
		// We do not need to bind the fbo since it is done in create_fbo

		// clear the surface
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, current_on ? current_on->fbo : 0);
	}

	return s;
}

void surface_free(Surface *s)
{
	if (!s)
		return;

	if (s->filename) {
		free(s->filename);
	}
	glDeleteTextures(1, &(s->tex));
	if (s->has_fbo) {
		glDeleteFramebuffers(1, &(s->fbo));
	}
	free(s);
}

void surface_draw_on(Surface *s)
{
	assert(s);

	s->has_mipmap = false;
	if (!s->has_fbo) {
		surface_create_fbo(s);
		// We do not need to bind the fbo since it is done in create_fbo
	} else {
		glBindFramebuffer(GL_FRAMEBUFFER, s->fbo);
	}
}

void surface_draw_from(Surface *s)
{
	assert(s);

	glBindTexture(GL_TEXTURE_2D, s->tex);

	if (!s->has_mipmap && s->filter >= FILTER_BILINEAR && !s->npot) {
		glGenerateMipmap(GL_TEXTURE_2D);
		s->has_mipmap = true;
	}
}

void surface_set_filter(Surface *s, FilterMode new_filter, Surface *current_surface)
{
	assert(s);

	if (s->filter == new_filter) {
		return;
	}

	s->filter = new_filter;

	glBindTexture(GL_TEXTURE_2D, s->tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, s->filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, s->filter > FILTER_LINEAR ? FILTER_LINEAR : s->filter);

	if (!s->has_mipmap && s->filter >= FILTER_BILINEAR && !s->npot) {
		glGenerateMipmap(GL_TEXTURE_2D);
		s->has_mipmap = true;
	}

	if (s != current_surface) {
		glBindTexture(GL_TEXTURE_2D, current_surface ? current_surface->tex : 0);
	}
}

#define RGBA_SIZE 4
int surface_load(const char *filename, Surface **surface, Surface *current_surface)
{
	assert(filename);
	assert(surface);

	int w, h;
	int n;
	Surface *tmp;
	stbi_uc *data = stbi_load(filename, &w, &h, &n, RGBA_SIZE);
	if (!data)
		return -ENOENT;
	if (w <= 0 || w > 2048 || h <= 0 || h > 2048) {
		stbi_image_free(data);
		return -E2BIG;
	}

	int potw = pow(2, ceil(log(w) / log(2)));
	int poth = pow(2, ceil(log(h) / log(2)));

	if (potw != w || poth != h) {
		int y;
		int x;
		GLubyte *pixels = new(GLubyte, potw * poth * RGBA_SIZE);
		memset(pixels, 0, potw * poth * RGBA_SIZE);
		for (y = 0; y < h; y++) {
			for (x = 0; x < w; x++) {
				int is = (x + y * w) * RGBA_SIZE;
				int id = (x + y * potw) * RGBA_SIZE;
				memcpy(pixels + id, data + is, RGBA_SIZE);
			}
		}
		tmp = surface_new(w, h, potw, poth, pixels, current_surface, NULL);
		free(pixels);
	} else {
		tmp = surface_new(w, h, w, h, data, current_surface, NULL);
	}

	stbi_image_free(data);

	tmp->filename = strdup(filename);
	*surface = tmp;

	return 0;
}

