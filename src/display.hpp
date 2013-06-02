#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include <SDL/SDL_ttf.h>

const int MAX_OFFSETS = 16;

struct SDL_Surface;

struct Sprite;

class Display
{
	private:
		int size_x;
		int size_y;
		bool resizable;
		SDL_Surface * screen;
		SDL_Surface * current;
		SDL_Surface * current_from;
		TTF_Font* fonts[128];

		TTF_Font* font;
		int16_t round;

		int offsetsx[MAX_OFFSETS];
		int offsetsy[MAX_OFFSETS];
		int offset_current;
		int offx, offy;
		uint8_t r, g, b;
		uint8_t alpha;
		bool fill;

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

		SDL_Surface* get_screen();
		SDL_Surface* new_surface(uint32_t, uint32_t);
		SDL_Surface* load_surface(const char *);
		void free_surface(SDL_Surface*);
		void draw_on(SDL_Surface*);
		void draw_from(SDL_Surface*);

		void draw_background();
		void draw_surface(SDL_Surface*, int x, int y);
		void draw_sprite(const Sprite& sp, int x, int y);
		void draw_rect(int x, int y, int w, int h);
		void draw_text(const char*, int x, int y);
		void draw_circle(int x, int y, int rad);
		void draw_arc(int x, int y, int radius, int rad1, int rad2);
		void draw_line(int x, int y, int x2, int y2);

		void text_size(const char* text, int *w, int *h);
		void surface_size(SDL_Surface* surface, int *w, int *h);

		void flip();
};

#endif
