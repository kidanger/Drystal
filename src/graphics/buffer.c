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

#include "buffer.h"
#include "display.h"
#include "shader.h"
#include "util.h"
#include "log.h"
#include "opengl_util.h"

log_category("buffer");

static void buffer_upload(Buffer *b, int method)
{
	size_t used;

	assert(b);

	used = b->current_color;
	if (used == 0 || b->uploaded) {
		return;
	}

	glBindBuffer(GL_ARRAY_BUFFER, b->buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, used * 2 * sizeof(GLfloat), b->positions, method);
	check_opengl_oom();

	glBindBuffer(GL_ARRAY_BUFFER, b->buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, used * 4 * sizeof(GLubyte), b->colors, method);
	check_opengl_oom();

	if (b->has_texture) {
		glBindBuffer(GL_ARRAY_BUFFER, b->buffers[2]);
		glBufferData(GL_ARRAY_BUFFER, used * 2 * sizeof(GLfloat), b->tex_coords, method);
		check_opengl_oom();
	}
	b->uploaded = true;
}

static void buffer_partial_free(Buffer *b)
{
	if (!b)
		return;

	free(b->positions);
	free(b->colors);
	free(b->tex_coords);
	b->positions = NULL;
	b->colors = NULL;
	b->tex_coords = NULL;
}

static bool buffer_is_full(const Buffer *b)
{
	assert(b);

	return b->current_position > b->size - 3;
}

static void buffer_resize(Buffer *b) {
	size_t size_positions = b->size * 2;
	size_t size_colors = b->size * 4;
	size_t size_tex_coords = b->size * 2;

	XREALLOC(b->positions, size_positions, size_positions + 2);
	XREALLOC(b->colors, size_colors, size_colors + 4);
	if (b->tex_coords) {
		XREALLOC(b->tex_coords, size_tex_coords, size_tex_coords + 2);
	}
	b->size = size_colors / 4;
	log_info("new size: %u", b->size);
}

Buffer *buffer_new(bool user_buffer, unsigned int size)
{
	Buffer *b = new0(Buffer, 1);
	b->size = size;
	b->positions = new(GLfloat, size * 2);
	b->colors = new(GLubyte, size * 4);
	b->user_buffer = user_buffer;

	return b;
}

void buffer_allocate(Buffer *b)
{
	assert(b);

	glGenBuffers(3, b->buffers);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glEnableVertexAttribArray(ATTR_LOCATION_POSITION);
	glEnableVertexAttribArray(ATTR_LOCATION_COLOR);
}

void buffer_free(Buffer *b)
{
	if (!b)
		return;

	glDeleteBuffers(3, b->buffers);
	buffer_partial_free(b);
	free(b);
}

void buffer_check_not_full(Buffer *b)
{
	assert(b);

	if (buffer_is_full(b)) {
		if (b->user_buffer) {
			buffer_resize(b);
		} else {
			buffer_flush(b);
		}
	}
}

void buffer_check_empty(Buffer *b)
{
	assert(b);

	if (b->current_color != 0) {
		buffer_flush(b);
	}
}

void buffer_check_use_texture(Buffer *b)
{
	assert(b);

	if (!b->has_texture) {
		buffer_flush(b);
		b->has_texture = true;
	}
	if (b->tex_coords == NULL) {
		b->tex_coords = new(GLfloat, b->size * 2);
	}
}

void buffer_check_not_use_texture(Buffer *b)
{
	assert(b);

	if (b->has_texture) {
		buffer_flush(b);
		b->has_texture = false;
	}
}

void buffer_push_vertex(Buffer *b, GLfloat x, GLfloat y)
{
	size_t cur;

	assert(b);
	assert(b->current_position < b->size);

	cur = b->current_position * 2;

	b->positions[cur + 0] = x;
	b->positions[cur + 1] = y;
	b->current_position += 1;
	b->uploaded = false;
}

void buffer_push_color(Buffer *buffer, GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
	size_t cur;

	assert(buffer);
	assert(buffer->current_color < buffer->size);

	cur = buffer->current_color * 4;

	buffer->colors[cur + 0] = r;
	buffer->colors[cur + 1] = g;
	buffer->colors[cur + 2] = b;
	buffer->colors[cur + 3] = a;
	buffer->current_color += 1;
	buffer->uploaded = false;
}

void buffer_push_tex_coord(Buffer *b, GLfloat x, GLfloat y)
{
	size_t cur;

	assert(b);
	assert(b->current_tex_coord < b->size);

	cur = b->current_tex_coord * 2;

	b->tex_coords[cur + 0] = x;
	b->tex_coords[cur + 1] = y;
	b->current_tex_coord += 1;
	b->uploaded = false;
}

void buffer_upload_and_free(Buffer *b)
{
	assert(b);

	buffer_upload(b, GL_STATIC_DRAW);
	buffer_partial_free(b);
}

void buffer_draw(Buffer *b, float dx, float dy)
{
	size_t used;

	assert(b);

	used = b->current_color;
	if (used == 0) {
		return;
	}

	Shader* shader = b->shader;

	assert(b->current_color == b->current_position);
	assert(!b->has_texture || b->current_color == b->current_tex_coord);
	assert(shader);
	assert(b->camera);

	GLint prog;
	VarLocationIndex locationIndex;
	if (b->has_texture) {
		prog = shader->prog_tex;
		locationIndex = VAR_LOCATION_TEX;
	} else {
		prog = shader->prog_color;
		locationIndex = VAR_LOCATION_COLOR;
	}
	glUseProgram(prog);

	if (!b->uploaded)
		buffer_upload(b, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, b->buffers[0]);
	glVertexAttribPointer(ATTR_LOCATION_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, b->buffers[1]);
	glVertexAttribPointer(ATTR_LOCATION_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, NULL);

	if (b->has_texture) {
		glBindBuffer(GL_ARRAY_BUFFER, b->buffers[2]);
		glEnableVertexAttribArray(ATTR_LOCATION_TEXCOORD);
		glVertexAttribPointer(ATTR_LOCATION_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	dx -= b->camera->dx;
	dy -= b->camera->dy;
	glUniform1f(shader->vars[locationIndex].dxLocation, dx);
	glUniform1f(shader->vars[locationIndex].dyLocation, dy);
	glUniform1f(shader->vars[locationIndex].zoomLocation, b->camera->zoom);
	glUniformMatrix2fv(shader->vars[locationIndex].rotationMatrixLocation, 1, GL_FALSE, b->camera->matrix);
	glUniform2f(shader->vars[locationIndex].destinationSizeLocation, b->draw_on->texw, b->draw_on->texh);
	if (b->draw_from)
		glUniform2f(shader->vars[locationIndex].sourceSizeLocation, b->draw_from->texw, b->draw_from->texh);

	glDrawArrays(GL_TRIANGLES, 0, used);

	if (b->has_texture) {
		glDisableVertexAttribArray(ATTR_LOCATION_TEXCOORD);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

