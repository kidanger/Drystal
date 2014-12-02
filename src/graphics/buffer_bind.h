/**
 * This file is part of Drystal.
 *
 * Drystal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option); any later version.
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

#include <lua.h>

#include "lua_util.h"
#include "buffer.h"

DECLARE_PUSHPOP(Buffer, buffer)

int mlua_new_buffer(lua_State* L);
int mlua_use_buffer(lua_State* L);
int mlua_use_default_buffer(lua_State* L);
int mlua_draw_buffer(lua_State* L);
int mlua_reset_buffer(lua_State* L);
int mlua_upload_and_free_buffer(lua_State* L);
int mlua_free_buffer(lua_State* L);

