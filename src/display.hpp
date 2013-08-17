#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include <SDL/SDL_opengl.h>
struct SDL_Surface;

#include "display_buffer.hpp"

const GLuint ATTR_POSITION_INDEX = 0; // WebGL wants 0 as an attribute, so here it is
const GLuint ATTR_COLOR_INDEX = 1;
const GLuint ATTR_TEXCOORD_INDEX = 2;

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
	GLuint prog;
	GLuint vert;
	GLuint frag;
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

		const Surface * current;
		const Surface * current_from;

		Buffer * current_buffer;

		float r;
		float g;
		float b;
		float alpha;
		bool available;

		inline void convert_coords(int x, int y, float *dx, float *dy) {
			*dx = (2.0 * x / current->w) - 1;
			*dy = (2.0 * y / current->h) - 1;
			if (current == screen)
				*dy *= -1.0;
		}
		inline void convert_texcoords(int x, int y, float *dx, float *dy) {
			*dx = (float) x / current_from->texw;
			*dy = (float) y / current_from->texh;
		}

		Shader* create_default_shader();

	public:
		Display();

		void show_cursor(bool);
		void set_resizable(bool);
		void resize(int w, int h);

		void set_color(int r, int g, int b);
		void set_alpha(int a);

		void get_color(int *r, int *g, int *b) { *r = this->r*255; *g = this->g*255; *b = this->b*255; };
		void get_alpha(int *a) { *a = this->alpha*255; };

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
		void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3);
		void draw_line(int x1, int y1, int x2, int y2);
		void draw_surface(int, int, int, int, int, int, int, int,
							int, int, int, int, int, int, int, int);

		Shader* new_shader(const char* strvert, const char* strfrag);
		void use_shader(Shader*);
		void feed_shader(Shader*, const char*, float);
		void free_shader(Shader*);

		Buffer* new_buffer(unsigned int size=BUFFER_DEFAULT_SIZE);
		void use_buffer(Buffer*);
		void draw_buffer(Buffer*);
		void free_buffer(Buffer*);

		void flip();
		bool is_available() const;
};

#endif
