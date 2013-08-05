#pragma once

#include <SDL/SDL_opengl.h>

enum BufferType
{
	LINE_BUFFER,
	TRIANGLE_BUFFER,
	IMAGE_BUFFER,
};
class Buffer
{
private:
	BufferType type;

	GLuint buffers[3]; // first is for positions, second for colors, and third (optional) for texcoords
	GLfloat* positions;
	GLfloat* colors;
	GLfloat* texCoords; // only if IMAGE_BUFFER
	unsigned int current_position;
	unsigned int current_color;
	unsigned int current_texCoord;

	void assert_not_full();
	void flush();
	void reset();

public:
	Buffer();
	~Buffer();

	void push_vertex(GLfloat, GLfloat);
	void push_color(GLfloat, GLfloat, GLfloat, GLfloat);
	void push_texCoord(GLfloat, GLfloat);

	void assert_type(BufferType);
	void assert_empty();

	void reallocate();
};

