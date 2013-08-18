#pragma once

#include <SDL/SDL_opengl.h>

// should be multiple of 2 (for GL_LINES) and of 3 (GL_TRIANGLES)
const unsigned int BUFFER_DEFAULT_SIZE = 2 * 3 * 4096;

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

	unsigned int size;
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
	Buffer(unsigned int size=BUFFER_DEFAULT_SIZE);
	~Buffer();

	void push_vertex(GLfloat, GLfloat);
	void push_color(GLfloat, GLfloat, GLfloat, GLfloat);
	void push_texCoord(GLfloat, GLfloat);

	void draw(float dx=0, float dy=0);

	void assert_type(BufferType);
	void assert_empty();

	void reallocate();
};

