#ifndef EMSCRIPTEN
#include <GLES2/gl2.h>
#endif

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#include <iostream>
#include <cassert>
#include <cmath>

#include "display.hpp"
#include "drawable.hpp"


void Display::init()
{
	int err = SDL_Init(SDL_INIT_EVERYTHING);
	err |= TTF_Init();
	assert(not err);
	alpha = 1;
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
	screen->fbo = 0; // back buffer

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if (current == old)
		draw_on(screen);

	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
}

void Display::show_cursor(bool b)
{
	SDL_ShowCursor(b);
}

void Display::flip()
{
	glBindFramebuffer(GL_FRAMEBUFFER, screen->fbo);
	glFlush();
	SDL_GL_SwapBuffers();
	glBindFramebuffer(GL_FRAMEBUFFER, current->fbo);
}

Surface* Display::surface_from_sdl(SDL_Surface* surf)
{
	assert(surf);

	int ow = surf->w;
	int oh = surf->h;
	int w = pow(2, ceil(log(surf->w)/log(2)));
	int h = pow(2, ceil(log(surf->h)/log(2)));

	bool should_free = false;
	if (w != surf->w or h != surf->h or surf->format->BytesPerPixel != 4)
	{
		// resize and convert
		SDL_Surface* resized = SDL_CreateRGBSurface(SDL_HWSURFACE, w, h, 32,
				surf->format->Rmask, surf->format->Gmask, surf->format->Bmask, surf->format->Amask);
#ifdef EMSCRIPTEN
		SDL_Surface* newSurface = resized;
#else
		SDL_Surface* newSurface = SDL_DisplayFormatAlpha(resized);
		SDL_FreeSurface(resized);
		// fill and copy old surface into the new one
		SDL_FillRect(newSurface, 0, 0);
#endif

		SDL_BlitSurface(surf, NULL, newSurface, NULL);

		surf = newSurface;
		should_free = true;
	}
	assert(surf);
	assert(surf->format->BytesPerPixel == 4);
	assert(surf->w == w);
	assert(surf->h == h);


	GLenum texture_format;
	if (surf->format->Rmask == 0x000000ff)
		texture_format = GL_RGBA;
	else
		texture_format = GL_BGRA;

	// gen texture
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0,
#ifdef EMSCRIPTEN
			GL_RGBA,
#else
			4,
#endif
			surf->w, surf->h, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, surf->pixels);
	//GLenum err = glGetError();
	//printf("%d %d\n", err, GL_INVALID_OPERATION);

	// gen framebuffer object
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

	GLenum status;
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	assert(status == GL_FRAMEBUFFER_COMPLETE);
	if (should_free)
		SDL_FreeSurface(surf);

	Surface* surface = new Surface;
	surface->tex = tex;
	surface->texw = surf->w;
	surface->texh = surf->h;
	surface->w = ow;
	surface->h = oh;
	surface->resizew = ow;
	surface->resizeh = oh;
	surface->fbo = fbo;
	surface->angle = 0;
	glBindFramebuffer(GL_FRAMEBUFFER, current->fbo);
	glBindTexture(GL_TEXTURE_2D, current_from ? current_from->tex : 0);
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
	SDL_Surface* surf = SDL_CreateRGBSurface(SDL_HWSURFACE, w, h, 32,
			rmask, gmask, bmask, amask);
	assert(surf);
	Surface* surface = surface_from_sdl(surf);
	SDL_FreeSurface(surf);
	return surface;
}
void Display::free_surface(Surface* surface)
{
	assert(surface);
	if (surface == current_from)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		current_from = NULL;
	}
	if (surface == current)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, screen->fbo);
		current = NULL;
	}
	glDeleteTextures(1, &surface->tex);
	glDeleteFramebuffers(1, &surface->fbo);
}

void Display::set_color(int r, int g, int b)
{
	//r = r < 0 ? r = 0 : r;
	//r = r > 255 ? r = 255 : r;
	//g = g < 0 ? g = 0 : g;
	//g = g > 255 ? g = 255 : g;
	//b = b < 0 ? b = 0 : b;
	//b = b > 255 ? b = 255 : b;
	this->r = r / 255.;
	this->g = g / 255.;
	this->b = b / 255.;
	glColor4f(this->r, this->g, this->b, alpha);
}

