#include <cstring>
#include <cmath>
#include <cassert>

#include <lua.hpp>

#include "engine.hpp"
#include "log.hpp"
#include "lua_functions.hpp"

#define DECLARE_FUNCTION(name) {#name, mlua_##name}

#define CALL(num_args) \
	if (lua_pcall(L, num_args, 0, 0)) { \
		luaL_error(L, "[ERROR] calling %s: %s", __func__, lua_tostring(L, -1)); \
	}

// used to access some engine's fields from lua callbacks
static Engine *engine;

static int luaopen_drystal(lua_State*); // defined at the end of this file

LuaFunctions::LuaFunctions(Engine& eng, const char *filename) :
	L(luaL_newstate()),
	drystal_table_ref(LUA_NOREF),
	filename(filename),
	library_loaded(false)
{
	engine = &eng;

	luaL_openlibs(L);
}

LuaFunctions::~LuaFunctions()
{
	luaL_unref(L, LUA_REGISTRYINDEX, drystal_table_ref);
	lua_close(L);
}

void LuaFunctions::add_search_path(const char* path)
{
	bool add_slash = path[strlen(path) - 1] != '/';
	// update path
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "path");
	lua_pushstring(L, ";");
	lua_pushstring(L, path);
	if (add_slash)
		lua_pushstring(L, "/");
	lua_pushstring(L, "?.lua");
	lua_concat(L, 4 + add_slash);
	lua_setfield(L, -2, "path");

	// update cpath
	lua_getfield(L, -1, "cpath");
	lua_pushstring(L, ";");
	lua_pushstring(L, path);
	if (add_slash)
		lua_pushstring(L, "/");
	lua_pushstring(L, "?.so");
	lua_concat(L, 4 + add_slash);
	lua_setfield(L, -2, "cpath");

	lua_pop(L, 1);
}

/**
 * Search for a function named 'name' in the drystal table.
 * Return true if found, and keep the function is the lua stack
 * Otherwise, return false (stack is cleaned as needed).
 */
bool LuaFunctions::get_function(lua_State* L, const char* name) const
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, drystal_table_ref);
	lua_getfield(L, -1, name);
	if (lua_isfunction(L, -1)) {
		return true;
	}
	lua_pop(L, 2);
	lua_getglobal(L, name); // fallback api, TOBEREMOVED
	if (lua_isfunction(L, -1)) {
		return true;
	}
	lua_pop(L, 1);
	return false;
}

void LuaFunctions::remove_userpackages(lua_State* L)
{
	printf("Removing old packages: ");
	const char* kept[] = {
		"_G",
		LUA_COLIBNAME,
		LUA_TABLIBNAME,
		LUA_IOLIBNAME,
		LUA_OSLIBNAME,
		LUA_STRLIBNAME,
		LUA_BITLIBNAME,
		LUA_MATHLIBNAME,
		LUA_DBLIBNAME,
		LUA_LOADLIBNAME,
		"drystal",
	};
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "loaded");
	lua_pushnil(L);
	while (lua_next(L, -2)) {
		bool remove = true;
		const char* name = lua_tostring(L, -2);
		for (unsigned long i = 0; i < sizeof(kept) / sizeof(const char*) && remove; i++) {
			remove = remove && strcmp(name, kept[i]);
		}
		if (remove) {
			lua_pushnil(L);
			printf("%s ", name);
			lua_setfield(L, -4, name);
		}
		lua_pop(L, 1);
	}
	printf("\n");
}

bool LuaFunctions::load_code()
{
	if (!library_loaded) {
		// add drystal lib
		luaL_requiref(L, "drystal", luaopen_drystal, 1); // as global
		lua_pop(L, 1);  /* remove lib */
		// then remove it from package.loaded, so the drystal.lua can be called if user wants
		lua_getglobal(L, "package");
		lua_getfield(L, -1, "loaded");
		lua_pushnil(L);
		lua_setfield(L, -2, "drystal");
		library_loaded = true;
	}

	if (luaL_dofile(L, filename)) {
		fprintf(stderr, "[ERROR] cannot run script: %s\n", lua_tostring(L, -1));
		return false;
	}

	return true;
}

bool LuaFunctions::reload_code()
{
	remove_userpackages(L);

	printf("Reloading code...\n");
	return load_code() && call_init();
}

