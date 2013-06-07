#include <cstring>
#include <lua.hpp>
#include <ctime>
#include <sys/time.h> /* for usleep in set_main_loop */

#include "emscripten.h"

#include "engine.hpp"
#include "display.hpp"
#include "drawable.hpp"
#include "event.hpp"
#include "network.hpp"

#ifndef EMSCRIPTEN
#include "file.hpp"
#endif

long unsigned get_now();

static Engine* engine;

void Engine::setup(const char* filename, int target_fps)
{
	engine = this;
	this->filename = filename;
	this->display = new Display();
	this->event = new EventManager(*this);
	this->net = new Network(*this);
	this->display->init();
	this->target_fps = target_fps;
	L = luaL_newstate();
	luaL_openlibs(L);
	reload();
	last_update = get_now();
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
	engine->display->draw_sprite(sp, destx, desty);
	return 0;
}

static int mlua_set_color(lua_State* L)
{
#ifndef EMSCRIPTEN
	if (not lua_istable(L, -1))
	{
		printf("color is not a table\n");
		return 0;
	}
#endif
	lua_pushnil(L);
	lua_next(L, -2);
	int r = lua_tointeger(L, -1);
	lua_pop(L, 1);
	lua_next(L, -2);
	int g = lua_tointeger(L, -1);
	lua_pop(L, 1);
	lua_next(L, -2);
	int b = lua_tointeger(L, -1);
	lua_pop(L, 1);
	lua_pop(L, 1);
	engine->display->set_color(r, g, b);
	return 0;
}
static int mlua_push_offset(lua_State* L)
{
	int ox = lua_tointeger(L, -2);
	int oy = lua_tointeger(L, -1);
	engine->display->push_offset(ox, oy);
	return 0;
}
static int mlua_pop_offset(lua_State*)
{
	engine->display->pop_offset();
	return 0;
}
static int mlua_set_font(lua_State* L)
{
	const char * name = lua_tostring(L, -2);
	int size = lua_tointeger(L, -1);
	engine->display->set_font(name, size);
	return 0;
}
static int mlua_set_round(lua_State* L)
{
	int round = lua_tointeger(L, -1);
	engine->display->set_round(round);
	return 0;
}
static int mlua_set_fill(lua_State* L)
{
	int fill = lua_toboolean(L, -1);
	engine->display->set_fill(fill);
	return 0;
}
static int mlua_set_alpha(lua_State* L)
{
	int alpha = lua_tointeger(L, -1);
	engine->display->set_alpha(alpha);
	return 0;
}

static int mlua_text_size(lua_State* L)
{
	const char* str = lua_tostring(L, -1);
	int w, h;
	engine->display->text_size(str, &w, &h);
	lua_pushnumber(L, w);
	lua_pushnumber(L, h);
	return 2;
}
static int mlua_surface_size(lua_State* L)
{
	Surface* surface = (Surface *) lua_touserdata(L, -1);
	int w, h;
	engine->display->surface_size(surface, &w, &h);
	lua_pushnumber(L, w);
	lua_pushnumber(L, h);
	return 2;
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
	lua_pushlightuserdata(L, engine->display->get_screen());
	lua_setglobal(L, "screen");
	return 0;
}
static int mlua_flip(lua_State*)
{
	engine->display->flip();
	return 0;
}
static int mlua_load_surface(lua_State* L)
{
	const char * filename = lua_tostring(L, -1);
	void* surface = engine->display->load_surface(filename);
	lua_pushlightuserdata(L, surface);
	return 1;
}
static int mlua_new_surface(lua_State* L)
{
	int w = lua_tointeger(L, -2);
	int h = lua_tointeger(L, -1);
	void* surface = engine->display->new_surface(w, h);
	lua_pushlightuserdata(L, surface);
	return 1;
}
static int mlua_free_surface(lua_State* L)
{
	Surface* surface = (Surface*) lua_touserdata(L, -1);
	engine->display->free_surface(surface);
	return 0;
}
static int mlua_rotate_surface(lua_State* L)
{
	Surface* surface = (Surface*) lua_touserdata(L, -2);
	double angle = lua_tonumber(L, -1);
	engine->display->rotate_surface(surface, angle);
	return 0;
}
static int mlua_resize_surface(lua_State* L)
{
	Surface* surface = (Surface*) lua_touserdata(L, -3);
	int sx = lua_tointeger(L, -2);
	int sy = lua_tointeger(L, -1);
	engine->display->resize_surface(surface, sx, sy);
	return 0;
}
static int mlua_draw_on(lua_State* L)
{
	Surface* surface = (Surface*) lua_touserdata(L, -1);
	engine->display->draw_on(surface);
	return 0;
}
static int mlua_draw_from(lua_State* L)
{
	Surface* surface = (Surface*) lua_touserdata(L, -1);
	engine->display->draw_from(surface);
	return 0;
}

