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

#include <cassert>
#ifdef BUILD_LIVECODING
#include <atomic>
#endif

struct lua_State;

class LuaFunctions
{
public:
	lua_State* L;
	int drystal_table_ref;

	LuaFunctions(const char *_filename);

	bool load_code();
	bool reload_code();

	bool call_init() const;
	void call_update(float dt) const;
	void call_draw() const;
	void call_atexit() const;
#ifdef BUILD_LIVECODING
	std::atomic<bool>& is_need_to_reload();
	void set_need_to_reload();
#endif

	bool get_function(const char* name) const;
	void free();

private:
	const char* filename;
#ifdef BUILD_LIVECODING
	std::atomic<bool> need_to_reload;
#endif
	bool library_loaded;

	LuaFunctions(const LuaFunctions&);
	LuaFunctions& operator=(const LuaFunctions&);
	void remove_userpackages() const;
	void register_modules();
};