bool LuaFunctions::call_init()
{
	if (!get_function(L, "init")) {
		fprintf(stderr, "[ERROR] cannot find init function in `%s'\n", filename);
		return false;
	} else if (lua_pcall(L, 0, 0, 0)) {
		fprintf(stderr, "[ERROR] cannot call init: %s\n", lua_tostring(L, -1));
		return false;
	}
	return true;
}

void LuaFunctions::call_mouse_motion(int mx, int my, int dx, int dy) const
{
	if (get_function(L, "mouse_motion")) {
		lua_pushnumber(L, mx);
		lua_pushnumber(L, my);
		lua_pushnumber(L, dx);
		lua_pushnumber(L, dy);
		CALL(4);
	}
}

void LuaFunctions::call_mouse_press(int mx, int my, int button) const
{
	if (get_function(L, "mouse_press")) {
		lua_pushnumber(L, mx);
		lua_pushnumber(L, my);
		lua_pushnumber(L, button);
		CALL(3);
	}
}

void LuaFunctions::call_mouse_release(int mx, int my, int button) const
{
	if (get_function(L, "mouse_release")) {
		lua_pushnumber(L, mx);
		lua_pushnumber(L, my);
		lua_pushnumber(L, button);
		CALL(3);
	}
}

void LuaFunctions::call_key_press(const char* key_string) const
{
	if (get_function(L, "key_press")) {
		lua_pushstring(L, key_string);
		CALL(1);
	}
}

void LuaFunctions::call_key_release(const char* key_string) const
{
	if (get_function(L, "key_release")) {
		lua_pushstring(L, key_string);
		CALL(1);
	}
}

void LuaFunctions::call_key_text(const char* string) const
{
	if (get_function(L, "key_text")) {
		lua_pushstring(L, string);
		CALL(1);
	}
}

void LuaFunctions::call_resize_event(int w, int h) const
{
	if (get_function(L, "resize_event")) {
		lua_pushnumber(L, w);
		lua_pushnumber(L, h);
		CALL(2);
	}
}

void LuaFunctions::call_update(float dt)
{
	if (get_function(L, "update")) {
		lua_pushnumber(L, dt);
		CALL(1);
	}
}
void LuaFunctions::call_draw()
{
	if (get_function(L, "draw")) {
		CALL(0);
	}
}

void LuaFunctions::call_atexit() const
{
	if (get_function(L, "atexit")) {
		CALL(0);
	}
}

static int mlua_stop(lua_State*)
{
	engine->stop();
	return 0;
}

static int mlua_reload(lua_State*)
{
	engine->lua.reload_code();
	return 0;
}

static int mlua_set_color(lua_State* L)
{
	int r = lua_tointeger(L, -3);
	int g = lua_tointeger(L, -2);
	int b = lua_tointeger(L, -1);
	engine->display.set_color(r, g, b);
	return 0;
}
static int mlua_set_alpha(lua_State* L)
{
	int alpha = lua_tointeger(L, 1);
	engine->display.set_alpha(alpha);
	return 0;
}
static int mlua_set_point_size(lua_State* L)
{
	int point_size = lua_tointeger(L, 1);
	engine->display.set_point_size(point_size);
	return 0;
}
static int mlua_set_line_width(lua_State* L)
{
	int width = lua_tointeger(L, 1);
	engine->display.set_line_width(width);
	return 0;
}
static int mlua_set_blend_mode(lua_State* L)
{
	BlendMode mode = static_cast<BlendMode>(luaL_checknumber(L, 1));
	engine->display.set_blend_mode(mode);
	return 0;
}
static int mlua_set_filter_mode(lua_State* L)
{
	FilterMode mode = static_cast<FilterMode>(luaL_checknumber(L, 1));
	engine->display.set_filter_mode(mode);
	return 0;
}

