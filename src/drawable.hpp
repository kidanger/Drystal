#ifndef SPRITE_HPP
#define SPRITE_HPP

#include <string>

struct SDL_Surface;

class Display;
struct Position;

struct Bounds
{
	int x, y;
	int w, h;
};

enum SpriteEnum
{
	HERO
};

class Drawable
{
	public:
		virtual void draw(Display&, const Position&) const = 0;
		virtual const Position& get_position() = 0;
};

class Sprite : public Drawable
{
	private:
		SDL_Surface* surface;
		Bounds bounds;
		const Position* position;

	public:
		Sprite(const std::string&);

		void draw(Display&, const Position&) const;
		const Position& get_position() { return *position; }
		void set_position(const Position* pos) { position = pos; }
		void set_bounds(int x, int y, int w, int h) { bounds.x = x; bounds.y = y; bounds.w = w; bounds.h = h; };
};

#endif
