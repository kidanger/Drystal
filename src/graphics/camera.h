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

#include <stdbool.h>

typedef struct Camera Camera;

struct Camera {
	float dx;
	float dy;
	float zoom;
	float angle;
	float matrix[4];
};

Camera *camera_new(void);
void camera_free(Camera *c);

void camera_update_matrix(Camera *c, int width, int height);
void camera_reset(Camera *c);
void camera_push(Camera *c);
void camera_pop(Camera *c);
bool camera_stack_is_full(void);
bool camera_stack_is_empty(void);

