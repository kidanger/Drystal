#pragma once

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengles2.h>

struct Shader;
struct Camera;

// should be multiple of 2 (for GL_LINES) and of 3 (GL_TRIANGLES)
const unsigned int BUFFER_DEFAULT_SIZE = 2 * 3 * 4096;

enum BufferType
{
	POINT_BUFFER,
	LINE_BUFFER,
	TRIANGLE_BUFFER,
};

class Buffer
{
private:
	BufferType type;

	unsigned int size;
	GLuint buffers[4]; // first is for positions, second for colors, third (optional) for texcoords, forth (optional) for point sizes
	GLfloat* positions;
	GLfloat* colors;
	GLfloat* tex_coords; // only if IMAGE_BUFFER
	GLfloat* point_sizes; // only if POINT_BUFFER
	unsigned int current_position;
	unsigned int current_color;
	unsigned int current_tex_coord;
	unsigned int current_point_size;
	bool uploaded;

	bool has_texture;
	Shader* shader;
	const Camera* camera;

	void assert_not_full();
	void flush();
	void upload(int method);
	void partial_free();

public:
	Buffer(unsigned int size=BUFFER_DEFAULT_SIZE);
	~Buffer();

	void push_vertex(GLfloat, GLfloat);
	void push_color(GLfloat, GLfloat, GLfloat, GLfloat);
	void push_tex_coord(GLfloat, GLfloat);
	void push_point_size(GLfloat);

	void use_camera(const Camera* camera);
	void use_shader(Shader* shader);
	void draw(float dx=0, float dy=0);

	void assert_type(BufferType);
	void assert_empty();
	void assert_use_texture();
	void assert_not_use_texture();

	void reset();
	void reallocate();
	void upload_and_free();
};

