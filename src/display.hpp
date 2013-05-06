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
		int r, g, b;
		int offx, offy;

	public:
		void init();
		void show_cursor(bool);
		void set_resizable(bool);
		void resize(int w, int h);

		void set_color(int r, int g, int b);
		void set_offset(int, int);

		void draw_background();
		void draw_sprite(const Sprite& sp, int x, int y);
		void draw_rect(int x, int y, int w, int h);
		SDL_Surface* create_text(const char* text);

		void flip();
};

#endif
