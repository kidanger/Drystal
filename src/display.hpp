#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include <SDL/SDL_ttf.h>
#include <SDL/SDL_opengl.h>

const int MAX_OFFSETS = 16;

struct SDL_Surface;
struct Surface
{
	GLuint tex;
	GLuint fbo;
	GLuint w;
	GLuint h;
	GLuint texw;
	GLuint texh;

	GLfloat angle;
	GLuint resizew;
	GLuint resizeh;
};

struct Shader
{
	GLuint prog;
	GLuint vert;
	GLuint frag;
};

struct Sprite;

class Display
{
	private:
		int size_x;
		int size_y;
		bool resizable;
		SDL_Surface * sdl_screen;
		Surface * screen;
		Surface * current;
		Surface * current_from;
		TTF_Font* fonts[128];

		TTF_Font* font;
		int16_t round;

		int offsetsx[MAX_OFFSETS];
		int offsetsy[MAX_OFFSETS];
		int offset_current;
		int offx, offy;
		float r, g, b;
		float alpha;
		bool fill;

		Surface* surface_from_sdl(SDL_Surface* surf);
	public:
		void init();
		void show_cursor(bool);
		void set_resizable(bool);
		void resize(int w, int h);

		void push_offset(int, int);
		void pop_offset();

		void set_color(int r, int g, int b);
		void set_alpha(uint8_t a);
		void set_font(const char*, int size);
		void set_round(uint16_t round);
		void set_fill(bool fill);

		Surface* get_screen();
		Surface* new_surface(uint32_t, uint32_t);
		Surface* load_surface(const char *);
		void free_surface(Surface*);
		void rotate_surface(Surface*, double);
		void resize_surface(Surface*, int, int);
		void draw_on(Surface*);
		void draw_from(Surface*);

		void draw_background();
		void draw_surface(Surface*, int x, int y);
		void draw_sprite(const Sprite& sp, int x, int y);
		void draw_rect(int x, int y, int w, int h);
		void draw_circle(int x, int y, int rad);
		void draw_arc(int x, int y, int radius, int rad1, int rad2);
		void draw_line(int x, int y, int x2, int y2);

		Surface* text_surface(const char*);
		void text_size(const char* text, int *w, int *h);
		void surface_size(Surface* surface, int *w, int *h);

		Shader* new_shader(const char* strvert, const char* strfrag);
		void use_shader(Shader*);
		void feed_shader(Shader*, const char*, float);
		void free_shader(Shader*);

		void flip();
};

#endif
