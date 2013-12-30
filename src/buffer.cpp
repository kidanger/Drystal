#include <cstdlib>
#include <cassert>

#include "buffer.hpp"
#include "log.hpp"
#include "stats.hpp"
#include "display.hpp"


Buffer::Buffer(unsigned int _size) :
	type(TRIANGLE_BUFFER),
	size(_size),
	positions(new GLfloat[size * 2]),
	colors(new GLfloat[size * 4]),
	tex_coords(NULL),
	point_sizes(NULL),
	current_position(0),
	current_color(0),
	current_tex_coord(0),
	current_point_size(0),
	uploaded(false),
	has_texture(false),
	shader(NULL),
	camera(NULL)
{
	buffers[0] = 0;
	buffers[1] = 0;
	buffers[2] = 0;
	buffers[3] = 0;
}

void Buffer::allocate()
{
	glGenBuffers(4, buffers);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glEnableVertexAttribArray(ATTR_POSITION_INDEX);
	glEnableVertexAttribArray(ATTR_COLOR_INDEX);
	type = TRIANGLE_BUFFER;
}

Buffer::~Buffer()
{
	glDeleteBuffers(4, buffers);
	partial_free();
}

void Buffer::partial_free()
{
	if (positions)
		delete[] positions;
	if (colors)
		delete[] colors;
	if (tex_coords)
		delete[] tex_coords;
	if (point_sizes)
		delete[] point_sizes;
	positions = colors = tex_coords = point_sizes = NULL;
}

void Buffer::use_camera(const Camera* _camera)
{
	this->camera = _camera;
}

void Buffer::use_shader(Shader* _shader)
{
	this->shader = _shader;
}

void Buffer::check_type(BufferType atype)
{
	if (type != atype) {
		flush();
	}
	type = atype;
	if (type == POINT_BUFFER && point_sizes == NULL) {
		point_sizes = new GLfloat[size];
	}

}

void Buffer::check_not_full()
{
	if (current_color > size - 3) { // if triangle, we will push at most 3 more elements
		flush(); // so flush if there is less than 3 space remaining
	}
}

void Buffer::check_empty()
{
	if (current_color != 0) {
		flush();
	}
}

void Buffer::check_use_texture()
{
	if (!has_texture) {
		flush();
		has_texture = true;
	}
	if (tex_coords == NULL) {
		tex_coords = new GLfloat[size * 2];
	}
}

void Buffer::check_not_use_texture()
{
	if (has_texture) {
		flush();
		has_texture = false;
	}
}


void Buffer::push_vertex(GLfloat x, GLfloat y)
{
	size_t cur = current_position * 2;
	positions[cur + 0] = x;
	positions[cur + 1] = y;
	current_position += 1;
	uploaded = false;
}
void Buffer::push_color(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	size_t cur = current_color * 4;
	colors[cur + 0] = r;
	colors[cur + 1] = g;
	colors[cur + 2] = b;
	colors[cur + 3] = a;
	current_color += 1;
	uploaded = false;
}
void Buffer::push_tex_coord(GLfloat x, GLfloat y)
{
	size_t cur = current_tex_coord * 2;
	tex_coords[cur + 0] = x;
	tex_coords[cur + 1] = y;
	current_tex_coord += 1;
	uploaded = false;
}
void Buffer::push_point_size(GLfloat s)
{
	size_t cur = current_point_size;
	point_sizes[cur] = s;
	current_point_size += 1;
	uploaded = false;
}

void Buffer::upload(int method)
{
	size_t used = current_color;
	if (used == 0 || uploaded) {
		return;
	}

	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, used * 2 * sizeof(GLfloat), positions, method);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, used * 4 * sizeof(GLfloat), colors, method);

	if (has_texture) {
		glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
		glBufferData(GL_ARRAY_BUFFER, used * 2 * sizeof(GLfloat), tex_coords, method);
	}
	if (type == POINT_BUFFER) {
		glBindBuffer(GL_ARRAY_BUFFER, buffers[3]);
		glBufferData(GL_ARRAY_BUFFER, used * sizeof(GLfloat), point_sizes, method);
	}
	uploaded = true;
}

void Buffer::draw(float dx, float dy)
{
	size_t used = current_color;
	if (used == 0) {
		return;
	}

	DEBUG();
	assert(current_color == current_position);
	assert(!has_texture || current_color == current_tex_coord);
	assert(type != POINT_BUFFER || current_color == current_point_size);

	GLint prog;
	VarLocationIndex locationIndex;
	if (has_texture) {
		prog = shader->prog_tex;
		locationIndex = TEX;
	} else {
		prog = shader->prog_color;
		locationIndex = COLOR;
	}
	glUseProgram(prog);

	if (!uploaded)
		upload(GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glVertexAttribPointer(ATTR_POSITION_INDEX, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glVertexAttribPointer(ATTR_COLOR_INDEX, 4, GL_FLOAT, GL_FALSE, 0, NULL);

	if (has_texture) {
		glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
		glEnableVertexAttribArray(ATTR_TEXCOORD_INDEX);
		glVertexAttribPointer(ATTR_TEXCOORD_INDEX, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	}
	if (type == POINT_BUFFER) {
		glBindBuffer(GL_ARRAY_BUFFER, buffers[3]);
		glEnableVertexAttribArray(ATTR_POINTSIZE_INDEX);
		glVertexAttribPointer(ATTR_POINTSIZE_INDEX, 1, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	dx += camera->dx_transformed;
	dy += camera->dy_transformed;
	glUniform1f(shader->vars[locationIndex].dxLocation, dx);
	glUniform1f(shader->vars[locationIndex].dyLocation, dy);
	glUniform1f(shader->vars[locationIndex].zoomLocation, camera->zoom);
	glUniformMatrix2fv(shader->vars[locationIndex].rotationMatrixLocation, 1, GL_FALSE, camera->matrix);

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

void Buffer::upload_and_free()
{
	upload(GL_STATIC_DRAW);
	partial_free();
}

