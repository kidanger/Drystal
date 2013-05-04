#include <SDL/SDL.h>
#include <iostream>

#include "display.hpp"
#include "drawable.hpp"
#include "position.hpp"

void Display::init()
{
	SDL_Init(SDL_INIT_VIDEO);
	this->screen = SDL_SetVideoMode(this->size_x, this->size_y, 32, SDL_HWSURFACE);
	if (this->screen == nullptr)
	{
		std::cerr << "SDL_SetVideoMode error: " << SDL_GetError() << std::endl;
	}
}

void Display::draw_start()
{
	int r = 1;
	int g = 100;
	int b = 1;
	SDL_FillRect(this->screen, NULL, SDL_MapRGB(this->screen->format, r, g, b));
}

void Display::draw(const Drawable& drawable, const Position& position)
{
	drawable.draw(*this, position);
}

void Display::draw_end()
{
	SDL_Flip(this->screen);
}


void Display::blit(SDL_Surface* surf, const Position& p, const Bounds& bounds)
{
	SDL_Rect dst, src;
	dst.x = p.x;
	dst.y = p.y;
	src.x = bounds.x;
	src.y = bounds.y;
	src.w = bounds.w;
	src.h = bounds.h;

	SDL_BlitSurface(surf, &src, this->screen, &dst);
}
