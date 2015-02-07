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

#include "emscripten.h" // see show_cursor
#endif

#define GL_VERTEX_PROGRAM_POINT_SIZE 0x8642

#include <assert.h>
#include <errno.h>
#include <math.h>

#include "display.h"
#include "log.h"
#include "shader.h"
#include "surface.h"
#include "camera.h"
#include "buffer.h"
#include "util.h"
#include "opengl_util.h"

log_category("display");

struct Display {
	Buffer *default_buffer;
	SDL_Window *sdl_window;
	Surface *screen;

	Shader *default_shader;
	Shader *current_shader;

	Surface *current_on;
	Surface *current_from;

	Buffer *current_buffer;

	float r;
	float g;
	float b;
	float alpha;

	Camera *camera;

	int original_width;
	int original_height;

	bool available;
	bool debug_mode;
} display;

static inline void display_convert_texcoords(float x, float y, float *dx, float *dy)
{
	*dx = x / display.current_from->texw;
	*dy = y / display.current_from->texh;
}

static Shader *display_create_default_shader()
{
	const char* strvert = DEFAULT_VERTEX_SHADER;
	const char* strfragcolor = DEFAULT_FRAGMENT_SHADER_COLOR;
	const char* strfragtex = DEFAULT_FRAGMENT_SHADER_TEX;
	char* error;
	Shader* shader = display_new_shader(strvert, strfragcolor, strfragtex, &error);
	if (!shader) {
		log_error("Failed to compile default shader:\n%s", error);
		free(error);
	}
	return shader;
}

static int display_create_window(int w, int h)
{
	assert(w > 0);
	assert(h > 0);

#ifndef EMSCRIPTEN
	display.sdl_window = SDL_CreateWindow("Drystal",
	                                      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	                                      w, h, SDL_WINDOW_OPENGL);
	if (!display.sdl_window) {
		return -1;
	}
	SDL_GL_CreateContext(display.sdl_window);
#else
	display.original_width = w;
	display.original_height = h;
	SDL_SetVideoMode(w, h, 32, SDL_OPENGL);
#endif

	SDL_GL_SetSwapInterval(1);
	SDL_GetWindowSize(display.sdl_window, &w, &h);

	display.screen = display_new_surface(w, h, true);
	display_draw_on(display.screen);

	display.default_shader = display_create_default_shader();
	if (!display.default_shader) {
		return -1;
	}
	display_use_default_shader();
	buffer_allocate(display.default_buffer);

	display_set_blend_mode(BLEND_DEFAULT);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
	glEnable(GL_TEXTURE_2D);

#ifndef EMSCRIPTEN
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
#endif

	return 0;
}


void display_init()
{
	int r;

	display.default_buffer = buffer_new(false, BUFFER_DEFAULT_SIZE);
	display.camera = camera_new();
	display.sdl_window = NULL;
	display.screen = NULL;
	display.default_shader = NULL;
	display.current_shader = NULL;
	display.current_on = NULL;
	display.current_from = NULL;
	display.current_buffer = display.default_buffer;
	display.r = 1;
	display.g = 1;
	display.b = 1;
	display.alpha = 1;
	display.original_width = 0;
	display.original_height = 0;
	display.available = false;
	display.debug_mode = false;

	r = SDL_InitSubSystem(SDL_INIT_VIDEO);
	if (r != 0) {
		log_error("Cannot initialize SDL video subsystem");
		return;
	}
	// create the window in the constructor
	// so we have an opengl context ready for the user
	r = display_create_window(2, 2);
	if (r < 0) {
		log_error("Cannot create window");
		return;
	}

	buffer_use_camera(display.default_buffer, display.camera);

	display.available = true;
}

