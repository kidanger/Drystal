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
#ifndef EMSCRIPTEN
#include <SDL2/SDL.h>
#else
#include <SDL/SDL.h>
#include <html5.h>
#endif

#include <map>
#include <lua.hpp>

#include "macro.hpp"
#include "engine.hpp"
#include "lua_util.hpp"
#include "event.hpp"
#include "log.hpp"

log_category("event");

typedef Sint32 SDL_Keycode;

std::map<SDL_Keycode, const char*> keynames;

/** from https://code.google.com/r/kyberneticist-webport/source/browse/project_files/web_exp/pas2c_build/emcc/patches/sdl_patch.c */
static int initKeys()
{
	keynames[SDLK_BACKSPACE] = "backspace";
	keynames[SDLK_TAB] = "tab";
	keynames[SDLK_CLEAR] = "clear";
	keynames[SDLK_RETURN] = "return";
	keynames[SDLK_PAUSE] = "pause";
	keynames[SDLK_ESCAPE] = "escape";
	keynames[SDLK_SPACE] = "space";
	keynames[SDLK_EXCLAIM]  = "!";
	keynames[SDLK_QUOTEDBL]  = "\"";
	keynames[SDLK_HASH]  = "#";
	keynames[SDLK_DOLLAR]  = "$";
	keynames[SDLK_AMPERSAND]  = "&";
	keynames[SDLK_QUOTE] = "'";
	keynames[SDLK_LEFTPAREN] = "(";
	keynames[SDLK_RIGHTPAREN] = ")";
	keynames[SDLK_ASTERISK] = "*";
	keynames[SDLK_PLUS] = "+";
	keynames[SDLK_COMMA] = ",";
	keynames[SDLK_MINUS] = "-";
	keynames[SDLK_PERIOD] = ".";
	keynames[SDLK_SLASH] = "/";
	keynames[SDLK_0] = "0";
	keynames[SDLK_1] = "1";
	keynames[SDLK_2] = "2";
	keynames[SDLK_3] = "3";
	keynames[SDLK_4] = "4";
	keynames[SDLK_5] = "5";
	keynames[SDLK_6] = "6";
	keynames[SDLK_7] = "7";
	keynames[SDLK_8] = "8";
	keynames[SDLK_9] = "9";
	keynames[SDLK_COLON] = ":";
	keynames[SDLK_SEMICOLON] = ";";
	keynames[SDLK_LESS] = "<";
	keynames[SDLK_EQUALS] = "=";
	keynames[SDLK_GREATER] = ">";
	keynames[SDLK_QUESTION] = "?";
	keynames[SDLK_AT] = "@";
	keynames[SDLK_LEFTBRACKET] = "[";
	keynames[SDLK_BACKSLASH] = "\\";
	keynames[SDLK_RIGHTBRACKET] = "]";
	keynames[SDLK_CARET] = "^";
	keynames[SDLK_UNDERSCORE] = "_";
	keynames[SDLK_BACKQUOTE] = "`";
	keynames[SDLK_a] = "a";
	keynames[SDLK_b] = "b";
	keynames[SDLK_c] = "c";
	keynames[SDLK_d] = "d";
	keynames[SDLK_e] = "e";
	keynames[SDLK_f] = "f";
	keynames[SDLK_g] = "g";
	keynames[SDLK_h] = "h";
	keynames[SDLK_i] = "i";
	keynames[SDLK_j] = "j";
	keynames[SDLK_k] = "k";
	keynames[SDLK_l] = "l";
	keynames[SDLK_m] = "m";
	keynames[SDLK_n] = "n";
	keynames[SDLK_o] = "o";
	keynames[SDLK_p] = "p";
	keynames[SDLK_q] = "q";
	keynames[SDLK_r] = "r";
	keynames[SDLK_s] = "s";
	keynames[SDLK_t] = "t";
	keynames[SDLK_u] = "u";
	keynames[SDLK_v] = "v";
	keynames[SDLK_w] = "w";
	keynames[SDLK_x] = "x";
	keynames[SDLK_y] = "y";
	keynames[SDLK_z] = "z";
	keynames[SDLK_DELETE] = "delete";

	keynames[SDLK_KP_0] = "[0]";
	keynames[SDLK_KP_1] = "[1]";
	keynames[SDLK_KP_2] = "[2]";
	keynames[SDLK_KP_3] = "[3]";
	keynames[SDLK_KP_4] = "[4]";
	keynames[SDLK_KP_5] = "[5]";
	keynames[SDLK_KP_6] = "[6]";
	keynames[SDLK_KP_7] = "[7]";
	keynames[SDLK_KP_8] = "[8]";
	keynames[SDLK_KP_9] = "[9]";
	keynames[SDLK_KP_PERIOD] = "[.]";
	keynames[SDLK_KP_DIVIDE] = "[/]";
	keynames[SDLK_KP_MULTIPLY] = "[*]";
	keynames[SDLK_KP_MINUS] = "[-]";
	keynames[SDLK_KP_PLUS] = "[+]";
	keynames[SDLK_KP_ENTER] = "enter";
	keynames[SDLK_KP_EQUALS] = "equals";

	keynames[SDLK_UP] = "up";
	keynames[SDLK_DOWN] = "down";
	keynames[SDLK_RIGHT] = "right";
	keynames[SDLK_LEFT] = "left";
	keynames[SDLK_DOWN] = "down";
	keynames[SDLK_INSERT] = "insert";
	keynames[SDLK_HOME] = "home";
	keynames[SDLK_END] = "end";
	keynames[SDLK_PAGEUP] = "page up";
	keynames[SDLK_PAGEDOWN] = "page down";

	keynames[SDLK_F1] = "f1";
	keynames[SDLK_F2] = "f2";
	keynames[SDLK_F3] = "f3";
	keynames[SDLK_F4] = "f4";
	keynames[SDLK_F5] = "f5";
	keynames[SDLK_F6] = "f6";
	keynames[SDLK_F7] = "f7";
	keynames[SDLK_F8] = "f8";
	keynames[SDLK_F9] = "f9";
	keynames[SDLK_F10] = "f10";
	keynames[SDLK_F11] = "f11";
	keynames[SDLK_F12] = "f12";
	keynames[SDLK_F13] = "f13";
	keynames[SDLK_F14] = "f14";
	keynames[SDLK_F15] = "f15";

	keynames[SDLK_CAPSLOCK] = "caps lock";
	keynames[SDLK_RSHIFT] = "right shift";
	keynames[SDLK_LSHIFT] = "left shift";
	keynames[SDLK_RCTRL] = "right ctrl";
	keynames[SDLK_LCTRL] = "left ctrl";
	keynames[SDLK_RALT] = "right alt";
	keynames[SDLK_LALT] = "left alt";
	keynames[SDLK_RGUI] = "right meta";
	keynames[SDLK_LGUI] = "left meta";
	keynames[SDLK_MODE] = "alt gr";
	return 0;
}
static _unused_ int unused = initKeys();

