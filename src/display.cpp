#ifndef EMSCRIPTEN
#include <GLES2/gl2.h>
#endif

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#include <iostream>
#include <cassert>
#include <cmath>

#include "log.hpp"
#include "display.hpp"

const char* DEFAULT_VERTEX_SHADER = R"(
#version 100
#ifdef GL_ES
precision highp float;
#endif

attribute vec2 position;	// position of the vertice (0,0) is topleft, frameSize is bottomright
attribute vec4 color;		// color of the vertice
attribute vec2 texCoord;	// texture coordinates

varying vec4 fColor;
varying vec2 fTexCoord;

void main()
{
	gl_Position = vec4(position, 0.0, 1.0);
	fColor = color;
	fTexCoord = texCoord;
}
)";

const char* DEFAULT_FRAGMENT_SHADER = R"(
#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D tex;
uniform int useTex; // TODO: replace it with two fragment shaders

varying vec4 fColor;
varying vec2 fTexCoord;

void main()
{
	vec4 color;
	if (useTex == 1) {
		vec4 texval = texture2D(tex, vec2(fTexCoord));
		if (texval.a == 0.0)
			discard; // don't process transparent pixels
		color.rgb = mix(texval.rgb, fColor.rgb, vec3(1.)-fColor.rgb);
		color.a = fColor.a;
	} else {
		color = fColor;
	}
	gl_FragColor = color;
}
)";

Display::Display()
	: resizable(false),
	  sdl_screen(NULL),
	  screen(NULL),
	  default_shader(NULL),
	  current(NULL),
	  current_from(NULL),
	  font(NULL),
	  r(1),g(1),b(1),alpha(1)

{
	DEBUG("");
	int err = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	err |= TTF_Init();
	assert(not err);

	for (size_t i = 0; i < sizeof(fonts)/sizeof(fonts[0]); i++)
		fonts[i] = NULL;
}

/**
 * Screen
 */

void Display::set_resizable(bool b)
{
	if (b != resizable) {
		resizable = b;
		if (screen)
			resize(size_x, size_y);
	}
}

void Display::resize(int w, int h)
{
	w = w > 0 ? w : 1;
	h = h > 0 ? h : 1;
	DEBUG("");
	Surface* old = screen;
	size_x = w;
	size_y = h;
	if (screen)
		SDL_FreeSurface(sdl_screen);
	sdl_screen = SDL_SetVideoMode(size_x, size_y, 32,
			SDL_OPENGL| (resizable ? SDL_VIDEORESIZE : 0));
#ifndef EMSCRIPTEN
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#endif
	assert(sdl_screen);

	if (screen)
		delete screen;
	screen = new Surface;
	screen->w = sdl_screen->w;
	screen->h = sdl_screen->h;
	screen->texw = sdl_screen->w;
	screen->texh = sdl_screen->h;
	screen->fbo = 0; // back buffer

	if (current == old) {
		current = nullptr; // force update
		draw_on(screen);
	}

	// regenerate shader (lost with the GL context)
	if (default_shader)
		free_shader(default_shader);
	default_shader = create_default_shader();
	use_shader(default_shader);

	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	DEBUG("end");
}

void Display::show_cursor(bool b)
{
	SDL_ShowCursor(b);
}

