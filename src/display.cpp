#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include <iostream>

#include "display.hpp"
#include "drawable.hpp"
#include "ressource.hpp"


void Display::init()
{
	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();
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
}

void Display::show_cursor(bool b)
{
	SDL_ShowCursor(b);
}

void Display::flip()
{
	SDL_Flip(this->screen);
}

void Display::set_color(int r, int g, int b)
{
	r = r < 0 ? r = 0 : r;
	r = r > 255 ? r = 255 : r;
	g = g < 0 ? g = 0 : g;
	g = g > 255 ? g = 255 : g;
	b = b < 0 ? b = 0 : b;
	b = b > 255 ? b = 255 : b;
	this->r = r;
	this->g = g;
	this->b = b;
}

void Display::set_offset(int ox, int oy)
{
	this->offx = ox;
	this->offy = oy;
}

void Display::set_font(const char* name, int size)
{
	if (not fonts[size])
	{
		fonts[size] = TTF_OpenFont(name, size);
	}
	font = fonts[size];
}

void Display::draw_background()
{
	SDL_FillRect(this->screen, NULL, SDL_MapRGB(this->screen->format, r, g, b));
}

void Display::draw_sprite(const Sprite& sp, int x, int y)
{
	SDL_Rect dst, src;
	dst.x = x + offx;
	dst.y = y + offy;
	src.x = sp.x;
	src.y = sp.y;
	src.w = sp.w;
	src.h = sp.h;

	SDL_BlitSurface(atlas, &src, this->screen, &dst);
}

void Display::draw_rect(int x, int y, int w, int h)
{
	if (h <= 0 or w <= 0)
		return;

	SDL_Rect dst;
	dst.x = x + offx;
	dst.y = y + offy;
	dst.w = w;
	dst.h = h;
	SDL_FillRect(this->screen, &dst, SDL_MapRGB(this->screen->format, r, g, b));
}

void Display::draw_text(const char* text, int x, int y)
{
	if (not font)
		return;
	SDL_Rect dst;
	dst.x = x + offx;
	dst.y = y + offy;
	SDL_Color color = { r, g, b, 0xff };
	SDL_Surface *surf = TTF_RenderText_Solid(font, text, color);
	SDL_BlitSurface (surf, NULL, screen, &dst);
	SDL_FreeSurface(surf);
}

void Display::text_size(const char* text, int *w, int *h)
{
	if (not font)
		return;
	TTF_SizeText(font, text, w, h);
}