static const char * mySDL_GetKeyName(SDL_Keycode key)
{
	const char *keyname;

	keyname = keynames[key];
	if (keyname == NULL) {
		keyname = "unknown key";
	}
	return keyname;
}

static void call_mouse_motion(int mx, int my, int dx, int dy)
{
	Engine& engine = get_engine();
	if (engine.lua.get_function("mouse_motion")) {
		lua_State* L = engine.lua.L;
		lua_pushnumber(L, mx);
		lua_pushnumber(L, my);
		lua_pushnumber(L, dx);
		lua_pushnumber(L, dy);
		CALL(4, 0);
	}
}

static void call_mouse_press(int mx, int my, Button button)
{
	Engine& engine = get_engine();
	if (engine.lua.get_function("mouse_press")) {
		lua_State* L = engine.lua.L;
		lua_pushnumber(L, mx);
		lua_pushnumber(L, my);
		lua_pushnumber(L, button);
		CALL(3, 0);
	}
}

static void call_mouse_release(int mx, int my, Button button)
{
	Engine& engine = get_engine();
	if (engine.lua.get_function("mouse_release")) {
		lua_State* L = engine.lua.L;
		lua_pushnumber(L, mx);
		lua_pushnumber(L, my);
		lua_pushnumber(L, button);
		CALL(3, 0);
	}
}

static void call_key_press(const char* key_string)
{
	assert(key_string);

	Engine& engine = get_engine();
	if (engine.lua.get_function("key_press")) {
		lua_State* L = engine.lua.L;
		lua_pushstring(L, key_string);
		CALL(1, 0);
	}
}

static void call_key_release(const char* key_string)
{
	assert(key_string);

	Engine& engine = get_engine();
	if (engine.lua.get_function("key_release")) {
		lua_State* L = engine.lua.L;
		lua_pushstring(L, key_string);
		CALL(1, 0);
	}
}

static void call_key_text(const char* string)
{
	assert(string);

	Engine& engine = get_engine();
	if (engine.lua.get_function("key_text")) {
		lua_State* L = engine.lua.L;
		lua_pushstring(L, string);
		CALL(1, 0);
	}
}