void Display::draw_background() const
{
	glClearColor(r, g, b, alpha);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Display::flip()
{
	DEBUG("");
	buffer.assert_empty();
	glBindFramebuffer(GL_FRAMEBUFFER, screen->fbo);
	glFlush();
	SDL_GL_SwapBuffers();
	glBindFramebuffer(GL_FRAMEBUFFER, current->fbo);
	DEBUG("end");
}

Surface* Display::get_screen() const
{
	return screen;
}

/**
 * State
 */

void Display::set_color(int r, int g, int b)
{
	this->r = r / 255.;
	this->g = g / 255.;
	this->b = b / 255.;
}

void Display::set_alpha(uint8_t a)
{
	this->alpha = a / 255.;
}

void Display::draw_from(Surface* surf)
{
	DEBUG("");
	assert(surf);
	if (current_from != surf) {
		buffer.assert_empty();
		this->current_from = surf;
		glBindTexture(GL_TEXTURE_2D, current_from->tex);
	}
}

void Display::draw_on(Surface* surf)
{
	DEBUG("");
	assert(surf);
	if (current != surf) {
		buffer.assert_empty();
		this->current = surf;
		glBindFramebuffer(GL_FRAMEBUFFER, current->fbo);

		int w = surf->w;
		int h = surf->h;
		glViewport(0, 0, w, h);
	}
}


/**
 * Surface
 */

Surface* Display::surface_from_sdl(SDL_Surface* surf) const
{
	DEBUG("");
	assert(surf);

	int ow = surf->w;
	int oh = surf->h;
	int w = pow(2, ceil(log(surf->w)/log(2)));
	int h = pow(2, ceil(log(surf->h)/log(2)));

	bool should_free = false;
	if (w != surf->w or h != surf->h or surf->format->BytesPerPixel != 4)
	{
		DEBUG("convert");
		// resize and convert
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
		SDL_Surface* newSurface = SDL_CreateRGBSurface(SDL_HWSURFACE, w, h, 32,
				rmask, gmask, bmask, amask);
		SDL_SetAlpha(surf, 0, 0);
		SDL_BlitSurface(surf, NULL, newSurface, NULL);

		surf = newSurface;
		should_free = true;
	}
	assert(surf);
	assert(surf->format->BytesPerPixel == 4);
	assert(surf->w == w);
	assert(surf->h == h);

	// gen texture
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexImage2D(GL_TEXTURE_2D, 0,
#ifdef EMSCRIPTEN
			GL_RGBA,
#else
			4,
#endif
			surf->w, surf->h, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, surf->pixels);
	GLDEBUG();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// gen framebuffer object
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
	GLDEBUG();

	GLenum status;
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	assert(status == GL_FRAMEBUFFER_COMPLETE);
	if (should_free)
		SDL_FreeSurface(surf);

	Surface* surface = new Surface;
	surface->tex = tex;
	surface->w = ow;
	surface->h = oh;
	surface->texw = surf->w;
	surface->texh = surf->h;
	surface->fbo = fbo;
	glBindFramebuffer(GL_FRAMEBUFFER, current->fbo);
	glBindTexture(GL_TEXTURE_2D, current_from ? current_from->tex : 0);
	DEBUG("end");
	return surface;
}

Surface* Display::load_surface(const char * filename) const
{
	assert(filename);
	SDL_Surface *surf = IMG_Load(filename);

	if (not surf)
		return nullptr;

	Surface* surface = surface_from_sdl(surf);
	SDL_FreeSurface(surf);
	return surface;
}

Surface* Display::new_surface(uint32_t w, uint32_t h) const
{
	assert(w > 0);
	assert(h > 0);
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
	if (surface == current_from) {
		buffer.assert_empty(); // TODO: check if IMAGE_BUFFER
		glBindTexture(GL_TEXTURE_2D, 0);
		current_from = NULL;
	}
	if (surface == current) {
		glBindFramebuffer(GL_FRAMEBUFFER, screen->fbo);
		current = NULL;
	}
	glDeleteTextures(1, &surface->tex);
	glDeleteFramebuffers(1, &surface->fbo);
	delete surface;
}

void Display::surface_size(Surface* surface, int *w, int *h)
{
	assert(surface);
	assert(w);
	assert(h);
	*w = surface->w;
	*h = surface->h;
}

/**
 * Primitive drawing
 */
void Display::convert_coords(int x, int y, float *dx, float *dy)
{
	*dx = (2.0 * x / current->w) - 1;
	*dy = (2.0 * y / current->h) - 1;
	if (current == screen)
		*dy *= -1.0;
}
void Display::convert_texcoords(int x, int y, float *dx, float *dy)
{
	*dx = (float) x / current_from->texw;
	*dy = (float) y / current_from->texh;
}

void Display::draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3)
{
	DEBUG("");
	float xx1, xx2, xx3;
	float yy1, yy2, yy3;
	convert_coords(x1, y1, &xx1, &yy1);
	convert_coords(x2, y2, &xx2, &yy2);
	convert_coords(x3, y3, &xx3, &yy3);

	buffer.assert_type(TRIANGLE_BUFFER);
	buffer.push_vertex(xx1, yy1);
	buffer.push_vertex(xx2, yy2);
	buffer.push_vertex(xx3, yy3);
	for (int i = 0; i < 3; i++)
		buffer.push_color(r, g, b, alpha);
}

void Display::draw_line(int x1, int y1, int x2, int y2)
{
	DEBUG("");
	float xx1, xx2;
	float yy1, yy2;
	convert_coords(x1, y1, &xx1, &yy1);
	convert_coords(x2, y2, &xx2, &yy2);

	buffer.assert_type(LINE_BUFFER);
	buffer.push_vertex(xx1, yy1);
	buffer.push_vertex(xx2, yy2);
	for (int i = 0; i < 2; i++)
		buffer.push_color(r, g, b, alpha);
}

