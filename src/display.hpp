#ifndef DISPLAY_HPP
#define DISPLAY_HPP

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
		int r, g, b;

	public:
		void init();
		void show_cursor(bool);
		void set_resizable(bool);
		void resize(int w, int h);

		void set_background(int r, int g, int b);

		void draw_background();
		void draw(const Sprite& sp, int x, int y);
		void flip();
};

#endif
