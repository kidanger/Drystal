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

	glBindBuffer(GL_ARRAY_BUFFER, b->buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, used * 4 * sizeof(GLfloat), b->colors, method);

	if (b->has_texture) {
		glBindBuffer(GL_ARRAY_BUFFER, b->buffers[2]);
		glBufferData(GL_ARRAY_BUFFER, used * 2 * sizeof(GLfloat), b->tex_coords, method);
	}
	if (b->type == POINT_BUFFER) {
		glBindBuffer(GL_ARRAY_BUFFER, b->buffers[3]);
		glBufferData(GL_ARRAY_BUFFER, used * sizeof(GLfloat), b->point_sizes, method);
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
	free(b->point_sizes);
	b->positions = b->colors = b->tex_coords = b->point_sizes = NULL;
}

Buffer *buffer_new(bool user_buffer, unsigned int size)
{
	Buffer *b = new(Buffer, 1);
	b->type = UNDEFINED;
	b->size = size;
	b->positions = new(GLfloat, size * 2);
	b->colors = new(GLfloat, size * 4);
	b->tex_coords = NULL;
	b->point_sizes = NULL;
	b->current_position = 0;
	b->current_color = 0;
	b->current_tex_coord = 0;
	b->current_point_size = 0;
	b->uploaded = false;
	b->has_texture = false;
	b->shader = NULL;
	b->camera = NULL;
	b->user_buffer = user_buffer;
	b->ref = 0;
	b->draw_on = NULL;

	b->buffers[0] = 0;
	b->buffers[1] = 0;
	b->buffers[2] = 0;
	b->buffers[3] = 0;

	return b;
}

void buffer_allocate(Buffer *b)
{
	assert(b);

	glGenBuffers(4, b->buffers);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glEnableVertexAttribArray(ATTR_LOCATION_POSITION);
	glEnableVertexAttribArray(ATTR_LOCATION_COLOR);
	b->type = UNDEFINED;
}

void buffer_free(Buffer *b)
{
	if (!b)
		return;

	glDeleteBuffers(4, b->buffers);
	buffer_partial_free(b);
	free(b);
}

void buffer_check_type(Buffer *b, BufferType atype)
{
	assert(b);

	if (b->type != atype) {
		buffer_flush(b);

		b->type = atype;
		if (b->type == POINT_BUFFER && b->point_sizes == NULL) {
			b->point_sizes = new(GLfloat, b->size);
		}
	}
}

void buffer_check_not_full(Buffer *b)
{
	assert(b);

	if (buffer_is_full(b)) {
		buffer_flush(b);
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

	cur = b->current_position * 2;

	assert(cur < b->size * 2);

	b->positions[cur + 0] = x;
	b->positions[cur + 1] = y;
	b->current_position += 1;
	b->uploaded = false;
}

void buffer_push_color(Buffer *buffer, GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	size_t cur;

	assert(buffer);

	cur = buffer->current_color * 4;

	assert(cur < buffer->size * 4);

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

	cur = b->current_tex_coord * 2;

	assert(cur < b->size * 2);

	b->tex_coords[cur + 0] = x;
	b->tex_coords[cur + 1] = y;
	b->current_tex_coord += 1;
	b->uploaded = false;
}

void buffer_push_point_size(Buffer *b, GLfloat s)
{
	size_t cur;

	assert(b);

	cur = b->current_point_size;

	assert(cur < b->size);

	b->point_sizes[cur] = s;
	b->current_point_size += 1;
	b->uploaded = false;
}

void buffer_upload_and_free(Buffer *b)
{
	assert(b);

	buffer_upload(b, GL_STATIC_DRAW);
	buffer_partial_free(b);
}

bool buffer_is_fulln(const Buffer *b, int quantity)
{
	assert(b);

	if (b->type == POINT_BUFFER)
		return b->current_position > b->size - 1 * quantity;
	if (b->type == LINE_BUFFER)
		return b->current_position > b->size - 2 * quantity;
	if (b->type == TRIANGLE_BUFFER)
		return b->current_position > b->size - 3 * quantity;
	return false;
}

void buffer_draw(Buffer *b, float dx, float dy)
{
	size_t used;

	assert(b);

	used = b->current_color;
	if (used == 0) {
		return;
	}

	assert(b->current_color == b->current_position);
	assert(!b->has_texture || b->current_color == b->current_tex_coord);
	assert(b->type != POINT_BUFFER || b->current_color == b->current_point_size);
	assert(b->type != UNDEFINED);
	assert(b->shader);
	assert(b->camera);

	GLint prog;
	VarLocationIndex locationIndex;
	if (b->has_texture) {
		prog = b->shader->prog_tex;
		locationIndex = VAR_LOCATION_TEX;
	} else {
		prog = b->shader->prog_color;
		locationIndex = VAR_LOCATION_COLOR;
	}
	glUseProgram(prog);

	if (!b->uploaded)
		buffer_upload(b, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, b->buffers[0]);
	glVertexAttribPointer(ATTR_LOCATION_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, b->buffers[1]);
	glVertexAttribPointer(ATTR_LOCATION_COLOR, 4, GL_FLOAT, GL_FALSE, 0, NULL);

	if (b->has_texture) {
		glBindBuffer(GL_ARRAY_BUFFER, b->buffers[2]);
		glEnableVertexAttribArray(ATTR_LOCATION_TEXCOORD);
		glVertexAttribPointer(ATTR_LOCATION_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	}
	if (b->type == POINT_BUFFER) {
		glBindBuffer(GL_ARRAY_BUFFER, b->buffers[3]);
		glEnableVertexAttribArray(ATTR_LOCATION_POINTSIZE);
		glVertexAttribPointer(ATTR_LOCATION_POINTSIZE, 1, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	dx -= b->camera->dx;
	dy -= b->camera->dy;
	glUniform1f(b->shader->vars[locationIndex].dxLocation, dx);
	glUniform1f(b->shader->vars[locationIndex].dyLocation, dy);
	glUniform1f(b->shader->vars[locationIndex].zoomLocation, b->camera->zoom);
	glUniformMatrix2fv(b->shader->vars[locationIndex].rotationMatrixLocation, 1, GL_FALSE, b->camera->matrix);
	glUniform2f(b->shader->vars[locationIndex].destinationSizeLocation, b->draw_on->texw, b->draw_on->texh);

	GLint draw_type;
	if (b->type == POINT_BUFFER) {
		draw_type = GL_POINTS;
	} else if (b->type == LINE_BUFFER) {
		draw_type = GL_LINES;
	} else {
		draw_type = GL_TRIANGLES;
	}
	glDrawArrays(draw_type, 0, used);

	if (b->has_texture) {
		glDisableVertexAttribArray(ATTR_LOCATION_TEXCOORD);
	}
	if (b->type == POINT_BUFFER) {
		glDisableVertexAttribArray(ATTR_LOCATION_POINTSIZE);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