static int mlua_camera__newindex(lua_State* L)
{
	const char * name = luaL_checkstring(L, 2);
	if (!strcmp(name, "x")) {
		lua_Number dx = luaL_checknumber(L, 3);
		engine->display.set_camera_position(dx, engine->display.get_camera().dy);
	} else if (!strcmp(name, "y")) {
		lua_Number dy = luaL_checknumber(L, 3);
		engine->display.set_camera_position(engine->display.get_camera().dx, dy);
	} else if (!strcmp(name, "angle")) {
		lua_Number angle = luaL_checknumber(L, 3);
		engine->display.set_camera_angle(angle);
	} else if (!strcmp(name, "zoom")) {
		lua_Number zoom = luaL_checknumber(L, 3);
		engine->display.set_camera_zoom(zoom);
	} else {
		lua_rawset(L, 1);
	}
	return 0;
}
static int mlua_camera__index(lua_State* L)
{
	const char * name = luaL_checkstring(L, 2);
	if (!strcmp(name, "x")) {
		lua_Number dx = engine->display.get_camera().dx;
		lua_pushnumber(L, dx);
		return 1;
	} else if (!strcmp(name, "y")) {
		lua_Number dy = engine->display.get_camera().dy;
		lua_pushnumber(L, dy);
		return 1;
	} else if (!strcmp(name, "angle")) {
		lua_Number angle = engine->display.get_camera().angle;
		lua_pushnumber(L, angle);
		return 1;
	} else if (!strcmp(name, "zoom")) {
		lua_Number zoom = engine->display.get_camera().zoom;
		lua_pushnumber(L, zoom);
		return 1;
	}
	return 0;
}
static int mlua_camera_reset(lua_State*)
{
	engine->display.reset_camera();
	return 0;
}

static int mlua_show_cursor(lua_State* L)
{
	bool show = lua_toboolean(L, 1);
	engine->display.show_cursor(show);
	return 0;
}
static int mlua_set_relative_mode(lua_State* L)
{
	bool relative = lua_toboolean(L, 1);
	engine->event.set_relative_mode(relative);
	return 0;
}
static int mlua_resize(lua_State* L)
{
	int w = lua_tointeger(L, 1);
	int h = lua_tointeger(L, 2);
	engine->display.resize(w, h);

	// update screen
	lua_rawgeti(L, LUA_REGISTRYINDEX, engine->lua.drystal_table_ref);
	Surface* screen = engine->display.get_screen();
	if (screen)
		lua_pushlightuserdata(L, screen);
	else
		lua_pushnil(L);
	lua_setfield(L, -2, "screen");
	lua_pop(L, 1);

	return 0;
}
static int mlua_screen2scene(lua_State* L)
{
	lua_Number x = lua_tointeger(L, 1);
	lua_Number y = lua_tointeger(L, 2);
	float tx, ty;
	engine->display.screen2scene(x, y, &tx, &ty);
	lua_pushnumber(L, tx);
	lua_pushnumber(L, ty);
	return 2;
}
static int mlua_flip(lua_State*)
{
	return 0;
}

static int mlua_start_text(lua_State*)
{
	engine->event.start_text();
	return 0;
}

static int mlua_stop_text(lua_State*)
{
	engine->event.stop_text();
	return 0;
}

DECLARE_PUSHPOP(Surface, surface)

static int mlua_load_surface(lua_State* L)
{
	const char * filename = lua_tostring(L, -1);
	Surface* surface = engine->display.load_surface(filename);
	if (surface) {
		push_surface(L, surface);
		return 1;
	}
	return 0;
}
static int mlua_new_surface(lua_State* L)
{
	int w = lua_tointeger(L, -2);
	int h = lua_tointeger(L, -1);
	Surface* surface = engine->display.new_surface(w, h);
	push_surface(L, surface);
	return 1;
}
static int mlua_free_surface(lua_State* L)
{
	DEBUG("");
	Surface* surface = pop_surface(L, -1);
	engine->display.free_surface(surface);
	return 0;
}
static int mlua_surface_size(lua_State* L)
{
	Surface* surface = pop_surface(L, -1);
	int w, h;
	engine->display.surface_size(surface, &w, &h);
	lua_pushnumber(L, w);
	lua_pushnumber(L, h);
	return 2;
}
static int mlua_draw_on(lua_State* L)
{
	Surface* surface = pop_surface(L, -1);
	engine->display.draw_on(surface);
	return 0;
}
static int mlua_draw_from(lua_State* L)
{
	Surface* surface = pop_surface(L, -1);
	engine->display.draw_from(surface);
	return 0;
}

