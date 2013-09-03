#include <cassert>

#ifndef EMSCRIPTEN
#include <GLES2/gl2.h>
#endif

#include "log.hpp"
#include "stats.hpp"
#include "display.hpp"
#include "buffer.hpp"

Buffer::Buffer(unsigned int size) :
	size(size),
	positions(new GLfloat[size * 2]),
	colors(new GLfloat[size * 4]),
	tex_coords(new GLfloat[size * 2]),
	point_sizes(new GLfloat[size]),
	has_texture(false)
{
	current_position = 0;
	current_color = 0;
	current_tex_coord = 0;
	current_point_size = 0;
	buffers[0] = 0;
	buffers[1] = 0;
	buffers[2] = 0;
	buffers[3] = 0;
}

void Buffer::reallocate()
{
	DEBUG("");
	reset();
	if (buffers[0] > 0)
	{
		glDeleteBuffers(4, buffers);
		buffers[0] = 0;
		buffers[1] = 0;
		buffers[2] = 0;
		buffers[3] = 0;
	}
	glGenBuffers(4, buffers);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glEnableVertexAttribArray(ATTR_POSITION_INDEX);
	glEnableVertexAttribArray(ATTR_COLOR_INDEX);
	type = TRIANGLE_BUFFER;
}

Buffer::~Buffer()
{
	glDeleteBuffers(4, buffers);
	delete[] positions;
	delete[] colors;
	delete[] tex_coords;
	delete[] point_sizes;
}

void Buffer::use_shader(Shader* shader)
{
	this->shader = shader;
}

void Buffer::assert_type(BufferType atype)
{
	if (type != atype) {
		flush();
	}
	type = atype;
}

void Buffer::assert_not_full()
{
	if (current_color == size) {
		flush();
	}
}

void Buffer::assert_empty()
{
	if (current_color != 0) {
		flush();
	}
}

void Buffer::assert_use_texture()
{
	if (not has_texture) {
		flush();
		has_texture = true;
	}
}

void Buffer::assert_not_use_texture()
{
	if (has_texture) {
		flush();
		has_texture = false;
	}
}


void Buffer::push_vertex(GLfloat x, GLfloat y)
{
	assert_not_full();
	size_t cur = current_position * 2;
	positions[cur + 0] = x;
	positions[cur + 1] = y;
	current_position += 1;
}
void Buffer::push_color(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	assert_not_full();
	size_t cur = current_color * 4;
	colors[cur + 0] = r;
	colors[cur + 1] = g;
	colors[cur + 2] = b;
	colors[cur + 3] = a;
	current_color += 1;
}
void Buffer::push_tex_coord(GLfloat x, GLfloat y)
{
	assert_not_full();
	size_t cur = current_tex_coord * 2;
	tex_coords[cur + 0] = x;
	tex_coords[cur + 1] = y;
	current_tex_coord += 1;
}
void Buffer::push_point_size(GLfloat s)
{
	assert_not_full();
	size_t cur = current_point_size;
	point_sizes[cur] = s;
	current_point_size += 1;
}

void Buffer::draw(float dx, float dy)
{
	size_t used = current_color;
	if (used == 0) {
		return;
	}

	DEBUG();
	assert(current_color == current_position);
	assert(not has_texture or current_color == current_tex_coord);
	assert(type != POINT_BUFFER or current_color == current_point_size);

	GLint prog;
	if (has_texture) {
		prog = shader->prog_tex;
	} else {
		prog = shader->prog_color;
	}
	glUseProgram(prog);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glVertexAttribPointer(ATTR_POSITION_INDEX, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glBufferData(GL_ARRAY_BUFFER, used * 2 * sizeof(GLfloat), positions, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glVertexAttribPointer(ATTR_COLOR_INDEX, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
	glBufferData(GL_ARRAY_BUFFER, used * 4 * sizeof(GLfloat), colors, GL_DYNAMIC_DRAW);

	if (has_texture) {
		glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
		glBufferData(GL_ARRAY_BUFFER, used * 2 * sizeof(GLfloat), tex_coords, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(ATTR_TEXCOORD_INDEX);
		glVertexAttribPointer(ATTR_TEXCOORD_INDEX, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	}
	if (type == POINT_BUFFER) {
		glBindBuffer(GL_ARRAY_BUFFER, buffers[3]);
		glBufferData(GL_ARRAY_BUFFER, used * sizeof(GLfloat), point_sizes, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(ATTR_POINTSIZE_INDEX);
		glVertexAttribPointer(ATTR_POINTSIZE_INDEX, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
	}

	glUniform1f(glGetUniformLocation(prog, "dx"), dx);
	glUniform1f(glGetUniformLocation(prog, "dy"), dy);

	GLint draw_type;
	if (type == POINT_BUFFER) {
		draw_type = GL_POINTS;
	} else if (type == LINE_BUFFER) {
		draw_type = GL_LINES;
	} else {
		draw_type = GL_TRIANGLES;
	}
	glDrawArrays(draw_type, 0, used);

	if (has_texture) {
		glDisableVertexAttribArray(ATTR_TEXCOORD_INDEX);
	}
	if (type == POINT_BUFFER) {
		glDisableVertexAttribArray(ATTR_POINTSIZE_INDEX);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);

#ifdef STATS
	stats.add_flush(used);
#endif
}

void Buffer::flush()
{
	draw();
	reset();
}

void Buffer::reset()
{
	current_position = current_color = current_tex_coord = current_point_size = 0;
}

