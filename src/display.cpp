/**
 * This file is part of Drystal.
 *
 * Drystal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Drystal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Drystal.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef EMSCRIPTEN
#include <SDL2/SDL.h>
#else
#include <SDL/SDL.h>
#endif

#define GL_VERTEX_PROGRAM_POINT_SIZE 0x8642

#include <cstdio>
#include <cassert>
#include <cstring>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "display.hpp"

#ifdef EMSCRIPTEN
#include "emscripten.h" // see show_cursor
#endif

#include "log.hpp"


#define STRINGIZE(x) #x
#define STRINGIZE2(x) STRINGIZE(x)
#define SHADER_STRING(text) STRINGIZE2(text)
#define HASH(x) x

const char* SHADER_PREFIX = SHADER_STRING
                            (
                                HASH(#)ifdef GL_ES \n
                                precision mediump float; \n
                                HASH(#)endif \n
                            );
const size_t SHADER_PREFIX_LEN = strlen(SHADER_PREFIX);

const char* DEFAULT_VERTEX_SHADER = SHADER_STRING
                                    (
                                        attribute vec2 position;	// position of the vertice
                                        attribute vec4 color;		// color of the vertice
                                        attribute vec2 texCoord;	// texture coordinates
                                        attribute float pointSize;	// size of points

                                        varying vec4 fColor;
                                        varying vec2 fTexCoord;

                                        uniform float cameraDx;
                                        uniform float cameraDy;
                                        uniform float cameraZoom;
                                        uniform mat2 rotationMatrix;
                                        mat2 cameraMatrix = rotationMatrix * cameraZoom;

                                        void main()
{
	gl_PointSize = pointSize * cameraZoom;
	vec2 position2d = cameraMatrix  * (position - vec2(cameraDx, cameraDy));
	gl_Position = vec4(position2d, 0.0, 1.0);
	fColor = color;
	fTexCoord = texCoord;
}
                                    );

const char* DEFAULT_FRAGMENT_SHADER_COLOR = SHADER_STRING
        (
            varying vec4 fColor;
            varying vec2 fTexCoord;

            void main()
{
	gl_FragColor = fColor;
}
        );

const char* DEFAULT_FRAGMENT_SHADER_TEX = SHADER_STRING
        (
            uniform sampler2D tex;

            varying vec4 fColor;
            varying vec2 fTexCoord;

            void main()
{
	vec4 color;
	vec4 texval = texture2D(tex, fTexCoord);
	color.rgb = mix(texval.rgb, fColor.rgb, vec3(1.) - fColor.rgb);
	color.a = texval.a * fColor.a;
	gl_FragColor = color;
}
        );

Display::Display(bool server_mode) :
	default_buffer(),
	sdl_window(NULL),
	screen(NULL),
	default_shader(NULL),
	current_shader(NULL),
	current(NULL),
	current_from(NULL),
	current_buffer(&default_buffer),
	r(1),
	g(1),
	b(1),
	alpha( 1),
	camera(),
	point_size(1),
	available(false),
	debug_mode(false)
{
	if (server_mode) { // fix those hacks!
		available = true;
		return;
	}
	int err = SDL_Init(SDL_INIT_VIDEO);
	if (err) {
		fprintf(stderr, "[ERROR] cannot initialize SDL\n");
		return;
	}
	// create the window in the constructor
	// so we have an opengl context ready for the user
	create_window(2, 2);

	default_buffer.use_camera(&camera);

	available = true;
}

Display::~Display()
{
	if (default_shader) {
		free_shader(default_shader);
		default_shader = NULL;
	}
	if (sdl_window) {
		SDL_DestroyWindow(sdl_window);
	}
	if (screen) {
		// freed by lua's gc
		screen = NULL;
	}
}

bool Display::is_available() const
{
	return available;
}

/**
 * Screen
 */

void Display::set_title(const char *title) const
{
	assert(title);
	assert(sdl_window);

	SDL_SetWindowTitle(sdl_window, title);
}