static int mlua_draw_background(lua_State*)
{
	engine->display.draw_background();
	return 0;
}
static int mlua_draw_point(lua_State* L)
{
	lua_Number x = lua_tonumber(L, 1);
	lua_Number y = lua_tonumber(L, 2);
	engine->display.draw_point(x, y);
	return 0;
}
static int mlua_draw_point_tex(lua_State* L)
{
	lua_Number xi = lua_tonumber(L, 1);
	lua_Number yi = lua_tonumber(L, 2);
	lua_Number xd = lua_tonumber(L, 3);
	lua_Number yd = lua_tonumber(L, 4);
	engine->display.draw_point_tex(xi, yi, xd, yd);
	return 0;
}
static int mlua_draw_line(lua_State* L)
{
	lua_Number x1 = lua_tonumber(L, 1);
	lua_Number y1 = lua_tonumber(L, 2);
	lua_Number x2 = lua_tonumber(L, 3);
	lua_Number y2 = lua_tonumber(L, 4);
	engine->display.draw_line(x1, y1, x2, y2);
	return 0;
}
static int mlua_draw_triangle(lua_State* L)
{
	lua_Number x1 = lua_tonumber(L, 1);
	lua_Number y1 = lua_tonumber(L, 2);
	lua_Number x2 = lua_tonumber(L, 3);
	lua_Number y2 = lua_tonumber(L, 4);
	lua_Number x3 = lua_tonumber(L, 5);
	lua_Number y3 = lua_tonumber(L, 6);
	engine->display.draw_triangle(x1, y1, x2, y2, x3, y3);
	return 0;
}
static int mlua_draw_surface(lua_State* L)
{
	lua_Number i1 = lua_tonumber(L, 1);
	lua_Number i2 = lua_tonumber(L, 2);
	lua_Number i3 = lua_tonumber(L, 3);
	lua_Number i4 = lua_tonumber(L, 4);
	lua_Number i5 = lua_tonumber(L, 5);
	lua_Number i6 = lua_tonumber(L, 6);
	lua_Number o1 = lua_tonumber(L, 7);
	lua_Number o2 = lua_tonumber(L, 8);
	lua_Number o3 = lua_tonumber(L, 9);
	lua_Number o4 = lua_tonumber(L, 10);
	lua_Number o5 = lua_tonumber(L, 11);
	lua_Number o6 = lua_tonumber(L, 12);
	engine->display.draw_surface(i1, i2, i3, i4, i5, i6,
	                             o1, o2, o3, o4, o5, o6);
	return 0;
}
static int mlua_draw_quad(lua_State* L)
{
	lua_Number i1 = lua_tonumber(L, 1);
	lua_Number i2 = lua_tonumber(L, 2);
	lua_Number i3 = lua_tonumber(L, 3);
	lua_Number i4 = lua_tonumber(L, 4);
	lua_Number i5 = lua_tonumber(L, 5);
	lua_Number i6 = lua_tonumber(L, 6);
	lua_Number i7 = lua_tonumber(L, 7);
	lua_Number i8 = lua_tonumber(L, 8);
	lua_Number o1 = lua_tonumber(L, 9);
	lua_Number o2 = lua_tonumber(L, 10);
	lua_Number o3 = lua_tonumber(L, 11);
	lua_Number o4 = lua_tonumber(L, 12);
	lua_Number o5 = lua_tonumber(L, 13);
	lua_Number o6 = lua_tonumber(L, 14);
	lua_Number o7 = lua_tonumber(L, 15);
	lua_Number o8 = lua_tonumber(L, 16);
	engine->display.draw_quad(i1, i2, i3, i4, i5, i6, i7, i8,
	                          o1, o2, o3, o4, o5, o6, o7, o8);
	return 0;
}

DECLARE_PUSHPOP(Shader, shader)

static int mlua_new_shader(lua_State* L)
{
	const char *vert = NULL, *frag_color = NULL, *frag_tex = NULL;
	if (lua_gettop(L) >= 1) { // one argument, it's the vertex shader
		vert = lua_tostring(L, 1);
	}
	if (lua_gettop(L) >= 2) {
		frag_color = lua_tostring(L, 2);
	}
	if (lua_gettop(L) >= 3) {
		frag_tex = lua_tostring(L, 3);
	}
	// null code will be set to defaut shader
	Shader* shader = engine->display.new_shader(vert, frag_color, frag_tex);
	if (shader) {
		push_shader(L, shader);
		return 1;
	}
	return 0; // returns nil
}
static int mlua_use_shader(lua_State* L)
{
	if (lua_gettop(L) == 0) { // use defaut shader
		engine->display.use_shader(NULL);
	} else {
		Shader* shader = pop_shader(L, -1);
		engine->display.use_shader(shader);
	}
	return 0;
}

