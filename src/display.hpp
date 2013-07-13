#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include <SDL/SDL_ttf.h>
#include <SDL/SDL_opengl.h>

const int MAX_OFFSETS = 16;

const GLuint ATTR_POSITION_INDEX = 0; // WebGL wants 0 as an attribute, so here it is
const GLuint ATTR_COLOR_INDEX = 1;
const GLuint ATTR_TEXCOORD_INDEX = 2;

struct SDL_Surface;
struct Surface
{
	GLuint tex;
	GLuint fbo;
	GLuint w;
	GLuint h;
	GLuint texw;
	GLuint texh;
};

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

	GLfloat* positions;
	GLfloat* colors;
	GLfloat* texCoords; // only if IMAGE_BUFFER
	size_t current_position;
	size_t current_color;
	size_t current_texCoord;

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
};

struct Shader
{
	GLuint prog;
	GLuint vert;
	GLuint frag;
};

struct Sprite
{
	int x, y;
	int w, h;
};

class Display
{
	private:
		Buffer buffer;
		int size_x;
		int size_y;
		bool resizable;
		SDL_Surface * sdl_screen;
		Surface * screen;

		Shader* default_shader;

		Surface * current;
		Surface * current_from;

		TTF_Font* font;
		TTF_Font* fonts[128];

		float r;
		float g;
		float b;
		float alpha;

		void convert_coords(int x, int y, float *dx, float *dy);
		void convert_texcoords(int x, int y, float *dx, float *dy);
		Surface* surface_from_sdl(SDL_Surface* surf) const;

		Shader* create_default_shader();

	public:
		Display();

		void show_cursor(bool);
		void set_resizable(bool);
		void resize(int w, int h);

		void set_color(int r, int g, int b);
		void set_alpha(uint8_t a);
		void set_font(const char*, int size);

		Surface* get_screen() const;
		Surface* new_surface(uint32_t, uint32_t) const;
		Surface* load_surface(const char *) const;
		void free_surface(Surface*);
		void draw_on(Surface*);
		void draw_from(Surface*);

		void draw_background() const;
		void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3);
		void draw_line(int x1, int y1, int x2, int y2);
		void draw_surface(int, int, int, int, int, int, int, int,
							int, int, int, int, int, int, int, int);

		Surface* text_surface(const char*) const;
		void text_size(const char* text, int *w, int *h) const;
		void surface_size(Surface* surface, int *w, int *h);

		Shader* new_shader(const char* strvert, const char* strfrag);
		void use_shader(Shader*);
		void feed_shader(Shader*, const char*, float);
		void free_shader(Shader*);

		void flip();
};

#endif
