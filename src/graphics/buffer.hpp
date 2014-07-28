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

#include <cstdlib>
#define GL_GLEXT_PROTOTYPES
#ifndef EMSCRIPTEN
#include <SDL2/SDL_opengles2.h>
#else
#include <SDL/SDL_opengl.h>
#endif

struct Shader;
struct Camera;
struct Surface;

// should be multiple of 2 (for GL_LINES) and of 3 (GL_TRIANGLES)
const unsigned int BUFFER_DEFAULT_SIZE = 2 * 3 * 4096;

enum BufferType {
	UNDEFINED,
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
	GLfloat* tex_coords; // only if has_texture
	GLfloat* point_sizes; // only if POINT_BUFFER
	unsigned int current_position;
	unsigned int current_color;
	unsigned int current_tex_coord;
	unsigned int current_point_size;
	bool uploaded;

	bool _has_texture;
	Shader* shader;
	const Camera* camera;
	bool user_buffer;

	void flush();
	void upload(int method);
	void partial_free();

public:
	int ref;
	const Surface* draw_on;

	Buffer(bool user_buffer, unsigned int size = BUFFER_DEFAULT_SIZE);
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

	bool has_texture() const
	{
		return _has_texture;
	}

	BufferType get_type() const
	{
		return type;
	}

	bool is_user_buffer() const
	{
		return user_buffer;
	}

	bool is_full(int quantity=1) const
	{
		if (type == POINT_BUFFER)
			return current_position > size - 1 * quantity;
		if (type == LINE_BUFFER)
			return current_position > size - 2 * quantity;
		if (type == TRIANGLE_BUFFER)
			return current_position > size - 3 * quantity;
		return false;
	}

	bool was_freed() const
	{
		return positions == NULL;
	}
};
