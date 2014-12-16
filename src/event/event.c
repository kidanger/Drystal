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

#include <lua.h>

#include "macro.h"
#include "engine.h"
#include "graphics/display.h"
#include "lua_util.h"
#include "event.h"
#include "log.h"
#include "dlua.h"

log_category("event");

static int keys_table_ref;

#define TRANSFORM(key) ((((key) >> 16) | (key)) & 0xffff)

static void initialize_keys_mapping(void)
{
	lua_State *L = dlua_get_lua_state();

	lua_createtable(L, 0, 118);
#define ADD_KEY(key, keyname) \
	lua_pushnumber(L, TRANSFORM(key)); \
	lua_pushliteral(L, keyname); \
	lua_settable(L, -3) \

	/** from https://code.google.com/r/kyberneticist-webport/source/browse/project_files/web_exp/pas2c_build/emcc/patches/sdl_patch.c */
	ADD_KEY(SDLK_BACKSPACE, "backspace");
	ADD_KEY(SDLK_TAB, "tab");
	ADD_KEY(SDLK_CLEAR, "clear");
	ADD_KEY(SDLK_RETURN, "return");
	ADD_KEY(SDLK_PAUSE, "pause");
	ADD_KEY(SDLK_ESCAPE, "escape");
	ADD_KEY(SDLK_SPACE, "space");
	ADD_KEY(SDLK_EXCLAIM,  "!");
	ADD_KEY(SDLK_QUOTEDBL, "\"");
	ADD_KEY(SDLK_HASH, "#");
	ADD_KEY(SDLK_DOLLAR, "$");
	ADD_KEY(SDLK_AMPERSAND, "&");
	ADD_KEY(SDLK_QUOTE, "'");
	ADD_KEY(SDLK_LEFTPAREN, "(");
	ADD_KEY(SDLK_RIGHTPAREN, ")");
	ADD_KEY(SDLK_ASTERISK, "*");
	ADD_KEY(SDLK_PLUS, "+");
	ADD_KEY(SDLK_COMMA, ",");
	ADD_KEY(SDLK_MINUS, "-");
	ADD_KEY(SDLK_PERIOD, ".");
	ADD_KEY(SDLK_SLASH, "/");
	ADD_KEY(SDLK_0, "0");
	ADD_KEY(SDLK_1, "1");
	ADD_KEY(SDLK_2, "2");
	ADD_KEY(SDLK_3, "3");
	ADD_KEY(SDLK_4, "4");
	ADD_KEY(SDLK_5, "5");
	ADD_KEY(SDLK_6, "6");
	ADD_KEY(SDLK_7, "7");
	ADD_KEY(SDLK_8, "8");
	ADD_KEY(SDLK_9, "9");
	ADD_KEY(SDLK_COLON, ":");
	ADD_KEY(SDLK_SEMICOLON, ";");
	ADD_KEY(SDLK_LESS, "<");
	ADD_KEY(SDLK_EQUALS, "=");
	ADD_KEY(SDLK_GREATER, ">");
	ADD_KEY(SDLK_QUESTION, "?");
	ADD_KEY(SDLK_AT, "@");
	ADD_KEY(SDLK_LEFTBRACKET, "[");
	ADD_KEY(SDLK_BACKSLASH, "\\");
	ADD_KEY(SDLK_RIGHTBRACKET, "]");
	ADD_KEY(SDLK_CARET, "^");
	ADD_KEY(SDLK_UNDERSCORE, "_");
	ADD_KEY(SDLK_BACKQUOTE, "`");
	ADD_KEY(SDLK_a, "a");
	ADD_KEY(SDLK_b, "b");
	ADD_KEY(SDLK_c, "c");
	ADD_KEY(SDLK_d, "d");
	ADD_KEY(SDLK_e, "e");
	ADD_KEY(SDLK_f, "f");
	ADD_KEY(SDLK_g, "g");
	ADD_KEY(SDLK_h, "h");
	ADD_KEY(SDLK_i, "i");
	ADD_KEY(SDLK_j, "j");
	ADD_KEY(SDLK_k, "k");
	ADD_KEY(SDLK_l, "l");
	ADD_KEY(SDLK_m, "m");
	ADD_KEY(SDLK_n, "n");
	ADD_KEY(SDLK_o, "o");
	ADD_KEY(SDLK_p, "p");
	ADD_KEY(SDLK_q, "q");
	ADD_KEY(SDLK_r, "r");
	ADD_KEY(SDLK_s, "s");
	ADD_KEY(SDLK_t, "t");
	ADD_KEY(SDLK_u, "u");
	ADD_KEY(SDLK_v, "v");
	ADD_KEY(SDLK_w, "w");
	ADD_KEY(SDLK_x, "x");
	ADD_KEY(SDLK_y, "y");
	ADD_KEY(SDLK_z, "z");
	ADD_KEY(SDLK_DELETE, "delete");

	ADD_KEY(SDLK_KP_0, "[0]");
	ADD_KEY(SDLK_KP_1, "[1]");
	ADD_KEY(SDLK_KP_2, "[2]");
	ADD_KEY(SDLK_KP_3, "[3]");
	ADD_KEY(SDLK_KP_4, "[4]");
	ADD_KEY(SDLK_KP_5, "[5]");
	ADD_KEY(SDLK_KP_6, "[6]");
	ADD_KEY(SDLK_KP_7, "[7]");
	ADD_KEY(SDLK_KP_8, "[8]");
	ADD_KEY(SDLK_KP_9, "[9]");
	ADD_KEY(SDLK_KP_PERIOD, "[.]");
	ADD_KEY(SDLK_KP_DIVIDE, "[/]");
	ADD_KEY(SDLK_KP_MULTIPLY, "[*]");
	ADD_KEY(SDLK_KP_MINUS, "[-]");
	ADD_KEY(SDLK_KP_PLUS, "[+]");
	ADD_KEY(SDLK_KP_ENTER, "enter");
	ADD_KEY(SDLK_KP_EQUALS, "equals");

	ADD_KEY(SDLK_UP, "up");
	ADD_KEY(SDLK_DOWN, "down");
	ADD_KEY(SDLK_RIGHT, "right");
	ADD_KEY(SDLK_LEFT, "left");
	ADD_KEY(SDLK_DOWN, "down");
	ADD_KEY(SDLK_INSERT, "insert");
	ADD_KEY(SDLK_HOME, "home");
	ADD_KEY(SDLK_END, "end");
	ADD_KEY(SDLK_PAGEUP, "page up");
	ADD_KEY(SDLK_PAGEDOWN, "page down");

	ADD_KEY(SDLK_F1, "f1");
	ADD_KEY(SDLK_F2, "f2");
	ADD_KEY(SDLK_F3, "f3");
	ADD_KEY(SDLK_F4, "f4");
	ADD_KEY(SDLK_F5, "f5");
	ADD_KEY(SDLK_F6, "f6");
	ADD_KEY(SDLK_F7, "f7");
	ADD_KEY(SDLK_F8, "f8");
	ADD_KEY(SDLK_F9, "f9");
	ADD_KEY(SDLK_F10, "f10");
	ADD_KEY(SDLK_F11, "f11");
	ADD_KEY(SDLK_F12, "f12");
	ADD_KEY(SDLK_F13, "f13");
	ADD_KEY(SDLK_F14, "f14");
	ADD_KEY(SDLK_F15, "f15");

	ADD_KEY(SDLK_CAPSLOCK, "caps lock");
	ADD_KEY(SDLK_RSHIFT, "right shift");
	ADD_KEY(SDLK_LSHIFT, "left shift");
	ADD_KEY(SDLK_RCTRL, "right ctrl");
	ADD_KEY(SDLK_LCTRL, "left ctrl");
	ADD_KEY(SDLK_RALT, "right alt");
	ADD_KEY(SDLK_LALT, "left alt");
	ADD_KEY(SDLK_RGUI, "right meta");
	ADD_KEY(SDLK_LGUI, "left meta");
	ADD_KEY(SDLK_MODE, "alt gr");
#undef ADD_KEY
	keys_table_ref = luaL_ref(L, LUA_REGISTRYINDEX);
}