void Display::draw_surface(int xi1, int yi1, int xi2, int yi2, int xi3, int yi3, int xi4, int yi4,
		int xo1, int yo1, int xo2, int yo2, int xo3, int yo3, int xo4, int yo4)
{
	assert(current_from);
	DEBUG("");
	float xxi1, xxi2, xxi3, xxi4;
	float yyi1, yyi2, yyi3, yyi4;
	convert_texcoords(xi1, yi1, &xxi1, &yyi1);
	convert_texcoords(xi2, yi2, &xxi2, &yyi2);
	convert_texcoords(xi3, yi3, &xxi3, &yyi3);
	convert_texcoords(xi4, yi4, &xxi4, &yyi4);

	float xxo1, xxo2, xxo3, xxo4;
	float yyo1, yyo2, yyo3, yyo4;
	convert_coords(xo1, yo1, &xxo1, &yyo1);
	convert_coords(xo2, yo2, &xxo2, &yyo2);
	convert_coords(xo3, yo3, &xxo3, &yyo3);
	convert_coords(xo4, yo4, &xxo4, &yyo4);

	buffer.assert_type(IMAGE_BUFFER);
	buffer.push_texCoord(xxi1, yyi1);
	buffer.push_texCoord(xxi3, yyi3);
	buffer.push_texCoord(xxi4, yyi4);

	buffer.push_texCoord(xxi1, yyi1);
	buffer.push_texCoord(xxi2, yyi2);
	buffer.push_texCoord(xxi3, yyi3);

	buffer.push_vertex(xxo1, yyo1);
	buffer.push_vertex(xxo3, yyo3);
	buffer.push_vertex(xxo4, yyo4);

	buffer.push_vertex(xxo1, yyo1);
	buffer.push_vertex(xxo2, yyo2);
	buffer.push_vertex(xxo3, yyo3);

	for (int i = 0; i < 6; i++)
		buffer.push_color(r, g, b, alpha);
}

/**
 * Text
 */

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

Surface* Display::text_surface(const char* text) const
{
	assert(text);
	assert(font);

	SDL_Color color = { (uint8_t) (r*255), (uint8_t) (g*255), (uint8_t) (b*255), (uint8_t) (alpha*255) };

	SDL_Surface *surf = TTF_RenderText_Blended(font, text, color);
	Surface* surface = surface_from_sdl(surf);
	SDL_FreeSurface(surf);
	return surface;
}

void Display::text_size(const char* text, int *w, int *h) const
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


/**
 * Shader
 */
static void printLog(GLuint obj, const char* prefix)
{
	int infologLength = 0;
	int maxLength;

	if(glIsShader(obj))
		glGetShaderiv(obj,GL_INFO_LOG_LENGTH,&maxLength);
	else
		glGetProgramiv(obj,GL_INFO_LOG_LENGTH,&maxLength);

	char infoLog[maxLength];

	if (glIsShader(obj))
		glGetShaderInfoLog(obj, maxLength, &infologLength, infoLog);
	else
		glGetProgramInfoLog(obj, maxLength, &infologLength, infoLog);

	if (infologLength > 0)
		printf("Err %s: %s\n", prefix, infoLog);
}

Shader* Display::create_default_shader()
{
	const char* strvert = DEFAULT_VERTEX_SHADER;
	const char* strfrag = DEFAULT_FRAGMENT_SHADER;
	Shader* shader = new_shader(strvert, strfrag);
	return shader;
}

Shader* Display::new_shader(const char* strvert, const char* strfrag)
{
	GLuint vert = 0;
	GLuint frag = 0;
	GLuint prog;

	if (not strvert or not *strvert) {
		strvert = DEFAULT_VERTEX_SHADER;
	}
	if (not strfrag or not *strfrag) {
		strfrag = DEFAULT_FRAGMENT_SHADER;
	}

	vert = glCreateShader(GL_VERTEX_SHADER);
	assert(vert);
	glShaderSource(vert, 1, &strvert, NULL);
	glCompileShader(vert);

	frag = glCreateShader(GL_FRAGMENT_SHADER);
	assert(frag);
	glShaderSource(frag, 1, &strfrag, NULL);
	glCompileShader(frag);

	prog = glCreateProgram();
	assert(prog);
	glBindAttribLocation(prog, ATTR_POSITION_INDEX, "position");
	glBindAttribLocation(prog, ATTR_COLOR_INDEX, "color");
	glBindAttribLocation(prog, ATTR_TEXCOORD_INDEX, "texCoord");
	glAttachShader(prog, vert);
	glAttachShader(prog, frag);
	glLinkProgram(prog);

	GLint status;
	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		printLog(vert, "vertex");
		printLog(frag, "fragment");
		printLog(prog, "program");
		return NULL;
	}

	Shader* shader = new Shader;
	shader->prog = prog;
	shader->vert = vert;
	shader->frag = frag;
	return shader;
}

void Display::use_shader(Shader* shader)
{
	DEBUG();
	buffer.assert_empty();

	glUseProgram(shader ? shader->prog : default_shader->prog);
}

void Display::feed_shader(Shader* shader, const char* name, float value)
{
	assert(shader);
	assert(name);
	GLint prog;
	glGetIntegerv(GL_CURRENT_PROGRAM, &prog);

	glUseProgram(shader->prog);
	GLint loc = glGetUniformLocation(shader->prog, name);
	if (loc >= 0)
		glUniform1f(loc, value);
	else
		printf("no location for %s\n", name);

	glUseProgram(prog);
}

void Display::free_shader(Shader* shader)
{
	GLuint prog;
	glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&prog);
	if (prog == shader->prog)
		use_shader(default_shader);

	glDeleteShader(shader->vert);
	glDeleteShader(shader->frag);
	glDeleteProgram(shader->prog);
	delete shader;
}
