#ifndef DISPLAY_HPP
#define DISPLAY_HPP

struct SDL_Surface;

class Drawable;
struct Position;
struct Bounds;

class Display
{
	private:
		int size_x = 600;
		int size_y = 400;
		SDL_Surface * screen;

	public:
		void init();

		void draw_start();
		void draw(const Drawable&, const Position&);
		void draw_end();

		void blit(SDL_Surface*, const Position&, const Bounds&);
};

#endif