static int mlua_feed_shader(lua_State* L)
{
	Shader* shader = pop_shader(L, 1);
	const char* name = lua_tostring(L, 2);
	float value = lua_tonumber(L, 3);
	engine->display.feed_shader(shader, name, value);
	return 0;
}
static int mlua_free_shader(lua_State* L)
{
	DEBUG("");
	Shader* shader = pop_shader(L, 1);
	engine->display.free_shader(shader);
	return 0;
}

DECLARE_PUSHPOP(Buffer, buffer)

static int mlua_new_buffer(lua_State* L)
{
	Buffer* buffer;
	if (lua_gettop(L) == 1) {
		lua_Number size = luaL_checknumber(L, 1);
		buffer = engine->display.new_buffer(size);
	} else {
		buffer = engine->display.new_buffer(); // let Display choose a size
	}
	if (buffer) {
		push_buffer(L, buffer);
		return 1;
	}
	return 0; // returns nil
}
static int mlua_use_buffer(lua_State* L)
{
	if (lua_gettop(L) == 0) { // use defaut buffer
		engine->display.use_buffer(NULL);
	} else {
		Buffer* buffer = pop_buffer(L, -1);
		engine->display.use_buffer(buffer);
	}
	return 0;
}
static int mlua_draw_buffer(lua_State* L)
{
	Buffer* buffer = pop_buffer(L, -1);
	lua_Number dx = 0, dy = 0;
	if (lua_gettop(L) >= 2)
		dx = luaL_checknumber(L, 2);
	if (lua_gettop(L) >= 3)
		dy = luaL_checknumber(L, 3);
	engine->display.draw_buffer(buffer, dx, dy);
	return 0;
}
static int mlua_reset_buffer(lua_State* L)
{
	Buffer* buffer = pop_buffer(L, -1);
	engine->display.reset_buffer(buffer);
	return 0;
}
static int mlua_upload_and_free_buffer(lua_State* L)
{
	Buffer* buffer = pop_buffer(L, -1);
	engine->display.upload_and_free_buffer(buffer);
	return 0;
}
static int mlua_free_buffer(lua_State* L)
{
	DEBUG("");
	Buffer* buffer = pop_buffer(L, -1);
	engine->display.free_buffer(buffer);
	return 0;
}

DECLARE_PUSHPOP(Sound, sound)
DECLARE_PUSHPOP(Music, music)

static int mlua_load_sound(lua_State *L)
{
	const char *filepath = lua_tostring(L, -1);
	Sound *chunk = engine->audio.load_sound(filepath);
	push_sound(L, chunk);
	return 1;
}

static int mlua_create_sound(lua_State *L)
{
	/*
	 * Multiple configurations allowed:
	 * [1]: table
	 * 	len = #table (can call __len)
	 * 	data = table[i] (can call __index)
	 * or
	 * [1]: table
	 * [2]: number
	 * 	len = number
	 * 	data = table[i] (can call __index)
	 * or
	 * [1]: function
	 * [2]: number
	 * 	len = number
	 * 	data = function(i)
	 */
	unsigned int len;
	if (lua_gettop(L) == 1) {
		len = luaL_len(L, 1);
	} else {
		len = luaL_checknumber(L, 2);
	}

	float buffer[len];
	if (lua_istable(L, 1)) {
		for (unsigned int i = 0; i < len; i++) {
			lua_pushnumber(L, i + 1);
			lua_gettable(L, 1);
			buffer[i] = luaL_checknumber(L, -1);
			lua_pop(L, 1);
		}
	} else if (lua_isfunction(L, 1)) {
		for (unsigned int i = 0; i < len; i++) {
			lua_pushvalue(L, 1);
			lua_pushnumber(L, i);
			lua_call(L, 1, 1);
			buffer[i] = luaL_checknumber(L, -1);
			lua_pop(L, 1);
		}
	}

	Sound *chunk = engine->audio.create_sound(len, buffer);
	push_sound(L, chunk);
	return 1;
}

