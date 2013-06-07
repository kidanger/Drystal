#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_opengl.h>

#include <iostream>
#include <cassert>

#include "display.hpp"
#include "drawable.hpp"


void Display::init()
{
	int err = SDL_Init(SDL_INIT_EVERYTHING);
	err |= TTF_Init();
	assert(not err);
	alpha = 255;
	fill = true;
	resize(680, 680);
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
	Surface* old = screen;
	size_x = w;
	size_y = h;
	if (screen)
		SDL_FreeSurface(sdl_screen);
	sdl_screen = SDL_SetVideoMode(size_x, size_y, 32,
			SDL_OPENGL| (resizable ? SDL_VIDEORESIZE : 0));
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	assert(sdl_screen);

	if (screen)
		free(screen);
	screen = new Surface;
	screen->w = w;
	screen->h = h;
	// gen tex

	if (current == old)
		current = screen;

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w, h, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Display::show_cursor(bool b)
{
	SDL_ShowCursor(b);
}

void Display::flip()
{
	glFlush();
	SDL_GL_SwapBuffers();
}

static Surface* surface_from_sdl(SDL_Surface* surf)
{
	//assert((surf->w & (surf->w - 1)) == 0);
	//assert((surf->h & (surf->h - 1)) == 0);

	GLenum texture_format = GL_RGBA;
	GLint colors = surf->format->BytesPerPixel;
	if (colors == 4)     // contains an alpha channel
	{
		if (surf->format->Rmask == 0x000000ff)
			texture_format = GL_RGBA;
		else
			texture_format = GL_BGRA;
	} else if (colors == 3)     // no alpha channel
	{
		if (surf->format->Rmask == 0x000000ff)
			texture_format = GL_RGB;
		else
			texture_format = GL_BGR;
	} else {
		assert(false);
	}

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, colors, surf->w, surf->h, 0, texture_format, GL_UNSIGNED_BYTE, surf->pixels);

	Surface* surface = new Surface;
	surface->tex = tex;
	surface->w = surf->w;
	surface->h = surf->h;
	surface->resizew = surf->w;
	surface->resizeh = surf->h;
	return surface;
}

Surface* Display::load_surface(const char * filename)
{
	assert(filename);
	SDL_Surface *surf = IMG_Load(filename);
	assert(surf);

	Surface* surface = surface_from_sdl(surf);
	SDL_FreeSurface(surf);
	return surface;
}

Surface* Display::new_surface(uint32_t w, uint32_t h)
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
	assert(surf);
	Surface* surface = surface_from_sdl(surf);
	SDL_FreeSurface(surf);
	return surface;
}
void Display::free_surface(Surface* surface)
{
	assert(surface);
	glDeleteTextures(1, &surface->tex);
}

void Display::set_color(int r, int g, int b)
{
	r = r < 0 ? r = 0 : r;
	r = r > 255 ? r = 255 : r;
	g = g < 0 ? g = 0 : g;
	g = g > 255 ? g = 255 : g;
	b = b < 0 ? b = 0 : b;
	b = b > 255 ? b = 255 : b;
	this->r = r / 255.;
	this->g = g / 255.;
	this->b = b / 255.;
}

void Display::set_alpha(uint8_t a)
{
	this->alpha = a;
}

void Display::push_offset(int ox, int oy)
{
	assert(offset_current + 1 < MAX_OFFSETS);
	offset_current += 1;
	offsetsx[offset_current] = ox;
	offsetsy[offset_current] = oy;
	this->offx = ox;
	this->offy = oy;
}
void Display::pop_offset()
{
	assert(offset_current > 0);
	offset_current -= 1;
	this->offx = offsetsx[offset_current];
	this->offy = offsetsy[offset_current];
}

void Display::set_font(const char* name, int size)
{
	assert(name);
	assert(size > 0);
	if (not fonts[size])
	{
		fonts[size] = TTF_OpenFont(name, size);
	}
	font = fonts[size];
	assert(font);
}

void Display::set_round(uint16_t round)
{
	assert(round >= 0);
	this->round = round;
}

void Display::set_fill(bool fill)
{
	this->fill = fill;
}

void Display::rotate_surface(Surface* surf, double angle)
{
	assert(surf);
	surf->angle = angle;
}
void Display::resize_surface(Surface* surf, int w, int h)
{
	assert(surf);
	assert(w);
	assert(h);
	surf->resizew = w;
	surf->resizeh = h;
}

