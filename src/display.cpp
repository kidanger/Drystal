#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_gfxPrimitives.h>

#include <iostream>

#include "display.hpp"
#include "drawable.hpp"


void Display::init()
{
	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();
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
	SDL_Surface* old = screen;
	size_x = w;
	size_y = h;
	if (screen)
		SDL_FreeSurface(screen);
	screen = SDL_SetVideoMode(size_x, size_y, 32,
			SDL_HWSURFACE | (resizable ? SDL_VIDEORESIZE : 0));

	if (current == old)
		current = screen;
}

void Display::show_cursor(bool b)
{
	SDL_ShowCursor(b);
}

void Display::flip()
{
	SDL_Flip(this->screen);
}

SDL_Surface* Display::load_surface(const char * filename)
{
	SDL_Surface *surf = IMG_Load(filename);
	return surf;
}

SDL_Surface* Display::new_surface(uint32_t w, uint32_t h)
{
	int rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif
	SDL_Surface* surf = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCALPHA, w, h, 32,
			rmask, gmask, bmask, amask);
	return surf;
}
void Display::free_surface(SDL_Surface* surface)
{
	SDL_FreeSurface(surface);
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

void Display::set_alpha(uint8_t a)
{
	this->alpha = a;
}

void Display::push_offset(int ox, int oy)
{
	offset_current += 1;
	offsetsx[offset_current] = ox;
	offsetsy[offset_current] = oy;
	this->offx = ox;
	this->offy = oy;
}
void Display::pop_offset()
{
	offset_current -= 1;
	this->offx = offsetsx[offset_current];
	this->offy = offsetsy[offset_current];
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

void Display::draw_from(SDL_Surface* surf)
{
	this->current_from = surf;
}
void Display::draw_on(SDL_Surface* surf)
{
	this->current = surf;
}

SDL_Surface* Display::get_screen()
{
	return this->screen;
}

void Display::draw_background()
{
	boxRGBA(current, 0, 0, size_x, size_y, r, g, b, alpha);
}

void Display::draw_surface(SDL_Surface* from, int x, int y)
{
	SDL_Rect dst, src;
	dst.x = x + offx;
	dst.y = y + offy;
	src.x = 0;
	src.y = 0;
	src.w = from->w;
	src.h = from->h;

	SDL_BlitSurface(from, &src, current, &dst);
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

	SDL_BlitSurface(current_from, &src, current, &dst);
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
			boxRGBA(current, x, y, x+w, y+h, r, g, b, alpha);
		else
			roundedBoxRGBA(current, x, y, x+w, y+h, round, r, g, b, alpha);
	}
	else
	{
		if (not round)
			rectangleRGBA(current, x, y, x+w, y+h, r, g, b, alpha);
		else
			roundedRectangleRGBA(current, x, y, x+w, y+h, round, r, g, b, alpha);
	}
}

void Display::draw_text(const char* text, int x, int y)
{
	if (not font)
	{
#ifndef EMSCRIPTEN
		printf("No font loaded to draw_text\n");
#endif
		return;
	}
	if (not text or not text[0])
		return;
	SDL_Rect dst;
	dst.x = x + offx;
	dst.y = y + offy;
	SDL_Color color = { (uint8_t) r, (uint8_t) g, (uint8_t) b, (uint8_t) alpha };
	SDL_Surface *surf = TTF_RenderText_Solid(font, text, color);
	SDL_BlitSurface (surf, NULL, current, &dst);
	SDL_FreeSurface(surf);
}

void Display::draw_circle(int x, int y, int rad)
{
	if (fill)
		filledCircleRGBA(current, x+offx, y+offy, rad, r, g, b, alpha);
	else
		circleRGBA(current, x+offx, y+offy, rad, r, g, b, alpha);
}

void Display::draw_arc(int x, int y, int radius, int rad1, int rad2)
{
	if (fill)
		filledPieRGBA(current, x+offx, y+offy, radius, rad1, rad2, r, g, b, alpha);
	else
		pieRGBA(current, x+offx, y+offy, radius, rad1, rad2, r, g, b, alpha);
}

void Display::draw_line(int x, int y, int x2, int y2)
{
	x += offx;
	x2 += offx;
	y += offy;
	y2 += offy;
	if (x < 0 and x2 > 0)
		x = 0;
	if (x >= 0 and x2 > current->w)
		x2 = current->w - 1;
	if (y < 0 and y2 > 0)
		y = 0;
	if (y >= 0 and y2 > current->h)
		y2 = current->h - 1;
	aalineRGBA(current, x, y, x2, y2, r, g, b, alpha);
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

void Display::surface_size(SDL_Surface* surface, int *w, int *h)
{
	*w = surface->w;
	*h = surface->h;
}

