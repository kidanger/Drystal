#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#define GL_GLEXT_PROTOTYPES
#include <SDL/SDL_opengl.h>

struct SDL_Surface;

#include "buffer.hpp"

const GLuint ATTR_POSITION_INDEX = 0; // WebGL wants 0 as an attribute, so here it is
const GLuint ATTR_COLOR_INDEX = 1;
const GLuint ATTR_TEXCOORD_INDEX = 2;
const GLuint ATTR_POINTSIZE_INDEX = 3;

struct Surface
{
	GLuint tex;
	GLuint fbo;
	GLuint w;
	GLuint h;
	GLuint texw;
	GLuint texh;
};


struct Shader
{
	GLuint prog_color;
	GLuint prog_tex;
	GLuint vert;
	GLuint frag_color;
	GLuint frag_tex;
};

struct Camera
{
	float dx;
	float dy;
	float dx_transformed;
	float dy_transformed;
	float zoom;
	float angle;
	float matrix[4];

	Camera() :
		dx(0), dy(0),
		dx_transformed(0), dy_transformed(0),
		zoom(1), angle(0)
	{
		matrix[0] = 0;
		matrix[1] = 0;
		matrix[2] = 0;
		matrix[3] = 0;
	}
};

enum BlendMode
{
	DEFAULT=0,
	ALPHA=0,
	ADD,
	MULT,
};

enum FilterMode
{
	NEAREST=GL_NEAREST,
	LINEAR=GL_LINEAR,
};

class Display
{
	private:
		Buffer default_buffer;
		int size_x;
		int size_y;
		bool resizable;
		SDL_Surface * sdl_screen;
		Surface * screen;

		Shader * default_shader;
		Shader * current_shader;

		const Surface * current;
		const Surface * current_from;
		FilterMode filter_mode;

		Buffer * current_buffer;

		float r;
		float g;
		float b;
		float alpha;

		Camera camera;

		float point_size;

		bool available;
		bool debug_mode;

		inline void convert_coords(float x, float y, float *dx, float *dy) {
			*dx = (2. * x / current->w) - 1;
			*dy = (2. * y / current->h) - 1;
			if (current == screen)
				*dy *= -1.;
		}
		inline void convert_texcoords(float x, float y, float *dx, float *dy) {
			*dx = x / current_from->texw;
			*dy = y / current_from->texh;
		}

		void update_camera_matrix();

		Shader* create_default_shader();

	public:
		Display();
		~Display();

		void show_cursor(bool);
		void set_resizable(bool);
		void resize(int w, int h);
		void screen2scene(float x, float y, float * tx, float * ty);
		void toggle_debug_mode();

		void set_color(int r, int g, int b);
		void set_alpha(int a);

		void get_color(int *r, int *g, int *b) { *r = this->r*255; *g = this->g*255; *b = this->b*255; };
		void get_alpha(int *a) { *a = this->alpha*255; };

		void set_point_size(float size);
		void get_point_size(float *size) { *size = this->point_size; };
		void set_line_width(float width);
		void set_blend_mode(BlendMode mode);
		void set_filter_mode(FilterMode mode);

		void reset_camera();
		void set_camera_position(float dx, float dy);
		void set_camera_angle(float angle);
		void set_camera_zoom(float zoom);
		const Camera& get_camera() { return camera; };

		Surface* get_screen() const;
		Surface* create_surface(int w, int h, int texw, int texh, unsigned char* pixels) const;
		Surface* new_surface(int w, int h) const;
		Surface* load_surface(const char *) const;
		void surface_size(Surface* surface, int *w, int *h);
		void free_surface(Surface*);

		void draw_on(const Surface*);
		void draw_from(const Surface*);
		const Surface* get_draw_on() const { return current; };
		const Surface* get_draw_from() const { return current_from; };

		void draw_background() const;
		void draw_point(float x, float y);
		void draw_point_tex(float xi, float yi, float xd, float yd);
		void draw_line(float x1, float y1, float x2, float y2);
		void draw_triangle(float x1, float y1, float x2, float y2, float x3, float y3);
		void draw_surface(float, float, float, float, float, float,
					   float, float, float, float, float, float);
		void draw_quad(float, float, float, float, float, float, float, float,
					   float, float, float, float, float, float, float, float);

		Shader* new_shader(const char* strvert, const char* strfragcolor, const char* strfragtex);
		void use_shader(Shader*);
		void feed_shader(Shader*, const char*, float);
		void free_shader(Shader*);

		Buffer* new_buffer(unsigned int size=BUFFER_DEFAULT_SIZE);
		void use_buffer(Buffer*);
		void draw_buffer(Buffer*, float dx, float dy);
		void reset_buffer(Buffer*);
		void upload_and_free_buffer(Buffer*);
		void free_buffer(Buffer*);

		void flip();
		bool is_available() const;
};

#endif