void Display::resize(int w, int h)
{
#ifdef EMSCRIPTEN
	emscripten_set_canvas_size(w, h);
#else
	int oldw, oldh;
	int posx, posy;
	SDL_GetWindowSize(sdl_window, &oldw, &oldh);
	SDL_GetWindowPosition(sdl_window, &posx, &posy);
	SDL_SetWindowSize(sdl_window, w, h); // resize
	SDL_SetWindowPosition(sdl_window,
			posx + oldw / 2 - w / 2,
			posy + oldh / 2 - h / 2); // and move it back
#endif
	SDL_GetWindowSize(sdl_window, &w, &h);
	// freed by lua's gc
	screen = new_surface(w, h, true);
	current = NULL; // force update
	draw_on(screen);
}

void Display::create_window(int w, int h)
{
	w = w > 0 ? w : 1;
	h = h > 0 ? h : 1;

#ifndef EMSCRIPTEN
	sdl_window = SDL_CreateWindow("Drystal",
	                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	                              w, h, SDL_WINDOW_OPENGL);
	SDL_GL_CreateContext(sdl_window);
	assert(sdl_window);
#else
	SDL_SetVideoMode(w, h, 32, SDL_OPENGL);
#endif

	SDL_GL_SetSwapInterval(1);
	SDL_GetWindowSize(sdl_window, &w, &h);

	screen = new_surface(w, h, true);
	assert(screen);
	draw_on(screen);

	default_shader = create_default_shader();
	use_shader(default_shader);
	default_buffer.allocate();

	set_blend_mode(DEFAULT);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
	glEnable(GL_TEXTURE_2D);

#ifndef EMSCRIPTEN
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
#endif
}

void Display::screen2scene(float x, float y, float * tx, float * ty) const
{
	float zoom = camera.zoom;
	x -= camera.dx;
	y -= camera.dy;
	*tx = camera.matrix[0] * x + camera.matrix[2] * y;
	*ty = camera.matrix[1] * x + camera.matrix[3] * y;

	float dx = (x + camera.dx) - screen->w / 2;
	float dy = (y + camera.dy) - screen->h / 2;
	// MAGIC, don't modify
	*tx += dx * (1 - zoom) / zoom;
	*ty += dy * (1 - zoom) / zoom;
}

void Display::show_cursor(bool b) const
{
#ifndef EMSCRIPTEN
	SDL_ShowCursor(b);
#else
	if (b) {
		emscripten_run_script("Module['canvas'].style.cursor='default';");
	} else {
		emscripten_run_script("Module['canvas'].style.cursor='none';");
	}
#endif
}

void Display::draw_background() const
{
	glClearColor(r, g, b, alpha);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Display::flip()
{
	GLDEBUG();
	// save context
	Surface* oldfrom = current_from;
	Buffer* oldbuffer = current_buffer;
	float oldr = r;
	float oldg = g;
	float oldb = b;
	float oldalpha = alpha;
	bool olddebug = debug_mode;

	// draw 'screen' on real screen, using default context (buffer, color, no debug, etc)
	use_buffer(NULL);
	draw_from(screen);
	use_shader(default_shader);
	set_color(255, 255, 255);
	set_alpha(255);
	float w = screen->w;
	float h = screen->h;
	debug_mode = false;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	draw_quad(0, 0, w, 0, w, h, 0, h,
				0, h, w, h, w, 0, 0, 0); // y reversed
	current_buffer->check_empty();
	SDL_GL_SwapWindow(sdl_window);

	// restore context
	glBindFramebuffer(GL_FRAMEBUFFER, current->fbo);
	if (oldfrom)
		draw_from(oldfrom);
	debug_mode = olddebug;
	use_buffer(oldbuffer);
	this->r = oldr;
	this->g = oldg;
	this->b = oldb;
	this->alpha = oldalpha;

	GLDEBUG();
}

Surface * Display::get_screen() const
{
	return screen;
}

void Display::toggle_debug_mode()
{
	debug_mode = !debug_mode;
}

/**
 * State
 */

void Display::set_color(int red, int green, int blue)
{
	assert(red >= 0 && red <= 255);
	assert(green >= 0 && green <= 255);
	assert(blue >= 0 && blue <= 255);

	this->r = red / 255.;
	this->g = green / 255.;
	this->b = blue / 255.;
}

void Display::set_alpha(int a)
{
	assert(a >= 0 && a <= 255);

	this->alpha = a / 255.;
}

void Display::set_point_size(float size)
{
	assert(size >= 0);
	this->point_size = size;
}

void Display::set_line_width(float width)
{
	assert(width >= 0);
	glLineWidth(width);
}

void Display::set_blend_mode(BlendMode mode)
{
	current_buffer->check_empty();

	switch (mode) {
		case ALPHA:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glBlendEquation(GL_FUNC_ADD);
			break;
		case MULT:
			glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
			glBlendEquation(GL_FUNC_ADD);
			break;
		case ADD:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glBlendEquation(GL_FUNC_ADD);
			break;
	}
}
void Display::set_filter(Surface* surface, FilterMode filter) const
{
	if (surface->filter != filter) {
		surface->filter = filter;

		glBindTexture(GL_TEXTURE_2D, surface->tex);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter > LINEAR ? LINEAR : filter);
		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.);
		GLDEBUG();
		//float maximumAnisotropy;
		//glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maximumAnisotropy);
		//DEBUGV("ani: %f", maximumAnisotropy);

		glBindTexture(GL_TEXTURE_2D, current_from ? current_from->tex : 0);
	}
}

