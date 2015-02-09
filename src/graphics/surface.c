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
#include <stdio.h>
#include <png.h>

#include "surface.h"
#include "log.h"
#include "util.h"
#include "macro.h"

log_category("graphics");

static int png_load(const char *filename, GLubyte **data, GLuint *width, GLuint *height, SurfaceFormat *format, GLint *internal_format)
{
	png_byte header[8] = {};
	FILE *f;

	assert(data);
	assert(filename);
	assert(width);
	assert(height);
	assert(format);
	assert(internal_format);

	f = fopen(filename, "rb");
	if (!f)
		return -ENOENT;

	fread(header, 1, sizeof(header), f);

	if (png_sig_cmp(header, 0, sizeof(header))) {
		fclose(f);
		return -EBADMSG;
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
		log_oom_and_exit();

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		log_oom_and_exit();

	/* Called if there is a libpng error */
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(f);
		return -EBADMSG;
	}

	png_init_io(png_ptr, f);

	/* let libpng know we already read the header */
	png_set_sig_bytes(png_ptr, sizeof(header));

	/* read all the info up to the image data */
	png_read_info(png_ptr, info_ptr);

	int bit_depth;
	int color_type;
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);

	/* Convert index color images to RGB images */
	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);

	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(png_ptr);

	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png_ptr);

	if (bit_depth == 16)
		png_set_strip_16(png_ptr);
	else if (bit_depth < 8)
		png_set_packing(png_ptr);

	/* update the png info struct to apply transformations */
	png_read_update_info(png_ptr, info_ptr);

	png_uint_32 temp_width;
	png_uint_32 temp_height;
	png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth,
	             &color_type, NULL, NULL, NULL);

	SurfaceFormat temp_format;
	GLint temp_internal_format;
	switch (color_type) {
		case PNG_COLOR_TYPE_GRAY:
			temp_format = FORMAT_LUMINANCE;
			temp_internal_format = 1;
			break;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			temp_format = FORMAT_LUMINANCE_ALPHA;
			temp_internal_format = 2;
			break;
		case PNG_COLOR_TYPE_RGB:
			temp_format = FORMAT_RGB;
			temp_internal_format = 3;
			break;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			temp_format = FORMAT_RGBA;
			temp_internal_format = 4;
			break;
		default:
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
			fclose(f);
			return -ENOTSUP;
	}

	png_byte *image_data = xmalloc(sizeof(png_byte) * temp_width * temp_height * temp_internal_format);
	png_bytep *row_pointers = xmalloc(sizeof(png_bytep) * temp_height);
	for (unsigned i = 0; i < temp_height; ++i) {
		row_pointers[i] = image_data + (i * temp_width * temp_internal_format);
	}

	/* read the image into image_data throught row_pointers */
	png_read_image(png_ptr, row_pointers);
	png_read_end(png_ptr, NULL);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	free(row_pointers);
	fclose(f);

	*width = temp_width;
	*height = temp_height;
	*format = temp_format;
	*internal_format = temp_internal_format;
	*data = image_data;

	return 0;
}

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

Surface *surface_new(unsigned int w, unsigned int h, unsigned int texw, unsigned int texh,
					 SurfaceFormat format, void *pixels, Surface *current_from, Surface *current_on)
{
	Surface *s = new(Surface, 1);
	s->filename = NULL;
	s->w = w;
	s->h = h;
	s->texw = texw;
	s->texh = texh;
	s->filter = FILTER_DEFAULT;
	s->has_fbo = false;
	s->has_mipmap = false;
	s->npot = false;
	s->ref = 0;
	s->tex = 0;
	s->fbo = 0;

	glGenTextures(1, &(s->tex));
	glBindTexture(GL_TEXTURE_2D, s->tex);

	glTexImage2D(GL_TEXTURE_2D, 0, format, s->texw, s->texh,
				 0, format, GL_UNSIGNED_BYTE, 0);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, s->w, s->h,
					format, GL_UNSIGNED_BYTE, pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, s->filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, s->filter);
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

int surface_load(const char *filename, Surface **surface, Surface *current_surface)
{
	assert(filename);
	assert(surface);

	GLuint w, h;
	SurfaceFormat format;
	GLint internal_format;
	GLubyte *data;
	int r;

	r = png_load(filename, &data, &w, &h, &format, &internal_format);
	if (r < 0)
		return r;
	if (w <= 0 || w > 2048 || h <= 0 || h > 2048) {
		free(data);
		return -E2BIG;
	}

	GLuint potw = pow(2, (int) ceil(log(w) / log(2)));
	GLuint poth = pow(2, (int) ceil(log(h) / log(2)));
	*surface = surface_new(w, h, potw, poth, format, data, current_surface, NULL);
	(*surface)->filename = xstrdup(filename);

	free(data);
	return 0;
}