static int mlua_draw_background(lua_State*)
{
	engine->display->draw_background();
	return 0;
}
static int mlua_draw_rect(lua_State* L)
{
	int x = lua_tointeger(L, -4);
	int y = lua_tointeger(L, -3);
	int w = lua_tointeger(L, -2);
	int h = lua_tointeger(L, -1);
	engine->display->draw_rect(x, y, w, h);
	return 0;
}
static int mlua_draw_surface(lua_State* L)
{
	Surface* surface = (Surface*) lua_touserdata(L, -3);
	int x = lua_tointeger(L, -2);
	int y = lua_tointeger(L, -1);
	engine->display->draw_surface(surface, x, y);
	return 0;
}
static int mlua_text_surface(lua_State* L)
{
	const char* text = lua_tostring(L, -1);
	Surface* surface = engine->display->text_surface(text);
	lua_pushlightuserdata(L, surface);
	return 1;
}
static int mlua_draw_circle(lua_State* L)
{
	int x = lua_tointeger(L, -3);
	int y = lua_tointeger(L, -2);
	int r = lua_tointeger(L, -1);
	engine->display->draw_circle(x, y, r);
	return 0;
}
static int mlua_draw_arc(lua_State* L)
{
	int x = lua_tointeger(L, -5);
	int y = lua_tointeger(L, -4);
	int radius = lua_tointeger(L, -3);
	int r1 = lua_tointeger(L, -2);
	int r2 = lua_tointeger(L, -1);
	engine->display->draw_arc(x, y, radius, r1, r2);
	return 0;
}
static int mlua_draw_line(lua_State* L)
{
	int x = lua_tointeger(L, -4);
	int y = lua_tointeger(L, -3);
	int x2 = lua_tointeger(L, -2);
	int y2 = lua_tointeger(L, -1);
	engine->display->draw_line(x, y, x2, y2);
	return 0;
}

static int mlua_connect(lua_State* L)
{
	const char* host = lua_tostring(L, -2);
	int port = lua_tointeger(L, -1);
	bool ok = engine->net->connect(host, port);
	lua_pushnumber(L, ok);
	return 1;
}
static int mlua_send(lua_State* L)
{
	const char* message = lua_tostring(L, -1);
	engine->net->send(message, strlen(message));
	return 0;
}
static int mlua_disconnect(lua_State*)
{
	engine->net->disconnect();
	return 0;
}

//
// LUA load
//

void Engine::send_globals()
{
	lua_register(L, "engine_stop", mlua_engine_stop);
	lua_register(L, "show_cursor", mlua_show_cursor);

	lua_register(L, "set_resizable", mlua_set_resizable);
	lua_register(L, "resize", mlua_resize);
	lua_register(L, "flip", mlua_flip);

	lua_register(L, "load_surface", mlua_load_surface);
	lua_register(L, "new_surface", mlua_new_surface);
	lua_register(L, "free_surface", mlua_free_surface);
	lua_register(L, "rotate_surface", mlua_rotate_surface);
	lua_register(L, "resize_surface", mlua_resize_surface);
	lua_register(L, "draw_on", mlua_draw_on);
	lua_register(L, "draw_from", mlua_draw_from);

	lua_register(L, "draw_background", mlua_draw_background);
	lua_register(L, "draw_rect", mlua_draw_rect);
	lua_register(L, "draw_surface", mlua_draw_surface);
	lua_register(L, "draw_sprite", mlua_draw_sprite);
	lua_register(L, "draw_circle", mlua_draw_circle);
	lua_register(L, "draw_arc", mlua_draw_arc);
	lua_register(L, "draw_line", mlua_draw_line);

	lua_register(L, "push_offset", mlua_push_offset);
	lua_register(L, "pop_offset", mlua_pop_offset);
	lua_register(L, "set_color", mlua_set_color);
	lua_register(L, "set_alpha", mlua_set_alpha);
	lua_register(L, "set_font", mlua_set_font);
	lua_register(L, "set_round", mlua_set_round);
	lua_register(L, "set_fill", mlua_set_fill);

	lua_register(L, "text_surface", mlua_text_surface);
	lua_register(L, "text_size", mlua_text_size);
	lua_register(L, "surface_size", mlua_surface_size);

	lua_register(L, "connect", mlua_connect);
	lua_register(L, "send", mlua_send);
	lua_register(L, "disconnect", mlua_disconnect);
}