void Display::reset_camera()
{
	current_buffer->check_empty();

	camera.dx = camera.dy = 0;
	camera.dx_transformed = camera.dy_transformed = 0;
	camera.angle = 0;
	camera.zoom = 1;
	update_camera_matrix();
}

void Display::set_camera_position(float dx, float dy)
{
	current_buffer->check_empty();

	camera.dx = dx;
	camera.dy = dy;

	update_camera_matrix();
}

void Display::set_camera_angle(float angle)
{
	current_buffer->check_empty();

	camera.angle = angle;

	update_camera_matrix();
}

void Display::set_camera_zoom(float zoom)
{
	current_buffer->check_empty();

	camera.zoom = zoom;
}

void Display::update_camera_matrix()
{
	assert(current);

	float angle = camera.angle;

	float ratio = (float) current->w / current->h;
	camera.matrix[0] = cos(angle);
	camera.matrix[1] = sin(angle) * ratio;
	camera.matrix[2] = -sin(angle) / ratio;
	camera.matrix[3] = cos(angle);

	float ddx = 2 * (- camera.dx / current->w);
	float ddy = 2 * (- camera.dy / current->h);
	camera.dx_transformed = ddx;
	camera.dy_transformed = ddy;
}

void Display::draw_from(Surface* surf)
{
	assert(surf);
	if (current_from != surf) {
		current_buffer->check_empty();
		this->current_from = surf;
		glBindTexture(GL_TEXTURE_2D, current_from->tex);
		GLDEBUG();

		if (!surf->has_mipmap && surf->filter >= BILINEAR && !surf->npot) {
			glGenerateMipmap(GL_TEXTURE_2D);
			GLDEBUG();
			surf->has_mipmap = true;
		}
	}
}

void Display::draw_on(Surface* surf)
{
	assert(surf);
	if (current != surf) {
		current_buffer->check_empty();
		if (!surf->has_fbo) {
			create_fbo(surf);
		}
		surf->has_mipmap = false;
		this->current = surf;
		glBindFramebuffer(GL_FRAMEBUFFER, current->fbo);

		int w = surf->w;
		int h = surf->h;
		glViewport(0, 0, w, h);
		update_camera_matrix();
	}
}

/**
 * Surface
 */

Surface * Display::create_surface(int w, int h, int texw, int texh, unsigned char* pixels) const
{
	assert(pixels);

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texw, texh, 0,
	             GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	GLDEBUG();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	GLDEBUG();

	Surface* surface = new Surface;
	surface->tex = tex;
	surface->w = w;
	surface->h = h;
	surface->texw = texw;
	surface->texh = texh;
	surface->has_fbo = false;
	surface->has_mipmap = true;
	surface->npot = false;
	surface->filter = LINEAR;

	glBindTexture(GL_TEXTURE_2D, current_from ? current_from->tex : 0);

	return surface;
}

