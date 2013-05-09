#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_gfxPrimitives.h>

#include <iostream>

#include "display.hpp"
#include "drawable.hpp"
#include "ressource.hpp"


void Display::init()
{
	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();
	atlas = get_image("data/image.png");
	alpha = 255;
	fill = true;
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

void Display::set_color(uint8_t r, uint8_t g, uint8_t b)
{
	// r = r < 0 ? r = 0 : r;
	// r = r > 255 ? r = 255 : r;
	// g = g < 0 ? g = 0 : g;
	// g = g > 255 ? g = 255 : g;
	// b = b < 0 ? b = 0 : b;
	// b = b > 255 ? b = 255 : b;
	this->r = r;
	this->g = g;
	this->b = b;
}

void Display::set_alpha(uint8_t a)
{
	this->alpha = a;
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

void Display::set_round(uint16_t round)
{
	this->round = round;
}

void Display::set_fill(bool fill)
{
	this->fill = fill;
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

	x += offx;
	y += offy;
	if (fill)
	{
		if (not round)
		{
			boxRGBA(this->screen, x, y, x+w, y+h, r, g, b, alpha);
		}
		else
		{
			roundedBoxRGBA(this->screen, x, y, x+w, y+h, round, r, g, b, alpha);
		}
	}
	else
	{
		if (not round)
		{
			rectangleRGBA(this->screen, x, y, x+w, y+h, r, g, b, alpha);
		}
		else
		{
			roundedRectangleRGBA(this->screen, x, y, x+w, y+h, round, r, g, b, alpha);
		}
	}
}

void Display::draw_text(const char* text, int x, int y)
{
	if (not font)
		return;
	if (not text or not text[0])
		return;
	SDL_Rect dst;
	dst.x = x + offx;
	dst.y = y + offy;
	SDL_Color color = { (uint8_t) r, (uint8_t) g, (uint8_t) b, (uint8_t) alpha };
	SDL_Surface *surf = TTF_RenderText_Solid(font, text, color);
	SDL_BlitSurface (surf, NULL, screen, &dst);
	SDL_FreeSurface(surf);
}

void Display::draw_circle(int x, int y, int rad)
{
	if (fill)
	{
		filledCircleRGBA(this->screen, x+offx, y+offy, rad, r, g, b, alpha);
	}
	else
	{
		circleRGBA(this->screen, x+offx, y+offy, rad, r, g, b, alpha);
	}
}

void Display::draw_arc(int x, int y, int radius, int rad1, int rad2)
{
	if (fill)
	{
		filledPieRGBA(this->screen, x+offx, y+offy, radius, rad1, rad2, r, g, b, alpha);
	}
	else
	{
		pieRGBA(this->screen, x+offx, y+offy, radius, rad1, rad2, r, g, b, alpha);
	}
}

void Display::text_size(const char* text, int *w, int *h)
{
	if (not font)
		return;
	if (not text or not text[0])
		*w = *h = 0;
	else
		TTF_SizeText(font, text, w, h);
}