static void push_keyname(lua_State *L, SDL_Keycode key)
{
	assert(L);

	lua_rawgeti(L, LUA_REGISTRYINDEX, keys_table_ref);
	lua_pushnumber(L, TRANSFORM(key));
	lua_rawget(L, -2);
	if (lua_isstring(L, -1)) {
		lua_remove(L, lua_gettop(L) - 1);
	} else {
		lua_pop(L, 2);
		lua_pushliteral(L, "unknown key");
	}
}

static void call_mouse_motion(int mx, int my, int dx, int dy)
{
	if (dlua_get_function("mouse_motion")) {
		lua_State* L = dlua_get_lua_state();
		lua_pushnumber(L, mx);
		lua_pushnumber(L, my);
		lua_pushnumber(L, dx);
		lua_pushnumber(L, dy);
		CALL(4, 0);
	}
}

static void call_mouse_press(int mx, int my, Button button)
{
	if (dlua_get_function("mouse_press")) {
		lua_State* L = dlua_get_lua_state();
		lua_pushnumber(L, mx);
		lua_pushnumber(L, my);
		lua_pushnumber(L, button);
		CALL(3, 0);
	}
}

static void call_mouse_release(int mx, int my, Button button)
{
	if (dlua_get_function("mouse_release")) {
		lua_State* L = dlua_get_lua_state();
		lua_pushnumber(L, mx);
		lua_pushnumber(L, my);
		lua_pushnumber(L, button);
		CALL(3, 0);
	}
}