void Display::create_fbo(Surface* surface) const
{
	// gen framebuffer object
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
	                       GL_TEXTURE_2D, surface->tex, 0);
	GLDEBUG();
	assert(fbo != 0);

	GLenum status;
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	assert(status == GL_FRAMEBUFFER_COMPLETE);

	surface->fbo = fbo;
	surface->has_fbo = true;

	glBindFramebuffer(GL_FRAMEBUFFER, current ? current->fbo : 0);
}

#define RGBA_SIZE 4
Surface * Display::load_surface(const char * filename) const
{
	assert(filename);
	int w, h;
	int n;
	unsigned char *data = stbi_load(filename, &w, &h, &n, RGBA_SIZE);

	if (!data)
		return NULL;

	int potw = pow(2, ceil(log(w) / log(2)));
	int poth = pow(2, ceil(log(h) / log(2)));

	Surface* surface = NULL;

	if (potw != w || poth != h) {
		unsigned char *pixels = new unsigned char[potw * poth * RGBA_SIZE];
		memset(pixels, 0, potw * poth * RGBA_SIZE);
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				int is = (x + y * w) * RGBA_SIZE;
				int id = (x + y * potw) * RGBA_SIZE;
				memcpy(pixels + id, data + is, RGBA_SIZE);
			}
		}
		surface = create_surface(w, h, potw, poth, pixels);
		delete[] pixels;
	} else {
		surface = create_surface(w, h, w, h, data);
	}

	stbi_image_free(data);

	return surface;
}

Surface * Display::new_surface(int w, int h, bool force_npot) const
{
	assert(w > 0);
	assert(h > 0);
	int potw = pow(2, ceil(log(w) / log(2)));
	int poth = pow(2, ceil(log(h) / log(2)));
	if (force_npot) {
		potw = w;
		poth = h;
	}

	unsigned char *pixels = new unsigned char[potw * poth * RGBA_SIZE];
	memset(pixels, 0, potw * poth * RGBA_SIZE);

	Surface *surface = create_surface(w, h, potw, poth, pixels);
	if (force_npot) {
		surface->npot = true;
	}
	delete[] pixels;
	return surface;
}