void Display::draw_from(Surface* surf)
{
	assert(surf);
	this->current_from = surf;
}
void Display::draw_on(Surface* surf)
{
	assert(surf);
	this->current = surf;
}

Surface* Display::get_screen()
{
	return this->screen;
}

void Display::draw_background()
{
	glClearColor(r, g, b, alpha);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Display::draw_surface(Surface* from, int x, int y)
{
	assert(current);
	assert(from);

	x += offx;
	y += offy;
	int w = from->w;
	int h = from->h;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, from->tex);

	glPushMatrix();
	glTranslated(x + w / 2, y + h / 2, 0);
	glRotatef(from->angle, 0, 0, 1);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2d(-w/2, -h/2);
	glTexCoord2f(0, 1);
	glVertex2d(-w/2, h/2);
	glTexCoord2f(1, 1);
	glVertex2d(w/2, h/2);
	glTexCoord2f(1, 0);
	glVertex2d(w/2, -h/2);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

void Display::draw_sprite(const Sprite& sp, int x, int y)
{
	assert(current_from);
	assert(current);

	float dx = (float) sp.x / current_from->resizew;
	float dy = (float) sp.y / current_from->resizeh;
	float dw = (float) sp.w / current_from->resizew;
	float dh = (float) sp.h / current_from->resizeh;

	x += offx;
	y += offy;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, current_from->tex);

	glPushMatrix();
	glTranslated(x + sp.w / 2, y + sp.h / 2, 0);
	glRotatef(current_from->angle, 0, 0, 1);

	glBegin(GL_QUADS);
	glTexCoord2f(dx, dy);
	glVertex2d(- sp.w / 2, - sp.h / 2);
	glTexCoord2f(dx, dy + dw);
	glVertex2d(- sp.w / 2, sp.h / 2);
	glTexCoord2d(dx + dw, dy + dh);
	glVertex2d(sp.w / 2, sp.h / 2);
	glTexCoord2d(dx + dw, dy);
	glVertex2d(sp.w / 2, - sp.h / 2);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

void Display::draw_rect(int x, int y, int w, int h)
{
	//assert(current);
	if (h <= 0 or w <= 0)
		return;

	x += offx;
	y += offy;
	glColor4f(r, g, b, alpha);

	glBegin(fill ? GL_QUADS : GL_LINE_LOOP);
	glVertex2d(x, y);
	glVertex2d(x, y+h);
	glVertex2d(x+w, y+h);
	glVertex2d(x+w, y);
	glEnd();
}

void Display::draw_circle(int x, int y, int rad)
{
	assert(current);
	x=y=rad;
}

void Display::draw_arc(int x, int y, int radius, int rad1, int rad2)
{
	assert(current);
	x = y = radius = rad1 = rad2;
}

void Display::draw_line(int x, int y, int x2, int y2)
{
	assert(current);
	x += offx;
	x2 += offx;
	y += offy;
	y2 += offy;
	if (x < 0 and x2 > 0)
		x = 0;
	if (x >= 0 and x2 > (int)current->w)
		x2 = current->w - 1;
	if (y < 0 and y2 > 0)
		y = 0;
	if (y >= 0 and y2 > (int)current->h)
		y2 = current->h - 1;
	glColor4f(r, g, b, alpha);
	glBegin(GL_LINES);
	glVertex2i(x, y);
	glVertex2i(x2, y2);
	glEnd();
}

Surface* Display::text_surface(const char* text)
{
	assert(current);
	assert(font);

	SDL_Color color = { (uint8_t) (r*255), (uint8_t) (g*255), (uint8_t) (b*255), (uint8_t) (alpha*255) };

	SDL_Surface *surf = TTF_RenderText_Blended(font, text, color);
	assert(surf);
	Surface* surface = surface_from_sdl(surf);
	SDL_FreeSurface(surf);
	return surface;
}

void Display::text_size(const char* text, int *w, int *h)
{
	assert(w);
	assert(h);
	if (not font)
		return;
	if (not text or not text[0])
		*w = *h = 0;
	else
		TTF_SizeText(font, text, w, h);
}

void Display::surface_size(Surface* surface, int *w, int *h)
{
	assert(surface);
	assert(w);
	assert(h);
	*w = surface->w;
	*h = surface->h;
}