void Engine::reload()
{
#ifndef EMSCRIPTEN
	time_t last = last_modified(filename);
	if (last == 0)
	{
		fprintf(stderr, "file %s does not exist\n", filename);
	}
	if (last_load < last) {
#endif
		send_globals();

		if (luaL_dofile(L, filename))
		{
			luaL_error(L, "error running script: %s", lua_tostring(L, -1));
		}

		lua_getglobal(L, "init");
		if (lua_pcall(L, 0, 0, 0))
			luaL_error(L, "error calling init: %s", lua_tostring(L, -1));

#ifndef EMSCRIPTEN
		last_load = last;
	}
#endif
}

//
// Main loop
//

void Engine::loop()
{
	emscripten_set_main_loop([]() { engine->update(); }, this->target_fps, true);
}

long unsigned get_now() // in microsecond
{
	struct timeval stTimeVal;
	gettimeofday(&stTimeVal, NULL);
	return stTimeVal.tv_sec * 1000000ll + stTimeVal.tv_usec;
}

void Engine::update()
{
	unsigned long at_start = get_now();
	static int tick = 0;
	event->poll();
	net->poll();

#ifndef EMSCRIPTEN
	if (tick % 30 == 0)
		reload();
#endif

	double dt = (get_now() - last_update) / 1000;
	lua_getglobal(L, "update");
	if (lua_isfunction(L, -1))
	{
		lua_pushnumber(L, dt);
		if (lua_pcall(L, 1, 0, 0))
			luaL_error(L, "error calling update: %s", lua_tostring(L, -1));
	}
	else
		lua_pop(L, 1);
	last_update = get_now();

	lua_getglobal(L, "draw");
	if (lua_isfunction(L, -1))
	{
		if (lua_pcall(L, 0, 0, 0))
			luaL_error(L, "error calling draw: %s", lua_tostring(L, -1));
	}
	else
		lua_pop(L, 1);
	tick += 1;

#ifndef EMSCRIPTEN
	unsigned long now = get_now();
	if ((now - at_start)/1000 < 1000/target_fps)
	{
		long sleep_time = 1000/target_fps - (now - at_start)/1000;
		if (sleep_time > 0)
		{
			SDL_Delay(sleep_time);
		}
	}
#endif
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
		luaL_error(L, "error calling mouse_press: %s", lua_tostring(L, -1));
	}
}

void Engine::mouse_release(int mx, int my, int button)
{
	lua_getglobal(L, "mouse_release");
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
		luaL_error(L, "error calling mouse_release: %s", lua_tostring(L, -1));
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

void Engine::net_recv(const char* str)
{
	lua_getglobal(L, "receive");
	if (not lua_isfunction(L, -1))
	{
		lua_pop(L, 1);
		return;
	}
	lua_pushstring(L, str);
	if (lua_pcall(L, 1, 0, 0))
	{
		luaL_error(L, "error calling receive: %s", lua_tostring(L, -1));
	}
}

void Engine::net_connected()
{
	lua_getglobal(L, "connected");
	if (not lua_isfunction(L, -1))
	{
		lua_pop(L, 1);
		return;
	}
	if (lua_pcall(L, 0, 0, 0))
	{
		luaL_error(L, "error calling receive: %s", lua_tostring(L, -1));
	}
}
void Engine::net_disconnected()
{
	lua_getglobal(L, "disconnected");
	if (not lua_isfunction(L, -1))
	{
		lua_pop(L, 1);
		return;
	}
	if (lua_pcall(L, 0, 0, 0))
	{
		luaL_error(L, "error calling receive: %s", lua_tostring(L, -1));
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
	net->disconnect();
#ifndef EMSCRIPTEN
	clean_up();
#endif
	emscripten_cancel_main_loop();
}