void Display::free_surface(Surface* surface)
{
	assert(surface);
	if (surface == current_from) {
		current_buffer->check_not_use_texture();
		glBindTexture(GL_TEXTURE_2D, 0);
		current_from = NULL;
	}
	if (surface == current) {
		current_buffer->check_empty();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		current = NULL;
	}
	glDeleteTextures(1, &surface->tex);
	if (surface->has_fbo) {
		glDeleteFramebuffers(1, &surface->fbo);
	}
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

void Display::draw_point(float x, float y)
{
	float xx, yy;
	convert_coords(x, y, &xx, &yy);

	current_buffer->check_type(POINT_BUFFER);
	current_buffer->check_not_use_texture();

	current_buffer->push_vertex(xx, yy);
	current_buffer->push_point_size(point_size);
	current_buffer->push_color(r, g, b, alpha);
}
void Display::draw_point_tex(float xi, float yi, float xd, float yd)
{
	float xxd, yyd;
	convert_coords(xd, yd, &xxd, &yyd);

	float xxi, yyi;
	convert_texcoords(xi, yi, &xxi, &yyi);

	current_buffer->check_type(POINT_BUFFER);
	current_buffer->check_use_texture();
	current_buffer->check_not_full();

	current_buffer->push_vertex(xxd, yyd);
	current_buffer->push_tex_coord(xxi, yyi);
	current_buffer->push_point_size(point_size);
	current_buffer->push_color(r, g, b, alpha);
}

void Display::draw_line(float x1, float y1, float x2, float y2)
{
	float xx1, xx2;
	float yy1, yy2;
	convert_coords(x1, y1, &xx1, &yy1);
	convert_coords(x2, y2, &xx2, &yy2);

	current_buffer->check_type(LINE_BUFFER);
	current_buffer->check_not_use_texture();
	current_buffer->check_not_full();

	current_buffer->push_vertex(xx1, yy1);
	current_buffer->push_vertex(xx2, yy2);
	for (int i = 0; i < 2; i++)
		current_buffer->push_color(r, g, b, alpha);
}

void Display::draw_triangle(float x1, float y1, float x2, float y2, float x3, float y3)
{
	if (debug_mode) {
		draw_line(x1, y1, x2, y2);
		draw_line(x2, y2, x3, y3);
		draw_line(x3, y3, x1, y1);
		return;
	}
	float xx1, xx2, xx3;
	float yy1, yy2, yy3;
	convert_coords(x1, y1, &xx1, &yy1);
	convert_coords(x2, y2, &xx2, &yy2);
	convert_coords(x3, y3, &xx3, &yy3);

	current_buffer->check_type(TRIANGLE_BUFFER);
	current_buffer->check_not_use_texture();
	current_buffer->check_not_full();

	current_buffer->push_vertex(xx1, yy1);
	current_buffer->push_vertex(xx2, yy2);
	current_buffer->push_vertex(xx3, yy3);
	for (int i = 0; i < 3; i++)
		current_buffer->push_color(r, g, b, alpha);
}

void Display::draw_surface(float xi1, float yi1, float xi2, float yi2, float xi3, float yi3,
                           float xo1, float yo1, float xo2, float yo2, float xo3, float yo3)
{
	if (debug_mode) {
		draw_line(xo1, yo1, xo2, yo2);
		draw_line(xo2, yo2, xo3, yo3);
		draw_line(xo3, yo3, xo1, yo1);
		return;
	}
	assert(current_from);
	float xxi1, xxi2, xxi3;
	float yyi1, yyi2, yyi3;
	convert_texcoords(xi1, yi1, &xxi1, &yyi1);
	convert_texcoords(xi2, yi2, &xxi2, &yyi2);
	convert_texcoords(xi3, yi3, &xxi3, &yyi3);

	float xxo1, xxo2, xxo3;
	float yyo1, yyo2, yyo3;
	convert_coords(xo1, yo1, &xxo1, &yyo1);
	convert_coords(xo2, yo2, &xxo2, &yyo2);
	convert_coords(xo3, yo3, &xxo3, &yyo3);

	current_buffer->check_type(TRIANGLE_BUFFER);
	current_buffer->check_use_texture();
	current_buffer->check_not_full();

	current_buffer->push_tex_coord(xxi1, yyi1);
	current_buffer->push_tex_coord(xxi2, yyi2);
	current_buffer->push_tex_coord(xxi3, yyi3);

	current_buffer->push_vertex(xxo1, yyo1);
	current_buffer->push_vertex(xxo2, yyo2);
	current_buffer->push_vertex(xxo3, yyo3);

	for (int i = 0; i < 3; i++)
		current_buffer->push_color(r, g, b, alpha);
}

void Display::draw_quad(float xi1, float yi1, float xi2, float yi2, float xi3, float yi3, float xi4, float yi4,
                        float xo1, float yo1, float xo2, float yo2, float xo3, float yo3, float xo4, float yo4)
{
	draw_surface(xi1, yi1, xi2, yi2, xi3, yi3, xo1, yo1, xo2, yo2, xo3, yo3);
	draw_surface(xi1, yi1, xi3, yi3, xi4, yi4, xo1, yo1, xo3, yo3, xo4, yo4);
}


/**
 * Shader
 */
static char* getShaderError(GLuint obj)
{
	int length;

	if (glIsShader(obj)) {
		glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &length);
	} else {
		glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &length);
	}

	char* error = NULL;
	if (length > 1) { // make sure the error is explained here
		error = new char[length];

		if (glIsShader(obj))
			glGetShaderInfoLog(obj, length, NULL, error);
		else
			glGetProgramInfoLog(obj, length, NULL, error);
	}

	return error;
}