static int mlua_play_sound(lua_State *L)
{
	Sound* chunk = pop_sound(L, 1);

	float volume = 1;
	float x = 0;
	float y = 0;
	if (!lua_isnone(L, 2))
		volume = lua_tonumber(L, 2);
	if (!lua_isnone(L, 3))
		x = lua_tonumber(L, 3);
	if (!lua_isnone(L, 4))
		y = lua_tonumber(L, 4);

	engine->audio.play_sound(chunk, volume, x, y);
	return 0;
}

static int mlua_free_sound(lua_State *L)
{
	DEBUG("");
	Sound* chunk = pop_sound(L, 1);
	engine->audio.free_sound(chunk);
	return 0;
}

static int mlua_free_music(lua_State *L)
{
	DEBUG("");
	Music* music = pop_music(L, 1);
	engine->audio.free_music(music);
	return 0;
}

class LuaMusicCallback : public MusicCallback
{
public:
	lua_State* L;
	int ref;
	int table_ref;

	LuaMusicCallback() :
		L(NULL),
		ref(LUA_NOREF),
		table_ref(LUA_NOREF) {
	}

	unsigned int feed_buffer(unsigned short* buffer, unsigned int len) {
		if (table_ref == LUA_NOREF) {
			lua_createtable(L, len, 0);
			this->table_ref = luaL_ref(L, LUA_REGISTRYINDEX);
		}
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
		lua_rawgeti(L, LUA_REGISTRYINDEX, this->table_ref);
		lua_pushunsigned(L, len);
		lua_call(L, 2, 1);

		unsigned int i = lua_tounsigned(L, -1);
		lua_pop(L, 1);

		lua_rawgeti(L, LUA_REGISTRYINDEX, table_ref);
		for (unsigned int k = 1; k <= i; k++) {
			lua_rawgeti(L, -1, k);
			lua_Number sample = lua_tonumber(L, -1);
			buffer[k] = sample * (1 << 15) + (1 << 15);
			lua_pop(L, 1);
		}
		return i;
	}

	~LuaMusicCallback() {
		luaL_unref(L, LUA_REGISTRYINDEX, ref);
		if (table_ref != LUA_NOREF)
			luaL_unref(L, LUA_REGISTRYINDEX, table_ref);
	}
};

static int mlua_load_music(lua_State *L)
{
	Music* music;
	if (lua_isstring(L, 1)) {
		const char* filename = lua_tostring(L, 1);
		music = engine->audio.load_music_from_file(filename);
	} else {
		LuaMusicCallback* callback = new LuaMusicCallback;
		callback->L = L;
		lua_pushvalue(L, 1);
		callback->ref = luaL_ref(L, LUA_REGISTRYINDEX);

		int samplesrate = luaL_optnumber(L, 2, DEFAULT_SAMPLES_RATE);
		music = engine->audio.load_music(callback, samplesrate);
	}
	push_music(L, music);
	return 1;
}

static int mlua_play_music(lua_State *L)
{
	Music* music = pop_music(L, 1);
	engine->audio.play_music(music);
	return 0;
}

static int mlua_set_sound_volume(lua_State *L)
{
	float volume = lua_tonumber(L, 1);
	engine->audio.set_sound_volume(volume);
	return 0;
}

static int mlua_set_music_volume(lua_State *L)
{
	float volume = lua_tonumber(L, 1);
	engine->audio.set_music_volume(volume);
	return 0;
}

static int mlua_stop_music(lua_State* L)
{
	Music* music = pop_music(L, 1);
	engine->audio.stop_music(music);
	return 0;
}

//
// Lua load
//

