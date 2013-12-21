#include <SDL2/SDL.h>

#include "event.hpp"
#include "engine.hpp"

typedef Sint32 SDL_Keycode;

#ifdef EMSCRIPTEN
static const char *keynames[0x800];

/** from https://code.google.com/r/kyberneticist-webport/source/browse/project_files/web_exp/pas2c_build/emcc/patches/sdl_patch.c */
static void initKeys()
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

	keynames[SDLK_KP0] = "[0]";
	keynames[SDLK_KP1] = "[1]";
	keynames[SDLK_KP2] = "[2]";
	keynames[SDLK_KP3] = "[3]";
	keynames[SDLK_KP4] = "[4]";
	keynames[SDLK_KP5] = "[5]";
	keynames[SDLK_KP6] = "[6]";
	keynames[SDLK_KP7] = "[7]";
	keynames[SDLK_KP8] = "[8]";
	keynames[SDLK_KP9] = "[9]";
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

	keynames[SDLK_NUMLOCK] = "numlock";
	keynames[SDLK_CAPSLOCK] = "caps lock";
	keynames[SDLK_SCROLLOCK] = "scroll lock";
	keynames[SDLK_RSHIFT] = "right shift";
	keynames[SDLK_LSHIFT] = "left shift";
	keynames[SDLK_RCTRL] = "right ctrl";
	keynames[SDLK_LCTRL] = "left ctrl";
	keynames[SDLK_RALT] = "right alt";
	keynames[SDLK_LALT] = "left alt";
	keynames[SDLK_RMETA] = "right meta";
	keynames[SDLK_LMETA] = "left meta";
	keynames[SDLK_LSUPER] = "left super";	/* "Windows" keys */
	keynames[SDLK_RSUPER] = "right super";
	keynames[SDLK_MODE] = "alt gr";
	keynames[SDLK_COMPOSE] = "compose";

	keynames[SDLK_HELP] = "help";
	keynames[SDLK_PRINT] = "print screen";
	keynames[SDLK_SYSREQ] = "sys req";
	keynames[SDLK_BREAK] = "break";
	keynames[SDLK_MENU] = "menu";
	keynames[SDLK_POWER] = "power";
	keynames[SDLK_EURO] = "euro";
	keynames[SDLK_UNDO] = "undo";
}
#endif

const char * mySDL_GetKeyName(SDL_Keycode key)
{
#ifdef EMSCRIPTEN
	const char *keyname;

	keyname = keynames[key];
	if (keyname == NULL) {
		keyname = "unknown key";
	}
	return keyname;
#else
	return SDL_GetKeyName(key);
#endif
}

EventManager::EventManager(Engine& eng) :
	engine(eng)
{
#ifdef EMSCRIPTEN
	initKeys();
	SDL_EnableKeyRepeat(0, 0);
	SDL_EnableUNICODE(SDL_ENABLE);
#endif
}

void EventManager::poll()
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		handle_event(event);
	}
}

void EventManager::handle_event(const SDL_Event& event)
{
	switch(event.type)
	{
		case SDL_QUIT:
			engine.stop();
			break;
		case SDL_KEYUP:
			engine.key_release(mySDL_GetKeyName(event.key.keysym.sym));
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
			} else {
				char str[2] = {0};
				engine.key_press(mySDL_GetKeyName(event.key.keysym.sym), str);
			}
			break;
		case SDL_MOUSEMOTION:
			engine.mouse_motion(event.motion.x, event.motion.y,
								event.motion.xrel, event.motion.yrel);
			break;
		case SDL_MOUSEBUTTONDOWN:
			engine.mouse_press(event.button.x, event.button.y, event.button.button);
			break;
		case SDL_MOUSEBUTTONUP:
			engine.mouse_release(event.button.x, event.button.y, event.button.button);
			break;
		case SDL_WINDOWEVENT_RESIZED:
			engine.resize_event(event.window.data1, event.window.data2);
			break;
		default:
			break;
	}
}
