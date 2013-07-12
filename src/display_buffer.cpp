#include <cassert>

#ifndef EMSCRIPTEN
#include <GLES2/gl2.h>
#endif

#include "display.hpp"

// should be multiple of 2 (for GL_LINES) and of 3 (GL_TRIANGLES)
const size_t BUFFER_DEFAULT_SIZE = 2 * 3 * 1024;

Buffer::Buffer()
{
	size = BUFFER_DEFAULT_SIZE;
	positions = new GLfloat[size * 2];
	colors = new GLfloat[size * 4];
	texCoords = new GLfloat[size * 2];

	reset();
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
	if (current_color == BUFFER_DEFAULT_SIZE) {
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
	size_t cur = current_position * 2;
	positions[cur + 0] = x;
	positions[cur + 1] = y;
	current_position += 1;
	assert_not_full();
}
void Buffer::push_color(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	size_t cur = current_color * 4;
	colors[cur + 0] = r;
	colors[cur + 1] = g;
	colors[cur + 2] = b;
	colors[cur + 3] = a;
	current_color += 1;
	assert_not_full();
}
void Buffer::push_texCoord(GLfloat x, GLfloat y)
{
	size_t cur = current_texCoord * 2;
	texCoords[cur + 0] = x;
	texCoords[cur + 1] = y;
	current_texCoord += 1;
	assert_not_full();
}

void Buffer::flush()
{
	DEBUG("");
	assert(current_color == current_position);
	assert(type != IMAGE_BUFFER or current_color == current_texCoord);

	size_t used = current_color;

	GLint prog;
	glGetIntegerv(GL_CURRENT_PROGRAM, &prog);

	glVertexAttribPointer(ATTR_POSITION_INDEX, 2, GL_FLOAT, GL_FALSE, 0, positions);
	glEnableVertexAttribArray(ATTR_POSITION_INDEX);

	glVertexAttribPointer(ATTR_COLOR_INDEX, 4, GL_FLOAT, GL_FALSE, 0, colors);
	glEnableVertexAttribArray(ATTR_COLOR_INDEX);

	if (type == IMAGE_BUFFER) {
		glVertexAttribPointer(ATTR_TEXCOORD_INDEX, 2, GL_FLOAT, GL_FALSE, 0, texCoords);
		glEnableVertexAttribArray(ATTR_TEXCOORD_INDEX);
	}
	glUniform1i(glGetUniformLocation(prog, "useTex"), type == IMAGE_BUFFER);

	glDrawArrays(type == LINE_BUFFER ? GL_LINES : GL_TRIANGLES, 0, used);
	GLDEBUG();

	glDisableVertexAttribArray(ATTR_POSITION_INDEX);
	glDisableVertexAttribArray(ATTR_COLOR_INDEX);
	if (type == IMAGE_BUFFER) {
		glDisableVertexAttribArray(ATTR_TEXCOORD_INDEX);
	}

	reset();
}

void Buffer::reset()
{
	current_position = current_color = current_texCoord = 0;
}

