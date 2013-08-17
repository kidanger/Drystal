#include <cassert>

#ifndef EMSCRIPTEN
#include <GLES2/gl2.h>
#endif

#include "log.hpp"
#include "stats.hpp"
#include "display.hpp"
#include "display_buffer.hpp"

Buffer::Buffer(unsigned int size) :
	size(size),
	positions(new GLfloat[size * 2]),
	colors(new GLfloat[size * 4]),
	texCoords(new GLfloat[size * 2])
{
	current_position = 0;
	current_color = 0;
	current_texCoord = 0;
	buffers[0] = 0;
	buffers[1] = 0;
	buffers[2] = 0;
}

void Buffer::reallocate()
{
	DEBUG("");
	reset();
	if (buffers[0] > 0)
	{
		glDeleteBuffers(3, buffers);
		buffers[0] = 0;
		buffers[1] = 0;
		buffers[2] = 0;
	}
	glGenBuffers(3, buffers);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glVertexAttribPointer(ATTR_POSITION_INDEX, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glVertexAttribPointer(ATTR_COLOR_INDEX, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
	glVertexAttribPointer(ATTR_TEXCOORD_INDEX, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glEnableVertexAttribArray(ATTR_POSITION_INDEX);
	glEnableVertexAttribArray(ATTR_COLOR_INDEX);
	type = TRIANGLE_BUFFER;
}

Buffer::~Buffer()
{
	glDeleteBuffers(3, buffers);
	delete positions;
	delete colors;
	delete texCoords;
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
		flush(); // implicit reset
	}
}

void Buffer::assert_empty()
{
	if (current_color != 0) {
		flush(); // implicit reset
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
void Buffer::push_texCoord(GLfloat x, GLfloat y)
{
	assert_not_full();
	size_t cur = current_texCoord * 2;
	texCoords[cur + 0] = x;
	texCoords[cur + 1] = y;
	current_texCoord += 1;
}

void Buffer::draw()
{
	flush(false);
}


void Buffer::flush(bool do_reset)
{
	DEBUG();
	assert(current_color == current_position);
	assert(type != IMAGE_BUFFER or current_color == current_texCoord);

	size_t used = current_color;

	GLint prog;
	glGetIntegerv(GL_CURRENT_PROGRAM, &prog);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, used * 2 * sizeof(GLfloat), positions, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, used * 4 * sizeof(GLfloat), colors, GL_DYNAMIC_DRAW);

	if (type == IMAGE_BUFFER) {
		glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
		glBufferData(GL_ARRAY_BUFFER, used * 2 * sizeof(GLfloat), texCoords, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(ATTR_TEXCOORD_INDEX);
	}

	glUniform1i(glGetUniformLocation(prog, "useTex"), type == IMAGE_BUFFER);
	glDrawArrays(type == LINE_BUFFER ? GL_LINES : GL_TRIANGLES, 0, used);

	if (type == IMAGE_BUFFER) {
		glDisableVertexAttribArray(ATTR_TEXCOORD_INDEX);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);

#ifdef STATS
	stats.add_flush(used);
#endif

	if (do_reset)
		reset();
}

void Buffer::reset()
{
	current_position = current_color = current_texCoord = 0;
}

