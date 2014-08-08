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
#include <SDL2/SDL_opengles2.h>
#else
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#endif

#define GL_VERTEX_PROGRAM_POINT_SIZE 0x8642

#include <cassert>
#include <cerrno>
#include <cmath>

#include "display.hpp"

#ifdef EMSCRIPTEN
#include "emscripten.h" // see show_cursor
#endif

#include "log.hpp"
#include "shader.hpp"

log_category("display");

#ifdef DODEBUG
static const char* getGLError(GLenum error)
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
}

#define GLDEBUG(x) \
	x; \
	{ \
		GLenum e; \
		while((e=glGetError()) != GL_NO_ERROR) \
		{ \
			log_debug("%s for call %s", getGLError(e), #x); \
			exit(1); \
		} \
	}
#else
#define GLDEBUG(x) \
	x;
#endif


Display::Display(bool server_mode) :
	default_buffer(false),
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
	alpha(1),
	camera(),
	point_size(1),
	original_width(0),
	original_height(0),
	available(false),
	debug_mode(false)
{
	int r;
	if (server_mode) { // fix those hacks!
		available = true;
		return;
	}
	r = SDL_InitSubSystem(SDL_INIT_VIDEO);
	if (r != 0) {
		log_error("Cannot initialize SDL video subsystem");
		return;
	}
	// create the window in the constructor
	// so we have an opengl context ready for the user
	r = create_window(2, 2);
	if (r < 0) {
		log_error("Cannot create window");
		return;
	}

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
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
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

void Display::set_fullscreen(bool fullscreen)
{
	if (fullscreen) {
#ifdef EMSCRIPTEN
		emscripten_run_script("var canvas = window.document.getElementById('canvas');canvas.width=window.innerWidth;canvas.height=window.innerHeight;");
#else
		SDL_SetWindowFullscreen(sdl_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
#endif
		int w, h;
		SDL_GetWindowSize(sdl_window, &w, &h);
		// freed by lua's gc
		screen = new_surface(w, h, true);
		draw_on(screen);
	} else {
#ifndef EMSCRIPTEN
		SDL_SetWindowFullscreen(sdl_window, SDL_FALSE);
#endif
		resize(original_width, original_height);
	}
}

void Display::resize(int w, int h)
{
	int currentw, currenth;
	SDL_GetWindowSize(sdl_window, &currentw, &currenth);
	if (w == currentw && h == currenth)
		return;
	original_width = w;
	original_height = h;
#ifdef EMSCRIPTEN
	emscripten_set_canvas_size(w, h);
#else
	int posx, posy;
	SDL_GetWindowPosition(sdl_window, &posx, &posy);
	SDL_SetWindowSize(sdl_window, w, h); // resize
	SDL_SetWindowPosition(sdl_window,
	                      posx + currentw / 2 - w / 2,
	                      posy + currenth / 2 - h / 2); // and move it back
#endif
	SDL_GetWindowSize(sdl_window, &w, &h);
	// freed by lua's gc
	screen = new_surface(w, h, true);
	draw_on(screen);
}

int Display::create_window(int w, int h)
{
	w = w > 0 ? w : 1;
	h = h > 0 ? h : 1;

#ifndef EMSCRIPTEN
	sdl_window = SDL_CreateWindow("Drystal",
	                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	                              w, h, SDL_WINDOW_OPENGL);
	if (!sdl_window) {
		return -1;
	}
	SDL_GL_CreateContext(sdl_window);
#else
	original_width = w;
	original_height = h;
	SDL_SetVideoMode(w, h, 32, SDL_OPENGL);
#endif

	SDL_GL_SetSwapInterval(1);
	SDL_GetWindowSize(sdl_window, &w, &h);

	screen = new_surface(w, h, true);
	if (!screen) {
		return -ENOMEM;
	}
	draw_on(screen);

	default_shader = create_default_shader();
	use_default_shader();
	default_buffer.allocate();

	set_blend_mode(DEFAULT);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
	glEnable(GL_TEXTURE_2D);

#ifndef EMSCRIPTEN
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
#endif

	return 0;
}

void Display::screen2scene(float x, float y, float * tx, float * ty) const
{
	assert(tx);
	assert(ty);

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
	current_buffer->check_empty();
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
	Camera oldcamera = camera;

	// draw 'screen' on real screen, using default context (buffer, color, no debug, etc)
	use_default_buffer();
	draw_from(screen);
	use_default_shader();
	set_color(255, 255, 255);
	set_alpha(255);
	reset_camera();
	float w = screen->w;
	float h = screen->h;
	debug_mode = false;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	draw_quad(0, 0, w, 0, w, h, 0, h,
	          0, h, w, h, w, 0, 0, 0); // y reversed
	current_buffer->check_empty();
	SDL_GL_SwapWindow(sdl_window);

	// restore context
	current->draw_on();
	if (oldfrom)
		draw_from(oldfrom);
	debug_mode = olddebug;
	use_buffer(oldbuffer);
	this->r = oldr;
	this->g = oldg;
	this->b = oldb;
	this->alpha = oldalpha;
	set_camera_angle(oldcamera.angle);
	set_camera_position(oldcamera.dx, oldcamera.dy);
	set_camera_zoom(oldcamera.zoom);

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
	assert(surface);

	surface->set_filter(filter, current_from);
}

void Display::reset_camera()
{
	current_buffer->check_empty();

	camera.reset();
	camera.update_matrix(current->w, current->h);
}

void Display::set_camera_position(float dx, float dy)
{
	current_buffer->check_empty();

	camera.dx = dx;
	camera.dy = dy;

	camera.update_matrix(current->w, current->h);
}

void Display::set_camera_angle(float angle)
{
	current_buffer->check_empty();

	camera.angle = angle;
	camera.update_matrix(current->w, current->h);
}

void Display::set_camera_zoom(float zoom)
{
	current_buffer->check_empty();

	camera.zoom = zoom;
}

void Display::draw_from(Surface* surf)
{
	assert(surf);
	if (current_from != surf) {
		current_buffer->check_empty();
		this->current_from = surf;
		surf->draw_from();
	}
}

void Display::draw_on(Surface* surf)
{
	assert(surf);
	if (current != surf) {
		current_buffer->check_empty();
		this->current = surf;
		surf->draw_on();

		int w = surf->w;
		int h = surf->h;
		glViewport(0, 0, w, h);
		camera.update_matrix(w, h);
		current_buffer->draw_on = current;
		default_buffer.draw_on = current;
	}
}

/**
 * Surface
 */

Surface * Display::create_surface(unsigned int w, unsigned int h, unsigned int texw, unsigned int texh, unsigned char* pixels) const
{
	Surface* surface = new Surface(w, h, texw, texh, pixels, current_from, current);
	if (!surface) {
		return NULL;
	}

	return surface;
}

int Display::load_surface(const char * filename, Surface **surface) const
{
	return Surface::load(filename, surface, current_from);
}

#define RGBA_SIZE 4
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

	Surface *surface = create_surface(w, h, potw, poth, NULL);
	if (!surface) {
		return NULL;
	}
	if (force_npot) {
		surface->npot = true;
	}
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
	delete surface;
}

/**
 * Primitive drawing
 */

void Display::draw_point(float x, float y)
{
	current_buffer->check_type(POINT_BUFFER);
	current_buffer->check_not_use_texture();
	current_buffer->check_not_full();

	current_buffer->push_vertex(x, y);
	current_buffer->push_point_size(point_size);
	current_buffer->push_color(r, g, b, alpha);
}
void Display::draw_point_tex(float xi, float yi, float xd, float yd)
{
	float xxi, yyi;
	convert_texcoords(xi, yi, &xxi, &yyi);

	current_buffer->check_type(POINT_BUFFER);
	current_buffer->check_use_texture();
	current_buffer->check_not_full();

	current_buffer->push_vertex(xd, yd);
	current_buffer->push_tex_coord(xxi, yyi);
	current_buffer->push_point_size(point_size);
	current_buffer->push_color(r, g, b, alpha);
}

void Display::draw_line(float x1, float y1, float x2, float y2)
{
	current_buffer->check_type(LINE_BUFFER);
	current_buffer->check_not_use_texture();
	current_buffer->check_not_full();

	current_buffer->push_vertex(x1, y1);
	current_buffer->push_vertex(x2, y2);
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

	current_buffer->check_type(TRIANGLE_BUFFER);
	current_buffer->check_not_use_texture();
	current_buffer->check_not_full();

	current_buffer->push_vertex(x1, y1);
	current_buffer->push_vertex(x2, y2);
	current_buffer->push_vertex(x3, y3);
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

	current_buffer->check_type(TRIANGLE_BUFFER);
	current_buffer->check_use_texture();
	current_buffer->check_not_full();

	current_buffer->push_tex_coord(xxi1, yyi1);
	current_buffer->push_tex_coord(xxi2, yyi2);
	current_buffer->push_tex_coord(xxi3, yyi3);

	current_buffer->push_vertex(xo1, yo1);
	current_buffer->push_vertex(xo2, yo2);
	current_buffer->push_vertex(xo3, yo3);

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
		log_error("Failed to compile default shader: %s", error);
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

	assert(strfragtex);
	assert(strfragcolor);
	assert(strvert);

	const char* new_strvert[] = {
		SHADER_PREFIX,
		strvert
	};
	const char* new_strfragcolor[] = {
		SHADER_PREFIX,
		strfragcolor
	};
	const char* new_strfragtex[] = {
		SHADER_PREFIX,
		strfragtex
	};

	vert = glCreateShader(GL_VERTEX_SHADER);
	assert(vert);
	glShaderSource(vert, 2, new_strvert, NULL);
	glCompileShader(vert);

	frag_color = glCreateShader(GL_FRAGMENT_SHADER);
	assert(frag_color);
	glShaderSource(frag_color, 2, new_strfragcolor, NULL);
	glCompileShader(frag_color);

	frag_tex = glCreateShader(GL_FRAGMENT_SHADER);
	assert(frag_tex);
	glShaderSource(frag_tex, 2, new_strfragtex, NULL);
	glCompileShader(frag_tex);

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

	Shader* shader = new Shader(prog_color, prog_tex, vert, frag_color, frag_tex);
	return shader;
}

void Display::use_shader(Shader* shader)
{
	assert(shader);

	current_buffer->check_empty();

	current_shader = shader;
	current_buffer->use_shader(shader);
}

void Display::use_default_shader()
{
	use_shader(default_shader);
}

void Display::free_shader(Shader* shader)
{
	assert(shader);
	if (shader != default_shader) {
		GLuint prog;
		glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&prog);
		if (prog == shader->prog_color || prog == shader->prog_tex)
			use_default_shader();
	}

	delete shader;
}

Buffer * Display::new_buffer(unsigned int size)
{
	Buffer* buffer = new Buffer(true, size);
	buffer->allocate();
	buffer->use_camera(&camera);
	return buffer;
}

void Display::use_buffer(Buffer* buffer)
{
	assert(buffer);

	current_buffer = buffer;
	current_buffer->use_shader(current_shader);
	current_buffer->draw_on = current;
}

void Display::use_default_buffer()
{
	use_buffer(&default_buffer);
}

void Display::draw_buffer(Buffer* buffer, float dx, float dy)
{
	assert(buffer);
	current_buffer->check_empty();
	buffer->draw_on = current;
	buffer->draw(dx, dy);
}