Shader * Display::create_default_shader()
{
	const char* strvert = DEFAULT_VERTEX_SHADER;
	const char* strfragcolor = DEFAULT_FRAGMENT_SHADER_COLOR;
	const char* strfragtex = DEFAULT_FRAGMENT_SHADER_TEX;
	char* error;
	Shader* shader = new_shader(strvert, strfragcolor, strfragtex, &error);
	if (!shader) {
		printf("Error compiling default shader:\n%s\n", error);
		delete[] error;
	}
	return shader;
}

Shader * Display::new_shader(const char* strvert, const char* strfragcolor, const char* strfragtex, char** error)
{
	GLuint vert = 0;
	GLuint frag_color = 0;
	GLuint frag_tex = 0;
	GLuint prog_color;
	GLuint prog_tex;

	if (!strvert || !*strvert) {
		strvert = DEFAULT_VERTEX_SHADER;
	}
	if (!strfragcolor || !*strfragcolor) {
		strfragcolor = DEFAULT_FRAGMENT_SHADER_COLOR;
	}
	if (!strfragtex || !*strfragtex) {
		strfragtex = DEFAULT_FRAGMENT_SHADER_TEX;
	}

	char* new_strvert = new char[SHADER_PREFIX_LEN + strlen(strvert) + 1];
	strcpy(new_strvert, SHADER_PREFIX);
	strcat(new_strvert, strvert);
	char* new_strfragcolor = new char[SHADER_PREFIX_LEN + strlen(strfragcolor) + 1];
	strcpy(new_strfragcolor, SHADER_PREFIX);
	strcat(new_strfragcolor, strfragcolor);
	char* new_strfragtex = new char[SHADER_PREFIX_LEN + strlen(strfragtex) + 1];
	strcpy(new_strfragtex, SHADER_PREFIX);
	strcat(new_strfragtex, strfragtex);

	vert = glCreateShader(GL_VERTEX_SHADER);
	assert(vert);
	glShaderSource(vert, 1, (const char**)&new_strvert, NULL);
	glCompileShader(vert);

	frag_color = glCreateShader(GL_FRAGMENT_SHADER);
	assert(frag_color);
	glShaderSource(frag_color, 1, (const char**)&new_strfragcolor, NULL);
	glCompileShader(frag_color);

	frag_tex = glCreateShader(GL_FRAGMENT_SHADER);
	assert(frag_tex);
	glShaderSource(frag_tex, 1, (const char**)&new_strfragtex, NULL);
	glCompileShader(frag_tex);

	delete[] new_strvert;
	delete[] new_strfragcolor;
	delete[] new_strfragtex;

	prog_color = glCreateProgram();
	assert(prog_color);
	glBindAttribLocation(prog_color, ATTR_POSITION_INDEX, "position");
	glBindAttribLocation(prog_color, ATTR_COLOR_INDEX, "color");
	glBindAttribLocation(prog_color, ATTR_TEXCOORD_INDEX, "texCoord");
	glBindAttribLocation(prog_color, ATTR_POINTSIZE_INDEX, "pointSize");
	glAttachShader(prog_color, vert);
	glAttachShader(prog_color, frag_color);
	glLinkProgram(prog_color);

	prog_tex = glCreateProgram();
	assert(prog_tex);
	glBindAttribLocation(prog_tex, ATTR_POSITION_INDEX, "position");
	glBindAttribLocation(prog_tex, ATTR_COLOR_INDEX, "color");
	glBindAttribLocation(prog_tex, ATTR_TEXCOORD_INDEX, "texCoord");
	glBindAttribLocation(prog_tex, ATTR_POINTSIZE_INDEX, "pointSize");
	glAttachShader(prog_tex, vert);
	glAttachShader(prog_tex, frag_tex);
	glLinkProgram(prog_tex);

	GLint status;
	glGetProgramiv(prog_color, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		if (error) {
			*error = getShaderError(vert);
			if (!*error)
				*error = getShaderError(frag_color);
			if (!*error)
				*error = getShaderError(prog_color);
		}
		return NULL;
	}

	glGetProgramiv(prog_tex, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		if (error) {
			*error = getShaderError(vert);
			if (!*error)
				*error = getShaderError(frag_tex);
			if (!*error)
				*error = getShaderError(prog_tex);
		}
		return NULL;
	}

	Shader* shader = new Shader;
	shader->vert = vert;
	shader->frag_color = frag_color;
	shader->frag_tex = frag_tex;
	shader->prog_color = prog_color;
	shader->prog_tex = prog_tex;

	shader->vars[COLOR].dxLocation = glGetUniformLocation(prog_color, "cameraDx");
	shader->vars[COLOR].dyLocation = glGetUniformLocation(prog_color, "cameraDy");
	shader->vars[COLOR].zoomLocation = glGetUniformLocation(prog_color, "cameraZoom");
	shader->vars[COLOR].rotationMatrixLocation = glGetUniformLocation(prog_color, "rotationMatrix");

	shader->vars[TEX].dxLocation = glGetUniformLocation(prog_tex, "cameraDx");
	shader->vars[TEX].dyLocation = glGetUniformLocation(prog_tex, "cameraDy");
	shader->vars[TEX].zoomLocation = glGetUniformLocation(prog_tex, "cameraZoom");
	shader->vars[TEX].rotationMatrixLocation = glGetUniformLocation(prog_tex, "rotationMatrix");

	return shader;
}