static void call_resize_event(int w, int h)
{
	Engine& engine = get_engine();
	if (engine.lua.get_function("resize_event")) {
		lua_State* L = engine.lua.L;
		lua_pushnumber(L, w);
		lua_pushnumber(L, h);
		CALL(2, 0);
	}
}

static void handle_event(const SDL_Event& event)
{
	Engine& engine = get_engine();
	Button button;
	switch (event.type) {
		case SDL_QUIT:
			engine.stop();
			break;
		case SDL_KEYUP:
			call_key_release(mySDL_GetKeyName(event.key.keysym.sym));
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_F1) {
				engine.toggle_update();
			} else if (event.key.keysym.sym == SDLK_F2) {
				engine.toggle_draw();
			} else if (event.key.keysym.sym == SDLK_F3) {
				engine.lua.reload_code();
			} else if (event.key.keysym.sym == SDLK_F4) {
				engine.display.toggle_debug_mode();
			} else if (event.key.keysym.sym == SDLK_F10) {
				engine.toggle_stats();
			} else {
				call_key_press(mySDL_GetKeyName(event.key.keysym.sym));
			}
			break;
		case SDL_TEXTINPUT:
			call_key_text(event.edit.text);
			break;
		case SDL_MOUSEMOTION:
			call_mouse_motion(event.motion.x, event.motion.y,
			                  event.motion.xrel, event.motion.yrel);
			break;
		case SDL_MOUSEBUTTONDOWN:
			button = static_cast<Button>(event.button.button);
			if (button != BUTTON_LEFT && button != BUTTON_RIGHT && button != BUTTON_MIDDLE) {
				// On SDL1 the mouse wheel up/down will be assigned to BUTTON_X1/X2 as well,
				// so to be consistant on native and web build we must consider them as wheel up/down only
				// and ignore all press/release except for left/middle/right button
				break;
			}
			call_mouse_press(event.button.x, event.button.y, button);
			break;
		case SDL_MOUSEBUTTONUP:
			button = static_cast<Button>(event.button.button);
			if (button != BUTTON_LEFT && button != BUTTON_RIGHT && button != BUTTON_MIDDLE) {
				break;
			}
			call_mouse_release(event.button.x, event.button.y, button);
			break;
		case SDL_MOUSEWHEEL: {
			if (event.button.button > 0) {
				break;
			}
			int x, y;
			SDL_GetMouseState(&x, &y);
			Button button = event.wheel.y > 0 ? WHEEL_UP : WHEEL_DOWN;
			call_mouse_press(x, y, button);
			call_mouse_release(x, y, button);
		}
		break;
#ifndef EMSCRIPTEN
		case SDL_WINDOWEVENT_RESIZED:
			call_resize_event(event.window.data1, event.window.data2);
			break;
#else
		case SDL_VIDEORESIZE:
			call_resize_event(event.resize.w, event.resize.h);
			break;
#endif
		default:
			break;
	}
}

void event_update()
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		handle_event(event);
	}
}

#ifdef EMSCRIPTEN
static EM_BOOL em_ui_callback(int eventType, const EmscriptenUiEvent *uiEvent, void *userData)
{
	call_resize_event(uiEvent->windowInnerWidth, uiEvent->windowInnerHeight);
	return false;
}
static EM_BOOL em_click_callback(int eventType, const EmscriptenMouseEvent *keyEvent, void *userData)
{
	// just define it so emscripten's html5 lib can detect click
	// and request pointer lock
	return false;
}
#endif

void event_init(void)
{
#ifndef EMSCRIPTEN
	int err = SDL_InitSubSystem(SDL_INIT_EVENTS);
	if (err) {
		log_error("Cannot initialize SDL events subsystem");
		return;
	}
#else
	emscripten_set_resize_callback(NULL, NULL, true, em_ui_callback);
	emscripten_set_click_callback(NULL, NULL, true, em_click_callback);
#endif
	SDL_StartTextInput();
}

void event_destroy(void)
{
#ifndef EMSCRIPTEN
	SDL_QuitSubSystem(SDL_INIT_EVENTS);
#endif
}

void event_set_relative_mode(bool relative)
{
#ifdef EMSCRIPTEN
	if (relative) {
		emscripten_request_pointerlock(NULL, true);
	} else {
		emscripten_exit_pointerlock();
	}
#else
	SDL_SetRelativeMouseMode(relative ? SDL_TRUE : SDL_FALSE);
#endif
}