static void call_key_press(SDL_Keycode key)
{
	if (dlua_get_function("key_press")) {
		lua_State* L = dlua_get_lua_state();
		push_keyname(L, key);
		CALL(1, 0);
	}
}

static void call_key_release(SDL_Keycode key)
{
	if (dlua_get_function("key_release")) {
		lua_State* L = dlua_get_lua_state();
		push_keyname(L, key);
		CALL(1, 0);
	}
}

static void call_key_text(const char* string)
{
	assert(string);

	if (dlua_get_function("key_text")) {
		lua_State* L = dlua_get_lua_state();
		lua_pushstring(L, string);
		CALL(1, 0);
	}
}

static void call_resize_event(int w, int h)
{
	if (dlua_get_function("resize_event")) {
		lua_State* L = dlua_get_lua_state();
		lua_pushnumber(L, w);
		lua_pushnumber(L, h);
		CALL(2, 0);
	}
}

static void handle_event(SDL_Event event)
{
	Button button;
	switch (event.type) {
		case SDL_QUIT:
			engine_stop();
			break;
		case SDL_KEYUP:
			call_key_release(event.key.keysym.sym);
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_F1) {
				engine_toggle_update();
			} else if (event.key.keysym.sym == SDLK_F2) {
				engine_toggle_draw();
			} else if (event.key.keysym.sym == SDLK_F3) {
				dlua_reload_code();
			} else if (event.key.keysym.sym == SDLK_F4) {
				display_toggle_debug_mode();
			} else {
				call_key_press(event.key.keysym.sym);
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
			button = (Button) event.button.button;
			if (button != BUTTON_LEFT && button != BUTTON_RIGHT && button != BUTTON_MIDDLE) {
				// On SDL1 the mouse wheel up/down will be assigned to BUTTON_X1/X2 as well,
				// so to be consistant on native and web build we must consider them as wheel up/down only
				// and ignore all press/release except for left/middle/right button
				break;
			}
			call_mouse_press(event.button.x, event.button.y, button);
			break;
		case SDL_MOUSEBUTTONUP:
			button = (Button) event.button.button;
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
#ifdef EMSCRIPTEN
			Button button = event.wheel.y < 0 ? WHEEL_UP : WHEEL_DOWN;
#else
			Button button = event.wheel.y > 0 ? WHEEL_UP : WHEEL_DOWN;
#endif
			call_mouse_press(x, y, button);
			call_mouse_release(x, y, button);
		}
		break;
#ifndef EMSCRIPTEN
		case SDL_WINDOWEVENT:
			switch (event.window.event) {
				case SDL_WINDOWEVENT_RESIZED:
					call_resize_event(event.window.data1, event.window.data2);
					break;
				default:
					break;
			}
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

	initialize_keys_mapping();
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