void display_free()
{
	display_free_shader(display.default_shader);
	display.default_shader = NULL;

	buffer_free(display.default_buffer);
	display.default_buffer = NULL;

	camera_free(display.camera);
	display.camera = NULL;

	if (display.sdl_window) {
		SDL_DestroyWindow(display.sdl_window);
	}
	if (display.screen) {
		// freed by lua's gc
		display.screen = NULL;
	}
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void display_get_color(int *red, int *green, int *blue)
{
	assert(red);
	assert(green);
	assert(blue);

	*red = display.r * 255;
	*green = display.g * 255;
	*blue = display.b * 255;
}

void display_get_alpha(int *a)
{
	assert(a);

	*a = display.alpha * 255;
}

Buffer *display_get_current_buffer(void)
{
	return display.current_buffer;
}

bool display_is_available()
{
	return display.available;
}

/**
 * Screen
 */

void display_set_title(const char *title)
{
	assert(title);
	assert(display.sdl_window);

	SDL_SetWindowTitle(display.sdl_window, title);
}

void display_set_fullscreen(bool fullscreen)
{
	if (fullscreen) {
		int w, h;
#ifdef EMSCRIPTEN
		emscripten_run_script("var canvas = window.document.getElementById('canvas');canvas.width=window.innerWidth;canvas.height=window.innerHeight;");
#else
		SDL_SetWindowFullscreen(display.sdl_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
#endif
		SDL_GetWindowSize(display.sdl_window, &w, &h);
		// freed by lua's gc
		display.screen = display_new_surface(w, h, true);
		display_draw_on(display.screen);
	} else {
#ifndef EMSCRIPTEN
		SDL_SetWindowFullscreen(display.sdl_window, SDL_FALSE);
#endif
		display_resize(display.original_width, display.original_height);
	}
}

void display_resize(int w, int h)
{
	int currentw, currenth;
	int posx, posy;

	SDL_GetWindowSize(display.sdl_window, &currentw, &currenth);
	if (w == currentw && h == currenth)
		return;
	display.original_width = w;
	display.original_height = h;
#ifdef EMSCRIPTEN
	emscripten_set_canvas_size(w, h);
#else
	SDL_GetWindowPosition(display.sdl_window, &posx, &posy);
	SDL_SetWindowSize(display.sdl_window, w, h); // resize
	SDL_SetWindowPosition(display.sdl_window,
	                      posx + currentw / 2 - w / 2,
	                      posy + currenth / 2 - h / 2); // and move it back
#endif
	SDL_GetWindowSize(display.sdl_window, &w, &h);
	// freed by lua's gc
	display.screen = display_new_surface(w, h, true);
	display_draw_on(display.screen);
}

void display_screen2scene(float x, float y, float * tx, float * ty)
{
	assert(tx);
	assert(ty);

	Camera *camera = display.camera;
	Surface *screen = display.screen;

	float zoom = camera->zoom;
	x -= camera->dx;
	y -= camera->dy;
	*tx = camera->matrix[0] * x + camera->matrix[2] * y;
	*ty = camera->matrix[1] * x + camera->matrix[3] * y;

	float dx = (x + camera->dx) - screen->w / 2;
	float dy = (y + camera->dy) - screen->h / 2;
	// MAGIC, don't modify
	*tx += dx * (1 - zoom) / zoom;
	*ty += dy * (1 - zoom) / zoom;
}

void display_show_cursor(bool b)
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

void display_draw_background()
{
	buffer_check_empty(display.current_buffer);
	glClearColor(display.r, display.g, display.b, display.alpha);
	glClear(GL_COLOR_BUFFER_BIT);
}

void display_flip()
{
	GLDEBUG();

	// save context
	Surface *oldfrom = display.current_from;
	Buffer *oldbuffer = display.current_buffer;
	float oldr = display.r;
	float oldg = display.g;
	float oldb = display.b;
	float oldalpha = display.alpha;
	bool olddebug = display.debug_mode;
	Camera oldcamera = *display.camera;

	// draw 'screen' on real screen, using default context (buffer, color, no debug, etc)
	display_use_default_buffer();
	display_draw_from(display.screen);
	display_use_default_shader();
	display_set_color(255, 255, 255);
	display_set_alpha(255);
	display_reset_camera();
	float w = display.screen->w;
	float h = display.screen->h;
	display.debug_mode = false;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0., 0., 0., 1.);
	glClear(GL_COLOR_BUFFER_BIT);
	display_draw_quad(0, 0, w, 0, w, h, 0, h,
	                  0, h, w, h, w, 0, 0, 0); // y reversed
	buffer_check_empty(display.current_buffer);
	SDL_GL_SwapWindow(display.sdl_window);

	// restore context
	surface_draw_on(display.current_on);
	if (oldfrom)
		display_draw_from(oldfrom);
	display_use_buffer(oldbuffer);
	display.debug_mode = olddebug;
	display.r = oldr;
	display.g = oldg;
	display.b = oldb;
	display.alpha = oldalpha;
	display_set_camera_angle(oldcamera.angle);
	display_set_camera_position(oldcamera.dx, oldcamera.dy);
	display_set_camera_zoom(oldcamera.zoom);

	GLDEBUG();
}

Surface *display_get_screen()
{
	return display.screen;
}

void display_toggle_debug_mode()
{
	display.debug_mode = !display.debug_mode;
}

bool display_is_debug()
{
	return display.debug_mode;
}

/**
 * State
 */

void display_set_color(int red, int green, int blue)
{
	assert(red >= 0 && red <= 255);
	assert(green >= 0 && green <= 255);
	assert(blue >= 0 && blue <= 255);

	display.r = red / 255.;
	display.g = green / 255.;
	display.b = blue / 255.;
}

void display_set_alpha(int a)
{
	assert(a >= 0 && a <= 255);

	display.alpha = a / 255.;
}

void display_set_line_width(float width)
{
	assert(width >= 0);
	buffer_check_empty(display.current_buffer);
	glLineWidth(width);
}

void display_set_blend_mode(BlendMode mode)
{
	buffer_check_empty(display.current_buffer);

	switch (mode) {
		case BLEND_ALPHA:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glBlendEquation(GL_FUNC_ADD);
			break;
		case BLEND_MULT:
			glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
			glBlendEquation(GL_FUNC_ADD);
			break;
		case BLEND_ADD:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glBlendEquation(GL_FUNC_ADD);
			break;
	}
}

void display_set_filter(Surface* surface, FilterMode filter)
{
	assert(surface);

	surface_set_filter(surface, filter, display.current_from);
}

Camera *display_get_camera()
{
	return display.camera;
}

void display_reset_camera()
{
	buffer_check_empty(display.current_buffer);

	camera_reset(display.camera);
	camera_update_matrix(display.camera, display.current_on->w, display.current_on->h);
}

void display_set_camera_position(float dx, float dy)
{
	buffer_check_empty(display.current_buffer);

	display.camera->dx = dx;
	display.camera->dy = dy;

	camera_update_matrix(display.camera, display.current_on->w, display.current_on->h);
}

void display_set_camera_angle(float angle)
{
	buffer_check_empty(display.current_buffer);

	display.camera->angle = angle;
	camera_update_matrix(display.camera, display.current_on->w, display.current_on->h);
}

void display_set_camera_zoom(float zoom)
{
	buffer_check_empty(display.current_buffer);

	display.camera->zoom = zoom;
}

Surface *display_get_draw_on()
{
	return display.current_on;
}

Surface *display_get_draw_from()
{
	return display.current_from;
}

void display_draw_from(Surface *surface)
{
	assert(surface);
	if (display.current_from != surface) {
		buffer_check_empty(display.current_buffer);
		display.current_from = surface;
		surface_draw_from(surface);
	}
}

void display_draw_on(Surface *surface)
{
	assert(surface);
	if (display.current_on != surface) {
		buffer_check_empty(display.current_buffer);
		display.current_on = surface;
		surface_draw_on(surface);

		int w = surface->w;
		int h = surface->h;
		glViewport(0, 0, w, h);
		camera_update_matrix(display.camera, w, h);
		display.current_buffer->draw_on = display.current_on;
		display.default_buffer->draw_on = display.current_on;
	}
}

/**
 * Surface
 */
Surface *display_create_surface(unsigned int w, unsigned int h, unsigned int texw, unsigned int texh, unsigned char* pixels)
{
	return surface_new(w, h, texw, texh, FORMAT_RGBA, pixels, display.current_from, display.current_on);
}

int display_load_surface(const char * filename, Surface **surface)
{
	return surface_load(filename, surface, display.current_from);
}

Surface *display_new_surface(int w, int h, bool force_npot)
{
	assert(w > 0);
	assert(h > 0);
	int potw = pow(2, ceil(log(w) / log(2)));
	int poth = pow(2, ceil(log(h) / log(2)));
	if (force_npot) {
		potw = w;
		poth = h;
	}

	Surface *surface = display_create_surface(w, h, potw, poth, NULL);
	if (force_npot) {
		surface->npot = true;
	}

	return surface;
}

void display_free_surface(Surface* surface)
{
	if (!surface)
		return;

	if (surface == display.current_from) {
		buffer_check_not_use_texture(display.current_buffer);
		glBindTexture(GL_TEXTURE_2D, 0);
		display.current_from = NULL;
	}
	if (surface == display.current_on) {
		buffer_check_empty(display.current_buffer);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		display.current_on = NULL;
	}
	surface_free(surface);
}

/**
 * Primitive drawing
 */

void display_draw_point(float x, float y, float size)
{
	Buffer *current_buffer = display.current_buffer;

	buffer_check_type(current_buffer, POINT_BUFFER);
	buffer_check_not_use_texture(current_buffer);
	buffer_check_not_full(current_buffer);

	buffer_push_vertex(current_buffer, x, y);
	buffer_push_point_size(current_buffer, size);
	buffer_push_color(current_buffer, display.r, display.g, display.b, display.alpha);
}
void display_draw_point_tex(float x, float y, float size)
{
	assert(display.current_from);
	Buffer *current_buffer = display.current_buffer;

	buffer_check_type(current_buffer, POINT_BUFFER);
	buffer_check_use_texture(current_buffer);
	buffer_check_not_full(current_buffer);

	buffer_push_vertex(current_buffer, x, y);
	buffer_push_tex_coord(current_buffer, 0, 0); // useless for textured points
	buffer_push_point_size(current_buffer, size);
	buffer_push_color(current_buffer, display.r, display.g, display.b, display.alpha);
}

void display_draw_line(float x1, float y1, float x2, float y2)
{
	Buffer *current_buffer = display.current_buffer;
	float r = display.r;
	float g = display.g;
	float b = display.b;
	float alpha = display.alpha;
	int i;

	buffer_check_type(current_buffer, LINE_BUFFER);
	buffer_check_not_use_texture(current_buffer);
	buffer_check_not_full(current_buffer);

	buffer_push_vertex(current_buffer, x1, y1);
	buffer_push_vertex(current_buffer, x2, y2);
	for (i = 0; i < 2; i++)
		buffer_push_color(current_buffer, r, g, b, alpha);
}

void display_draw_triangle(float x1, float y1, float x2, float y2, float x3, float y3)
{
	Buffer *current_buffer = display.current_buffer;
	float r = display.r;
	float g = display.g;
	float b = display.b;
	float alpha = display.alpha;
	int i;

	if (display.debug_mode) {
		display_draw_line(x1, y1, x2, y2);
		display_draw_line(x2, y2, x3, y3);
		display_draw_line(x3, y3, x1, y1);
		return;
	}

	buffer_check_type(current_buffer, TRIANGLE_BUFFER);
	buffer_check_not_use_texture(current_buffer);
	buffer_check_not_full(current_buffer);

	buffer_push_vertex(current_buffer, x1, y1);
	buffer_push_vertex(current_buffer, x2, y2);
	buffer_push_vertex(current_buffer, x3, y3);
	for (i = 0; i < 3; i++)
		buffer_push_color(current_buffer, r, g, b, alpha);
}

void display_draw_surface(float xi1, float yi1, float xi2, float yi2, float xi3, float yi3,
                          float xo1, float yo1, float xo2, float yo2, float xo3, float yo3)
{
	Buffer *current_buffer = display.current_buffer;
	float r = display.r;
	float g = display.g;
	float b = display.b;
	float alpha = display.alpha;
	int i;
	float xxi1, xxi2, xxi3;
	float yyi1, yyi2, yyi3;

	if (display.debug_mode) {
		display_draw_line(xo1, yo1, xo2, yo2);
		display_draw_line(xo2, yo2, xo3, yo3);
		display_draw_line(xo3, yo3, xo1, yo1);
		return;
	}

	assert(display.current_from);

	display_convert_texcoords(xi1, yi1, &xxi1, &yyi1);
	display_convert_texcoords(xi2, yi2, &xxi2, &yyi2);
	display_convert_texcoords(xi3, yi3, &xxi3, &yyi3);

	buffer_check_type(current_buffer, TRIANGLE_BUFFER);
	buffer_check_use_texture(current_buffer);
	buffer_check_not_full(current_buffer);

	buffer_push_tex_coord(current_buffer, xxi1, yyi1);
	buffer_push_tex_coord(current_buffer, xxi2, yyi2);
	buffer_push_tex_coord(current_buffer, xxi3, yyi3);

	buffer_push_vertex(current_buffer, xo1, yo1);
	buffer_push_vertex(current_buffer, xo2, yo2);
	buffer_push_vertex(current_buffer, xo3, yo3);

	for (i = 0; i < 3; i++)
		buffer_push_color(current_buffer, r, g, b, alpha);
}

void display_draw_quad(float xi1, float yi1, float xi2, float yi2, float xi3, float yi3, float xi4, float yi4,
                       float xo1, float yo1, float xo2, float yo2, float xo3, float yo3, float xo4, float yo4)
{
	display_draw_surface(xi1, yi1, xi2, yi2, xi3, yi3, xo1, yo1, xo2, yo2, xo3, yo3);
	display_draw_surface(xi1, yi1, xi3, yi3, xi4, yi4, xo1, yo1, xo3, yo3, xo4, yo4);
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
		error = new(char, length);

		if (glIsShader(obj))
			glGetShaderInfoLog(obj, length, NULL, error);
		else
			glGetProgramInfoLog(obj, length, NULL, error);
	}

	return error;
}