void Display::set_alpha(uint8_t a)
{
	this->alpha = a / 255.;
	glColor4f(this->r, this->g, this->b, this->alpha);
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
	glBindTexture(GL_TEXTURE_2D, current_from->tex);
}
void Display::draw_on(Surface* surf)
{
	assert(surf);
	this->current = surf;
	glBindFramebuffer(GL_FRAMEBUFFER, current->fbo);

	int w = surf->w;
	int h = surf->h;
	glViewport(0, 0, w, h);
	glLoadIdentity();
	if (surf == screen)
		glOrtho(0, w, h, 0, -1, 1);
	else
		glOrtho(0, w, 0, h, -1, 1);
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

	const GLfloat vertices[] = {
		-w / 2.f, -h / 2.f,
		-w / 2.f, h / 2.f,
		w / 2.f, -h / 2.f,
		w / 2.f, h / 2.f,
	};
	const GLfloat coords[] = {
		0, 0,
		0, (float)from->h / from->texh,
		(float)from->w / from->texw, 0,
		(float)from->w / from->texw, (float)from->h / from->texh,
	};

	glBindTexture(GL_TEXTURE_2D, from->tex);
	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glPushMatrix();
	glTranslated(x + w / 2, y + h / 2, 0);
	glRotatef(from->angle, 0, 0, 1);
	glScalef((float) from->resizew / from->w,
			(float) from->resizeh / from->h,
			1);

	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, coords);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glPopMatrix();

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, current_from ? current_from->tex : 0);
}

void Display::draw_sprite(const Sprite& sp, int x, int y)
{
	assert(current_from);
	assert(current);

	float dx = (float) sp.x / current_from->resizew * (float) current_from->w / current_from->texw;
	float dy = (float) sp.y / current_from->resizeh * (float) current_from->h / current_from->texh;
	float dw = (float) sp.w / current_from->resizew * (float) current_from->w / current_from->texw;
	float dh = (float) sp.h / current_from->resizeh * (float) current_from->h / current_from->texh;

	x += offx;
	y += offy;

	const GLfloat vertices[] = {
		-sp.w / 2.f, -sp.h / 2.f,
		-sp.w / 2.f, sp.h / 2.f,
		sp.w / 2.f, -sp.h / 2.f,
		sp.w / 2.f, sp.h / 2.f,
	};
	const GLfloat coords[] = {
		dx, dy,
		dx, dy + dh,
		dx + dw, dy,
		dx + dw, dy + dh,
	};

	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glPushMatrix();
	glTranslated(x + sp.w / 2, y + sp.h / 2, 0);
	glRotatef(current_from->angle, 0, 0, 1);

	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, coords);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glPopMatrix();

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_TEXTURE_2D);
}

void Display::draw_rect(int x, int y, int w, int h)
{
	assert(current);
	if (h <= 0 or w <= 0)
		return;

	x += offx;
	y += offy;

	const GLfloat vertices[] = {
		x+.0f, y+.0f,
		x+.0f, y + h+.0f,
		x+.0f + w, y + h+.0f,
		x+.0f + w, y+.0f,
		x+.0f, y+.0f,
	};
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glDrawArrays(fill ? GL_TRIANGLE_FAN : GL_LINE_LOOP, 0, 5);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void Display::draw_circle(int cx, int cy, int rad)
{
	cx += offx;
	cy += offy;
	// http://slabode.exofire.net/circle_draw.shtml
	assert(current);
	int num_segments = 10 * sqrtf(rad);
	float theta = 2 * 3.1415926 / float(num_segments);
	float c = cosf(theta); //precalculate the sine and cosine
	float s = sinf(theta);
	float t;

	float x = rad;//we start at angle = 0
	float y = 0;

	GLfloat vertices[2 * num_segments];

	for(int ii = 0; ii < num_segments; ii++)
	{
		vertices[ii*2] = x + cx;
		vertices[ii*2 + 1] = y + cy;

		//apply the rotation matrix
		t = x;
		x = c * x - s * y;
		y = s * t + c * y;
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glDrawArrays(fill ? GL_TRIANGLE_FAN : GL_LINE_LOOP, 0, num_segments);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void Display::draw_arc(int cx, int cy, int radius, int rad1, int rad2)
{
	assert(current);
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

	SDL_Surface *surf = TTF_RenderText_Solid(font, text, color);
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

