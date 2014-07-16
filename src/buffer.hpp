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

#define GL_GLEXT_PROTOTYPES
#ifndef EMSCRIPTEN
#include <SDL2/SDL_opengles2.h>
#else
#include <SDL/SDL_opengl.h>
#endif

struct Shader;
struct Camera;

// should be multiple of 2 (for GL_LINES) and of 3 (GL_TRIANGLES)
const unsigned int BUFFER_DEFAULT_SIZE = 2 * 3 * 4096;

enum BufferType {
	POINT_BUFFER,
	LINE_BUFFER,
	TRIANGLE_BUFFER,
};

class Buffer
{
private:
	Buffer(const Buffer&);
	Buffer& operator=(const Buffer&);
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

	void flush();
	void upload(int method);
	void partial_free();

public:
	Buffer(unsigned int _size = BUFFER_DEFAULT_SIZE);
	~Buffer();

	void push_vertex(GLfloat, GLfloat);
	void push_color(GLfloat, GLfloat, GLfloat, GLfloat);
	void push_tex_coord(GLfloat, GLfloat);
	void push_point_size(GLfloat);

	void use_camera(const Camera* _camera);
	void use_shader(Shader* _shader);
	void draw(float dx = 0, float dy = 0);

	void check_type(BufferType);
	void check_empty();
	void check_use_texture();
	void check_not_use_texture();
	void check_not_full();

	void reset();
	void upload_and_free();
	void allocate();
};
