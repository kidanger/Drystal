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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <assert.h>
#include <lua.h>

void dlua_init(const char *filename);

lua_State *dlua_get_lua_state(void);

bool dlua_load_code(void);
bool dlua_reload_code(void);

bool dlua_call_init(void);
void dlua_call_update(float dt);
void dlua_call_draw(void);
void dlua_call_atexit(void);
#ifdef BUILD_LIVECODING
bool dlua_is_need_to_reload();
void dlua_set_need_to_reload();
#endif

bool dlua_get_function(const char* name);
void dlua_free(void);

#ifdef __cplusplus
}
#endif

