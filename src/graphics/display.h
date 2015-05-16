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
#pragma once

#define GL_GLEXT_PROTOTYPES
#ifndef EMSCRIPTEN
#include <SDL2/SDL_opengles2.h>
#else
#include <SDL/SDL_opengl.h>
#endif

#include <stdbool.h>

#include "buffer.h"
#include "surface.h"
#include "camera.h"
#include "shader.h"

typedef struct SDL_Surface SDL_Surface;
typedef struct SDL_Window SDL_Window;

enum BlendMode {
	BLEND_DEFAULT = 0,
	BLEND_ALPHA = 0,
	BLEND_ADD,
	BLEND_MULT,
};
typedef enum BlendMode BlendMode;

int display_init(void);
void display_free();

void display_set_title(const char *title);
void display_show_cursor(bool);
void display_resize(int w, int h);
void display_screen2scene(float x, float y, float * tx, float * ty);
void display_toggle_debug_mode();
void display_set_fullscreen(bool fullscreen);

void display_set_color(int red, int green, int blue);
void display_set_alpha(int a);

void display_get_color(int *red, int *green, int *blue);
void display_get_alpha(int *a);

void display_set_line_width(float width);
void display_set_blend_mode(BlendMode mode);
void display_set_filter(Surface* surface, FilterMode mode);
void display_get_pixel(Surface* surface, unsigned int x, unsigned int y,
		       int* red, int* green, int* blue, int* alpha);

Camera *display_get_camera();
void display_reset_camera();
void display_push_camera();
void display_pop_camera();
void display_set_camera_position(float dx, float dy);
void display_set_camera_angle(float angle);
void display_set_camera_zoom(float zoom);

Surface* display_get_screen();
Surface* display_create_surface(unsigned int w, unsigned int h, unsigned int texw, unsigned int texh, unsigned char* pixels);
Surface* display_new_surface(int w, int h, bool force_npot);
int display_load_surface(const char *filename, Surface **surface);
void display_free_surface(Surface *surface);

void display_draw_on(Surface *surface);
void display_draw_from(Surface *surface);
Surface *display_get_draw_on();
Surface *display_get_draw_from();

void display_draw_background();
void display_draw_point(float x, float y, float size);
void display_draw_point_tex(float sx, float sy, float x, float y, float size);
void display_draw_line(float x1, float y1, float x2, float y2);
void display_draw_triangle(float x1, float y1, float x2, float y2, float x3, float y3);
void display_draw_surface(float, float, float, float, float, float, float, float, float, float, float, float);
void display_draw_quad(float xi1, float yi1, float xi2, float yi2, float xi3, float yi3, float xi4, float yi4,
                       float xo1, float yo1, float xo2, float yo2, float xo3, float yo3, float xo4, float yo4);

Shader* display_new_shader(const char* strvert, const char* strfragcolor, const char* strfragtex, char** error);
void display_use_shader(Shader *shader);
void display_use_default_shader();
void display_free_shader(Shader *shader);

Buffer* display_new_buffer(unsigned int size);
static inline Buffer* display_new_auto_buffer(void)
{
	return display_new_buffer(BUFFER_DEFAULT_SIZE);
}
void display_use_buffer(Buffer *buffer);
void display_use_default_buffer();
void display_draw_buffer(Buffer *buffer, float dx, float dy);
Buffer *display_get_current_buffer(void);
void display_free_buffer(Buffer* buffer);

void display_flip();
bool display_is_debug();

