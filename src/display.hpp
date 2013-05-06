#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include <SDL/SDL_ttf.h>

struct SDL_Surface;

struct Sprite;

class Display
{
	private:
		int size_x;
		int size_y;
		bool resizable;
		SDL_Surface * screen;
		SDL_Surface * atlas;
		TTF_Font* font;
		uint8_t r, g, b;
		int offx, offy;
		TTF_Font* fonts[128];

	public:
		void init();
		void show_cursor(bool);
		void set_resizable(bool);
		void resize(int w, int h);

		void set_color(int r, int g, int b);
		void set_offset(int, int);
		void set_font(const char*, int size);

		void draw_background();
		void draw_sprite(const Sprite& sp, int x, int y);
		void draw_rect(int x, int y, int w, int h);
		void draw_text(const char*, int x, int y);

		void text_size(const char* text, int *w, int *h);

		void flip();
};

#endif
