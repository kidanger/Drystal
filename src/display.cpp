#include <SDL/SDL.h>
#include <iostream>

#include "display.hpp"
#include "drawable.hpp"
#include "ressource.hpp"

void Display::init()
{
	SDL_Init(SDL_INIT_VIDEO);
	atlas = get_image("data/image.png");
}

void Display::set_resizable(bool b)
{
	if (b != resizable)
	{
		resizable = b;
		resize(size_x, size_y);
	}
}

void Display::resize(int w, int h)
{
	size_x = w;
	size_y = h;
	screen = SDL_SetVideoMode(size_x, size_y, 32,
			SDL_HWSURFACE | (resizable ? SDL_VIDEORESIZE : 0));
	if (screen == nullptr)
	{
		std::cerr << "SDL_SetVideoMode error: " << SDL_GetError() << std::endl;
	}
}

void Display::show_cursor(bool b)
{
	SDL_ShowCursor(b);
}

void Display::draw_background()
{
	if (not this->screen)
	{
		fprintf(stderr, "Screen is not initialized\n");
		return;
	}
	SDL_FillRect(this->screen, NULL, SDL_MapRGB(this->screen->format, r, g, b));
}

void Display::flip()
{
	if (not this->screen)
	{
		fprintf(stderr, "Screen is not initialized\n");
		return;
	}
	SDL_Flip(this->screen);
}

void Display::set_background(int r, int g, int b)
{
	this->r = r;
	this->g = g;
	this->b = b;
}

void Display::draw(const Sprite& sp, int x, int y)
{
	if (not this->screen)
	{
		fprintf(stderr, "Screen is not initialized\n");
		return;
	}
	SDL_Rect dst, src;
	dst.x = x;
	dst.y = y;
	src.x = sp.x;
	src.y = sp.y;
	src.w = sp.w;
	src.h = sp.h;

	SDL_BlitSurface(atlas, &src, this->screen, &dst);
}