int luaopen_drystal(lua_State* L)
{
	DECLARE_GC(sound, mlua_free_sound)
	DECLARE_GC(music, mlua_free_music)
	DECLARE_GC(buffer, mlua_free_buffer)
	DECLARE_GC(shader, mlua_free_shader)
	DECLARE_GC(surface, mlua_free_surface)

	REGISTER_GC(sound);
	REGISTER_GC(music);
	REGISTER_GC(shader);
	REGISTER_GC(buffer);
	REGISTER_GC(surface);

	static const luaL_Reg lib[] = {
		{"engine_stop", mlua_stop},
		DECLARE_FUNCTION(stop),
		DECLARE_FUNCTION(reload),

		DECLARE_FUNCTION(show_cursor),
		DECLARE_FUNCTION(set_relative_mode),

		DECLARE_FUNCTION(resize),
		DECLARE_FUNCTION(screen2scene),
		DECLARE_FUNCTION(flip), // DEPRECATED

		DECLARE_FUNCTION(start_text),
		DECLARE_FUNCTION(stop_text),

		/* DISPLAY SURFACE */
		DECLARE_FUNCTION(load_surface),
		DECLARE_FUNCTION(new_surface),
		DECLARE_FUNCTION(surface_size),
		DECLARE_FUNCTION(draw_on),
		DECLARE_FUNCTION(draw_from),

		/* DISPLAY DRAWERS */
		DECLARE_FUNCTION(draw_background),
		DECLARE_FUNCTION(draw_point),
		DECLARE_FUNCTION(draw_point_tex),
		DECLARE_FUNCTION(draw_line),
		DECLARE_FUNCTION(draw_triangle),
		DECLARE_FUNCTION(draw_surface),
		DECLARE_FUNCTION(draw_quad),

		/* DISPLAY SETTERS */
		DECLARE_FUNCTION(set_color),
		DECLARE_FUNCTION(set_alpha),
		DECLARE_FUNCTION(set_point_size),
		DECLARE_FUNCTION(set_line_width),
		DECLARE_FUNCTION(set_blend_mode),
		DECLARE_FUNCTION(set_filter_mode),

		/* DISPLAY SHADER */
		DECLARE_FUNCTION(new_shader),
		DECLARE_FUNCTION(use_shader),
		DECLARE_FUNCTION(feed_shader),

		/* DISPLAY BUFFER */
		DECLARE_FUNCTION(new_buffer),
		DECLARE_FUNCTION(use_buffer),
		DECLARE_FUNCTION(draw_buffer),
		DECLARE_FUNCTION(reset_buffer),
		DECLARE_FUNCTION(upload_and_free_buffer),

		/* AUDIO */
		DECLARE_FUNCTION(set_music_volume),
		DECLARE_FUNCTION(load_music),
		DECLARE_FUNCTION(play_music),
		DECLARE_FUNCTION(stop_music),

		DECLARE_FUNCTION(load_sound),
		DECLARE_FUNCTION(create_sound),
		DECLARE_FUNCTION(play_sound),
		DECLARE_FUNCTION(set_sound_volume),

		{NULL, NULL}
	};

	luaL_newlib(L, lib);
	luaL_setfuncs(L, lib, 0);

	{
		// screen
		Surface* screen = engine->display.get_screen();
		if (screen)
			lua_pushlightuserdata(L, screen);
		else
			lua_pushnil(L);
		lua_setfield(L, -2, "screen");
		lua_pushvalue(L, -1);
	}

	{
		// blend modes
		lua_pushnumber(L, DEFAULT);
		lua_setfield(L, -2, "BLEND_DEFAULT");
		lua_pushvalue(L, -1);
		lua_pushnumber(L, ALPHA);
		lua_setfield(L, -2, "BLEND_ALPHA");
		lua_pushvalue(L, -1);
		lua_pushnumber(L, ADD);
		lua_setfield(L, -2, "BLEND_ADD");
		lua_pushvalue(L, -1);
		lua_pushnumber(L, MULT);
		lua_setfield(L, -2, "BLEND_MULT");
		lua_pushvalue(L, -1);
	}
	{
		// filter modes
		lua_pushnumber(L, LINEAR);
		lua_setfield(L, -2, "FILTER_LINEAR");
		lua_pushvalue(L, -1);
		lua_pushnumber(L, NEAREST);
		lua_setfield(L, -2, "FILTER_NEAREST");
		lua_pushvalue(L, -1);
	}

	{
		// camera
		lua_newtable(L);
		luaL_newmetatable(L, "__camera_class");
		lua_pushcfunction(L, mlua_camera__newindex);
		lua_setfield(L, -2, "__newindex");
		lua_pushcfunction(L, mlua_camera__index);
		lua_setfield(L, -2, "__index");
		lua_setmetatable(L, -2);
		lua_pushcfunction(L, mlua_camera_reset);
		lua_setfield(L, -2, "reset");

		// glue it on drystal table
		lua_setfield(L, -2, "camera");
		lua_pushvalue(L, -1);
	}

	engine->lua.drystal_table_ref = luaL_ref(L, LUA_REGISTRYINDEX);

	return 1;
}

