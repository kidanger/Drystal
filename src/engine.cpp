#include <lua.hpp>

#include "emscripten.h"

#include "engine.hpp"
#include "display.hpp"
#include "drawable.hpp"
#include "event.hpp"
#include "file.hpp"

static Engine* engine;

void Engine::setup(int target_fps)
{
	engine = this;
	this->display = new Display();
	this->event = new EventManager(*this);
	this->display->init();
	this->target_fps = target_fps;
	L = luaL_newstate();
	luaL_openlibs(L);
}

//
// LUA functions
//

static int mlua_engine_stop(lua_State*)
{
	engine->stop();
	return 0;
}

static int mlua_draw_sprite(lua_State* L)
{
#ifndef EMSCRIPTEN
	if (not lua_istable(L, -3))
		printf("sprite is not a table\n");
	if (not lua_isnumber(L, -2))
		printf("y is not a number\n");
	if (not lua_isnumber(L, -1))
		printf("x is not a number\n");
#endif
	Sprite sp;
	// get x
	lua_pushstring(L, "x");
	lua_gettable(L, -4);
	sp.x = lua_tointeger(L, -1);
	lua_pop(L, 1);
	// get y
	lua_pushstring(L, "y");
	lua_gettable(L, -4);
	sp.y = lua_tointeger(L, -1);
	lua_pop(L, 1);
	// get w
	lua_pushstring(L, "w");
	lua_gettable(L, -4);
	sp.w = lua_tointeger(L, -1);
	lua_pop(L, 1);
	// get h
	lua_pushstring(L, "h");
	lua_gettable(L, -4);
	sp.h = lua_tointeger(L, -1);
	lua_pop(L, 1);
	// unload table
	int destx = lua_tointeger(L, -2);
	int desty = lua_tointeger(L, -1);
	engine->display->draw(sp, destx, desty);
	return 0;
}

static int mlua_set_background(lua_State* L)
{
	int r = lua_tointeger(L, -3);
	int g = lua_tointeger(L, -2);
	int b = lua_tointeger(L, -1);
	r = r < 0 ? r = 0 : r;
	r = r > 255 ? r = 255 : r;
	g = g < 0 ? g = 0 : g;
	g = g > 255 ? g = 255 : g;
	b = b < 0 ? b = 0 : b;
	b = b > 255 ? b = 255 : b;
	engine->display->set_background(r, g, b);
	return 0;
}

static int mlua_show_cursor(lua_State* L)
{
	int show = lua_toboolean(L, -1);
	engine->display->show_cursor(show);
	return 0;
}
static int mlua_set_resizable(lua_State* L)
{
	int r = lua_toboolean(L, -1);
	engine->display->set_resizable(r);
	return 0;
}
static int mlua_resize(lua_State* L)
{
	int w = lua_tointeger(L, -2);
	int h = lua_tointeger(L, -1);
	engine->display->resize(w, h);
	return 0;
}
static int mlua_flip(lua_State*)
{
	engine->display->flip();
	return 0;
}
static int mlua_draw_background(lua_State*)
{
	engine->display->draw_background();
	return 0;
}

//
// LUA load
//

void Engine::send_globals()
{
	lua_pushcfunction(L, mlua_engine_stop);
	lua_setglobal(L, "engine_stop");

	lua_pushcfunction(L, mlua_draw_sprite);
	lua_setglobal(L, "draw_sprite");
	lua_pushcfunction(L, mlua_set_background);
	lua_setglobal(L, "set_background");
	lua_pushcfunction(L, mlua_show_cursor);
	lua_setglobal(L, "show_cursor");
	lua_pushcfunction(L, mlua_set_resizable);
	lua_setglobal(L, "set_resizable");
	lua_pushcfunction(L, mlua_resize);
	lua_setglobal(L, "resize");
	lua_pushcfunction(L, mlua_flip);
	lua_setglobal(L, "flip");
	lua_pushcfunction(L, mlua_draw_background);
	lua_setglobal(L, "draw_background");
}

void Engine::reload()
{
	std::string filename = "game.lua";
	if (last_load < last_modified(filename)) {
		send_globals();

		if (luaL_dofile(L, filename.c_str()))
		{
			luaL_error(L, "error running script: %s", lua_tostring(L, -1));
		}

		lua_getglobal(L, "init");
		lua_pcall(L, 0, 0, 0);

		last_load = last_modified(filename);
	}
}

//
// Main loop
//

void Engine::loop()
{
	emscripten_set_main_loop([]() { engine->update(); }, this->target_fps, true);
}

void Engine::update()
{
	static int tick = 0;
	event->pull();

	if (tick % 30 == 0)
		reload();

	lua_getglobal(L, "update");
	if (lua_isfunction(L, -1))
	{
		if (lua_pcall(L, 0, 0, 0))
			luaL_error(L, "error calling update: %s", lua_tostring(L, -1));
	}
	else
		lua_pop(L, 1);

	lua_getglobal(L, "draw");
	if (lua_isfunction(L, -1))
	{
		if (lua_pcall(L, 0, 0, 0))
			luaL_error(L, "error calling draw: %s", lua_tostring(L, -1));
	}
	else
		lua_pop(L, 1);
	tick += 1;
}

//
// Events
//

void Engine::mouse_motion(int mx, int my)
{
	lua_getglobal(L, "mouse_motion");
	if (not lua_isfunction(L, -1))
	{
		lua_pop(L, 1);
		return;
	}
	lua_pushnumber(L, mx);
	lua_pushnumber(L, my);
	if (lua_pcall(L, 2, 0, 0))
	{
		luaL_error(L, "error calling mouse_motion: %s", lua_tostring(L, -1));
	}
}

void Engine::mouse_press(int mx, int my, int button)
{
	lua_getglobal(L, "mouse_press");
	if (not lua_isfunction(L, -1))
	{
		lua_pop(L, 1);
		return;
	}
	lua_pushnumber(L, mx);
	lua_pushnumber(L, my);
	lua_pushnumber(L, button);
	if (lua_pcall(L, 3, 0, 0))
	{
		luaL_error(L, "error calling mouse_button: %s", lua_tostring(L, -1));
	}
}

void Engine::key_press(const char* key_string)
{
	lua_getglobal(L, "key_press");
	if (not lua_isfunction(L, -1))
	{
		lua_pop(L, 1);
		return;
	}
	lua_pushstring(L, key_string);
	if (lua_pcall(L, 1, 0, 0))
	{
		luaL_error(L, "error calling key_press: %s", lua_tostring(L, -1));
	}
}

void Engine::key_release(const char* key_string)
{
	lua_getglobal(L, "key_release");
	if (not lua_isfunction(L, -1))
	{
		lua_pop(L, 1);
		return;
	}
	lua_pushstring(L, key_string);
	if (lua_pcall(L, 1, 0, 0))
	{
		luaL_error(L, "error calling key_release: %s", lua_tostring(L, -1));
	}
}

void Engine::event_resize(int w, int h)
{
	lua_getglobal(L, "resize_event");
	if (not lua_isfunction(L, -1))
	{
		lua_pop(L, 1);
		return;
	}
	lua_pushnumber(L, w);
	lua_pushnumber(L, h);
	if (lua_pcall(L, 2, 0, 0))
	{
		luaL_error(L, "error calling resize_event: %s", lua_tostring(L, -1));
	}
}


//
// Clean
//

void Engine::clean_up()
{
	delete event;
	delete display;
	// lua_close(L);
}

void Engine::stop()
{
#ifndef EMSCRIPTEN
	clean_up();
#endif
	emscripten_cancel_main_loop();
}

