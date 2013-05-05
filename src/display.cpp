#include <SDL/SDL.h>

#ifndef EMSCRIPTEN
#include <iostream>
#endif

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
#ifndef EMSCRIPTEN
	if (screen == nullptr)
	{
		std::cerr << "SDL_SetVideoMode error: " << SDL_GetError() << std::endl;
	}
#endif
}

void Display::show_cursor(bool b)
{
	SDL_ShowCursor(b);
}

void Display::draw_background()
{
#ifndef EMSCRIPTEN
	if (not this->screen)
	{
		std::cerr << "Screen is not initialized" << std::endl;
		return;
	}
#endif
	SDL_FillRect(this->screen, NULL, SDL_MapRGB(this->screen->format, r, g, b));
}

void Display::flip()
{
#ifndef EMSCRIPTEN
	if (not this->screen)
	{
		std::cerr << "Screen is not initialized" << std::endl;
		return;
	}
#endif
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
#ifndef EMSCRIPTEN
	if (not this->screen)
	{
		std::cerr << "Screen is not initialized" << std::endl;
		return;
	}
#endif
	SDL_Rect dst, src;
	dst.x = x;
	dst.y = y;
	src.x = sp.x;
	src.y = sp.y;
	src.w = sp.w;
	src.h = sp.h;

	SDL_BlitSurface(atlas, &src, this->screen, &dst);
}

