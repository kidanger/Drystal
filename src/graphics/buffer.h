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

#include <stdbool.h>
#include <stdlib.h>

#define GL_GLEXT_PROTOTYPES
#ifndef EMSCRIPTEN
#include <SDL2/SDL_opengles2.h>
#else
#include <SDL/SDL_opengl.h>
#endif

typedef struct Buffer Buffer;

#include "shader.h"
#include "camera.h"
#include "surface.h"

// should be multiple of 2 (for GL_LINES) and of 3 (GL_TRIANGLES)
#define BUFFER_DEFAULT_SIZE 2 * 3 * 4096

enum BufferType {
	UNDEFINED,
	POINT_BUFFER,
	LINE_BUFFER,
	TRIANGLE_BUFFER,
};
typedef enum BufferType BufferType;

struct Buffer {
	BufferType type;

	unsigned int size;
	GLuint buffers[4]; // first is for positions, second for colors, third (optional) for texcoords, forth (optional) for point sizes
	GLshort* positions;
	GLubyte* colors;
	GLfloat* tex_coords; // only if has_texture
	GLfloat* point_sizes; // only if POINT_BUFFER
	unsigned int current_position;
	unsigned int current_color;
	unsigned int current_tex_coord;
	unsigned int current_point_size;
	bool uploaded;

	bool has_texture;
	Shader* shader;
	Camera* camera;
	bool user_buffer;

	int ref;
	const Surface* draw_on;
};

Buffer *buffer_new(bool user_buffer, unsigned int size);
void buffer_free(Buffer *b);
void buffer_allocate(Buffer *b);

void buffer_push_vertex(Buffer *b, GLshort, GLshort);
void buffer_push_color(Buffer *b, GLubyte, GLubyte, GLubyte, GLubyte);
void buffer_push_tex_coord(Buffer *b, GLfloat, GLfloat);
void buffer_push_point_size(Buffer *b, GLfloat);

void buffer_draw(Buffer *b, float dx, float dy);

void buffer_check_type(Buffer *b, BufferType);
void buffer_check_empty(Buffer *b);
void buffer_check_use_texture(Buffer *b);
void buffer_check_not_use_texture(Buffer *b);
void buffer_check_not_full(Buffer *b);

void buffer_upload_and_free(Buffer *b);

static inline bool buffer_was_freed(const Buffer *b)
{
	return b->positions == NULL;
}

static inline void buffer_reset(Buffer *b)
{
	assert(b);

	b->current_position = b->current_color = b->current_tex_coord = b->current_point_size = 0;
}

static inline void buffer_flush(Buffer *b)
{
	assert(b);

	buffer_draw(b, 0, 0);
	buffer_reset(b);
}

static inline void buffer_use_shader(Buffer *b, Shader *s)
{
	assert(b);
	assert(s);

	b->shader = s;
}

static inline void buffer_use_camera(Buffer *b, Camera *c)
{
	assert(b);
	assert(c);

	b->camera = c;
}

