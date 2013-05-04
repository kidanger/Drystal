#include "emscripten.h"

#include <lua.hpp>

#include "engine.hpp"
#include "display.hpp"
#include "drawable.hpp"
#include "event.hpp"
#include "gamestate.hpp"
#include "file.hpp"


void Engine::setup(int target_fps)
{
	this->display = new Display();
	this->event = new EventManager(*this);
	this->display->init();
	this->target_fps = target_fps;
	L = luaL_newstate();
	luaL_openlibs(L);
	reload();
}

static Engine* engine;
static void clb_update()
{
	engine->update();
}
static int mlua_engine_stop(lua_State*)
{
	engine->stop();
	return 0;
}

void Engine::loop()
{
	engine = this;
	emscripten_set_main_loop(clb_update, this->target_fps, true);
}

void Engine::send_globals()
{
	lua_pushcfunction(L, mlua_engine_stop);
	lua_setglobal(L, "engine_stop");
}

void Engine::reload()
{
	std::string filename = "game.lua";
	if (last_load < last_modified(filename)) {
		if (luaL_dofile(L, filename.c_str()))
		{
			luaL_error(L, "error running script: %s", lua_tostring(L, -1));
		}

		send_globals();

		lua_getglobal(L, "init");
		lua_pcall(L, 0, 0, 0);

		last_load = last_modified(filename);
	}
}

void Engine::update()
{
	event->pull();
	reload();

	game->update(*this);
	lua_getglobal(L, "update");
	if (lua_pcall(L, 0, 0, 0))
	{
		luaL_error(L, "error calling update: %s", lua_tostring(L, -1));
	}

	display->draw_start();
	game->draw(*display);
	lua_getglobal(L, "draw");
	if (lua_pcall(L, 0, 0, 0))
	{
		luaL_error(L, "error calling update: %s", lua_tostring(L, -1));
	}

	display->draw_end();
}

void Engine::mouse_motion(int mx, int my)
{
	lua_getglobal(L, "mouse_motion");
	if (not lua_isfunction(L, -1))
	{
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

void Engine::push_state(GameState* game)
{
	game->set_parent(this->game);
	this->game = game;
	game->preload();
	game->setup(*this);
}

void Engine::pop_state()
{
	this->game->clean(*this);
	this->game = this->game->get_parent();
}

void Engine::clean_up()
{
	delete event;
	delete display;
	lua_close(L);
}

void Engine::stop()
{
	printf("Stopping engine...\n");
#ifndef EMSCRIPTEN
	clean_up();
#endif
	emscripten_cancel_main_loop();
}

