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

int engine_init(const char *filename, unsigned int target_fps);
void engine_free(void);
void engine_load(void);
void engine_loop(void);
void engine_update(void);
bool engine_is_loaded(void);
void engine_stop(void);
void engine_toggle_update(void);
void engine_toggle_draw(void);
#ifdef BUILD_LIVECODING
void engine_wait_next_reload(void);
void engine_add_file_to_reloadqueue(const char* filename);
#endif