void Display::use_shader(Shader* shader)
{
	current_buffer->check_empty();

	if (!shader) {
		shader = default_shader;
	}
	current_shader = shader;
	current_buffer->use_shader(shader);
}

void Display::feed_shader(Shader* shader, const char* name, float value)
{
	assert(shader);
	assert(name);

	GLint prog;
	glGetIntegerv(GL_CURRENT_PROGRAM, &prog);

	GLint locColor = glGetUniformLocation(shader->prog_color, name);
	GLint locTex = glGetUniformLocation(shader->prog_tex, name);

	if (locColor >= 0) {
		glUseProgram(shader->prog_color);
		glUniform1f(locColor, value);
	}
	if (locTex >= 0) {
		glUseProgram(shader->prog_tex);
		glUniform1f(locTex, value);
	}

	if (locTex < 0 && locColor < 0) {
		printf("No location for %s.\n", name);
	}

	glUseProgram(prog);
}

void Display::free_shader(Shader* shader)
{
	assert(shader);
	GLuint prog;
	glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&prog);
	if (prog == shader->prog_color)
		use_shader(default_shader);

	glDeleteShader(shader->vert);
	glDeleteShader(shader->frag_color);
	glDeleteShader(shader->frag_tex);
	glDeleteProgram(shader->prog_color);
	glDeleteProgram(shader->prog_tex);
	delete shader;
}

Buffer * Display::new_buffer(unsigned int size)
{
	Buffer* buffer = new Buffer(size);
	buffer->allocate();
	buffer->use_camera(&camera);
	return buffer;
}
void Display::use_buffer(Buffer* buffer)
{
	if (!buffer) {
		current_buffer = &default_buffer;
	} else {
		current_buffer = buffer;
	}
	current_buffer->use_shader(current_shader);
}
void Display::draw_buffer(Buffer* buffer, float dx, float dy)
{
	assert(buffer);
	dx /= current->w;
	dy /= current->h;
	current_buffer->check_empty();
	buffer->draw(dx, dy);
}
void Display::reset_buffer(Buffer* buffer)
{
	assert(buffer);
	buffer->reset();
}
void Display::upload_and_free_buffer(Buffer* buffer)
{
	assert(buffer);
	buffer->upload_and_free();
}
void Display::free_buffer(Buffer* buffer)
{
	assert(buffer);
	delete buffer;
}

const char* getGLError(GLenum error)
{
#define casereturn(x) case x: return #x
	switch (error) {
			casereturn(GL_INVALID_ENUM);
			casereturn(GL_INVALID_VALUE);
			casereturn(GL_INVALID_OPERATION);
			casereturn(GL_INVALID_FRAMEBUFFER_OPERATION);
			casereturn(GL_OUT_OF_MEMORY);
		default:
		case GL_NO_ERROR:
			return "";
	}
#undef casereturn
	return "";
}
