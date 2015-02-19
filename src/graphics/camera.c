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
#include <math.h>
#include <assert.h>

#include "camera.h"
#include "util.h"

Camera *camera_new(void)
{
	Camera *c = new0(Camera, 1);
	c->zoom = 1;

	return c;
}

void camera_free(Camera *c)
{
	if (!c)
		return;

	free(c);
}

void camera_update_matrix(Camera *c, int width, int height)
{
	float ratio;
	float angle;

	assert(c);
	assert(width > 0);
	assert(height > 0);

	angle = c->angle;
	ratio = (float) width / height;
	c->matrix[0] = cosf(angle);
	c->matrix[1] = sinf(angle) * ratio;
	c->matrix[2] = -sinf(angle) / ratio;
	c->matrix[3] = cosf(angle);
}

void camera_reset(Camera *c)
{
	assert(c);

	c->dx = 0;
	c->dy = 0;
	c->angle = 0;
	c->zoom = 1;
}

