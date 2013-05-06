#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include <iostream>

#include "display.hpp"
#include "drawable.hpp"
#include "ressource.hpp"

//#include <SDL_oglblit.h>

void Display::init()
{
	//OGL_Init(800, 600, 0, 0);
	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();
	atlas = get_image("data/image.png");
	//atlas = OGL_FromSurfaceFree(get_image("data/image.png"));
	font = TTF_OpenFont("arial.ttf", 26);
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
	//OGL_Flip();
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
	// OGL_Blit(atlas, &src, x, y, 1, 1, 1);
}

void Display::draw_rect(int x, int y, int w, int h)
{
	if (h <= 0 or w <= 0)
		return;

	SDL_Color color = { 0xff, 0x99, 0x00, 0xff };
	static SDL_Surface *text = TTF_RenderText_Solid(font, "hello world", color);
	SDL_BlitSurface (text, NULL, screen, NULL);
	//SDL_FreeSurface(text);

	SDL_Rect dst;
	dst.x = x + offx;
	dst.y = y + offy;
	dst.w = w;
	dst.h = h;
	SDL_FillRect(this->screen, &dst, SDL_MapRGB(this->screen->format, r, g, b));
}