Shader * display_new_shader(const char* strvert, const char* strfragcolor, const char* strfragtex, char** error)
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
	glBindAttribLocation(prog_color, ATTR_LOCATION_POSITION, "position");
	glBindAttribLocation(prog_color, ATTR_LOCATION_COLOR, "color");
	glBindAttribLocation(prog_color, ATTR_LOCATION_TEXCOORD, "texCoord");
	glBindAttribLocation(prog_color, ATTR_LOCATION_POINTSIZE, "pointSize");
	glAttachShader(prog_color, vert);
	glAttachShader(prog_color, frag_color);
	glLinkProgram(prog_color);

	prog_tex = glCreateProgram();
	assert(prog_tex);
	glBindAttribLocation(prog_tex, ATTR_LOCATION_POSITION, "position");
	glBindAttribLocation(prog_tex, ATTR_LOCATION_COLOR, "color");
	glBindAttribLocation(prog_tex, ATTR_LOCATION_TEXCOORD, "texCoord");
	glBindAttribLocation(prog_tex, ATTR_LOCATION_POINTSIZE, "pointSize");
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

	return shader_new(prog_color, prog_tex, vert, frag_color, frag_tex);
}

void display_use_shader(Shader* shader)
{
	assert(shader);

	buffer_check_empty(display.current_buffer);

	display.current_shader = shader;
	buffer_use_shader(display.current_buffer, shader);
}

void display_use_default_shader()
{
	display_use_shader(display.default_shader);
}

void display_free_shader(Shader *shader)
{
	if (!shader)
		return;

	if (shader != display.default_shader) {
		GLuint prog;
		glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&prog);
		if (prog == shader->prog_color || prog == shader->prog_tex)
			display_use_default_shader();
	}

	shader_free(shader);
}

Buffer *display_new_buffer(unsigned int size)
{
	Buffer* buffer = buffer_new(true, size);
	buffer_allocate(buffer);
	buffer_use_camera(buffer, display.camera);

	return buffer;
}

void display_use_buffer(Buffer* buffer)
{
	assert(buffer);

	display.current_buffer = buffer;
	buffer_use_shader(buffer, display.current_shader);
	buffer->draw_on = display.current_on;
}

void display_use_default_buffer()
{
	display_use_buffer(display.default_buffer);
}

void display_draw_buffer(Buffer* buffer, float dx, float dy)
{
	assert(buffer);

	buffer_check_empty(display.current_buffer);
	buffer->draw_on = display.current_on;
	buffer_draw(buffer, dx, dy);
}

