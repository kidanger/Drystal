#include <cstring>
#include <cmath>
#include <cassert>

#include <lua.hpp>

#include "engine.hpp"
#include "log.hpp"
#include "lua_functions.hpp"

#define DECLARE_FUNCTION(name) {#name, mlua_##name}

// used to access some engine's fields from lua callbacks
static Engine *engine;

static int luaopen_drystal(lua_State*); // defined at the end of this file

DECLARE_PUSHPOP(Shader, shader)
DECLARE_PUSHPOP(Surface, surface)
DECLARE_PUSHPOP(Buffer, buffer)
DECLARE_PUSHPOP(Sound, sound)
DECLARE_PUSHPOP(Music, music)

LuaFunctions::LuaFunctions(Engine& eng, const char *_filename) :
	L(luaL_newstate()),
	drystal_table_ref(LUA_NOREF),
	filename(_filename),
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

void LuaFunctions::add_search_path(const char* path) const
{
	assert(path);

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

int traceback(lua_State *L)
{
	// from lua/src/lua.c
	const char *msg = lua_tostring(L, 1);
	if (msg)
		luaL_traceback(L, L, msg, 1);
	else if (!lua_isnoneornil(L, 1)) {  /* is there an error object? */
		if (!luaL_callmeta(L, 1, "__tostring"))  /* try its 'tostring' metamethod */
			lua_pushliteral(L, "(no error message)");
	}
	return 1;
}

/**
 * Search for a function named 'name' in the drystal table.
 * Return true if found and keep the function in the lua stack
 * Otherwise, return false (stack is cleaned as needed).
 */
bool LuaFunctions::get_function(const char* name) const
{
	assert(name);

	lua_rawgeti(L, LUA_REGISTRYINDEX, drystal_table_ref);
	lua_getfield(L, -1, name);
	if (lua_isfunction(L, -1)) {
		return true;
	}
	lua_pop(L, 2);
	return false;
}

void LuaFunctions::remove_userpackages() const
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

	lua_pushcfunction(L, traceback);
	if (luaL_loadfile(L, filename) || lua_pcall(L, 0, 0, -2)) {
		fprintf(stderr, "[ERROR] cannot run script: %s\n", lua_tostring(L, -1));
		return false;
	}
	return true;
}

bool LuaFunctions::reload_code()
{
	if (get_function("prereload")) {
		CALL(0, 0);
	}
	remove_userpackages();

	printf("Reloading code...\n");
	bool ok = load_code() && call_init();
	if (ok) {
		if (get_function("postreload")) {
			CALL(0, 0);
		}
	}
	return ok;
}

bool LuaFunctions::call_init() const
{
	if (get_function("init")) {
		CALL(0, 0);
	}
	return true;
}

void LuaFunctions::call_mouse_motion(int mx, int my, int dx, int dy) const
{
	if (get_function("mouse_motion")) {
		lua_pushnumber(L, mx);
		lua_pushnumber(L, my);
		lua_pushnumber(L, dx);
		lua_pushnumber(L, dy);
		CALL(4, 0);
	}
}

void LuaFunctions::call_mouse_press(int mx, int my, int button) const
{
	if (get_function("mouse_press")) {
		lua_pushnumber(L, mx);
		lua_pushnumber(L, my);
		lua_pushnumber(L, button);
		CALL(3, 0);
	}
}

void LuaFunctions::call_mouse_release(int mx, int my, int button) const
{
	if (get_function("mouse_release")) {
		lua_pushnumber(L, mx);
		lua_pushnumber(L, my);
		lua_pushnumber(L, button);
		CALL(3, 0);
	}
}

void LuaFunctions::call_key_press(const char* key_string) const
{
	assert(key_string);

	if (get_function("key_press")) {
		lua_pushstring(L, key_string);
		CALL(1, 0);
	}
}

void LuaFunctions::call_key_release(const char* key_string) const
{
	assert(key_string);

	if (get_function("key_release")) {
		lua_pushstring(L, key_string);
		CALL(1, 0);
	}
}

void LuaFunctions::call_key_text(const char* string) const
{
	assert(string);

	if (get_function("key_text")) {
		lua_pushstring(L, string);
		CALL(1, 0);
	}
}

void LuaFunctions::call_resize_event(int w, int h) const
{
	if (get_function("resize_event")) {
		lua_pushnumber(L, w);
		lua_pushnumber(L, h);
		CALL(2, 0);
	}
}

void LuaFunctions::call_update(float dt) const
{
	if (get_function("update")) {
		lua_pushnumber(L, dt);
		CALL(1, 0);
	}
}

void LuaFunctions::call_draw() const
{
	if (get_function("draw")) {
		CALL(0, 0);
	}
}

void LuaFunctions::call_atexit() const
{
	if (get_function("atexit")) {
		CALL(0, 0);
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
	assert(L);

	if (lua_istable(L, 1)) {
		lua_rawgeti(L, 1, 1);
		lua_rawgeti(L, 1, 2);
		lua_rawgeti(L, 1, 3);
	}
	int r = luaL_checkint(L, -3);
	int g = luaL_checkint(L, -2);
	int b = luaL_checkint(L, -1);
	engine->display.set_color(r, g, b);
	return 0;
}

static int mlua_set_alpha(lua_State* L)
{
	assert(L);

	int alpha = luaL_checkint(L, 1);
	engine->display.set_alpha(alpha);
	return 0;
}

static int mlua_set_point_size(lua_State* L)
{
	assert(L);

	lua_Number point_size = luaL_checknumber(L, 1);
	engine->display.set_point_size(point_size);
	return 0;
}

static int mlua_set_line_width(lua_State* L)
{
	assert(L);

	lua_Number width = luaL_checknumber(L, 1);
	engine->display.set_line_width(width);
	return 0;
}

static int mlua_set_title(lua_State* L)
{
	assert(L);

	const char *title = luaL_checkstring(L, 1);
	engine->display.set_title(title);
	return 0;
}

static int mlua_set_blend_mode(lua_State* L)
{
	assert(L);

	BlendMode mode = static_cast<BlendMode>(luaL_checknumber(L, 1));
	engine->display.set_blend_mode(mode);
	return 0;
}

static int mlua_camera__newindex(lua_State* L)
{
	assert(L);

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
	assert(L);

	bool show = lua_toboolean(L, 1);
	engine->display.show_cursor(show);
	return 0;
}

static int mlua_set_relative_mode(lua_State* L)
{
	assert(L);

	bool relative = lua_toboolean(L, 1);
	engine->event.set_relative_mode(relative);
	return 0;
}

static int mlua_resize(lua_State* L)
{
	assert(L);

	int w = luaL_checkint(L, 1);
	int h = luaL_checkint(L, 2);
	engine->display.resize(w, h);

	// update screen
	lua_rawgeti(L, LUA_REGISTRYINDEX, engine->lua.drystal_table_ref);
	Surface* screen = engine->display.get_screen();
	if (screen)
		push_surface(L, screen);
	else
		lua_pushnil(L);
	lua_setfield(L, -2, "screen");
	lua_pop(L, 1);

	return 0;
}

static int mlua_screen2scene(lua_State* L)
{
	assert(L);

	lua_Number x = luaL_checknumber(L, 1);
	lua_Number y = luaL_checknumber(L, 2);
	float tx, ty;
	engine->display.screen2scene(x, y, &tx, &ty);
	lua_pushnumber(L, tx);
	lua_pushnumber(L, ty);
	return 2;
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


static int __surface_class_index(lua_State* L)
{
	assert(L);

	Surface* surface = pop_surface(L, 1);
	const char* index = luaL_checkstring(L, 2);
	if (strcmp(index, "w") == 0) {
		lua_pushnumber(L, surface->w);
	} else if (strcmp(index, "h") == 0) {
		lua_pushnumber(L, surface->h);
	} else {
		lua_getmetatable(L, 1);
		lua_getfield(L, -1, index);
	}
	return 1;
}

static int mlua_load_surface(lua_State* L)
{
	assert(L);

	const char * filename = luaL_checkstring(L, 1);
	Surface* surface = engine->display.load_surface(filename);
	if (surface) {
		push_surface(L, surface);
		return 1;
	}
	return luaL_fileresult(L, 0, filename);
}

static int mlua_new_surface(lua_State* L)
{
	assert(L);

	int w = luaL_checkint(L, 1);
	int h = luaL_checkint(L, 2);
	Surface* surface = engine->display.new_surface(w, h);
	push_surface(L, surface);
	return 1;
}

static int mlua_free_surface(lua_State* L)
{
	assert(L);

	DEBUG("");
	Surface* surface = pop_surface(L, 1);
	if (surface != engine->display.get_screen()) {
		engine->display.free_surface(surface);
	}
	return 0;
}

static int mlua_draw_on(lua_State* L)
{
	assert(L);

	Surface* surface = pop_surface(L, 1);
	engine->display.draw_on(surface);
	return 0;
}

static int mlua_draw_from(lua_State* L)
{
	assert(L);

	Surface* surface = pop_surface(L, 1);
	engine->display.draw_from(surface);
	return 0;
}

static int mlua_set_filter(lua_State* L)
{
	assert(L);

	Surface* surface = pop_surface(L, 1);
	FilterMode mode = static_cast<FilterMode>(luaL_checknumber(L, 2));
	engine->display.set_filter(surface, mode);
	return 0;
}

static int mlua_draw_background(lua_State*)
{
	engine->display.draw_background();
	return 0;
}

static int mlua_draw_point(lua_State* L)
{
	assert(L);

	lua_Number x = luaL_checknumber(L, 1);
	lua_Number y = luaL_checknumber(L, 2);
	engine->display.draw_point(x, y);
	return 0;
}

static int mlua_draw_point_tex(lua_State* L)
{
	assert(L);

	lua_Number xi = luaL_checknumber(L, 1);
	lua_Number yi = luaL_checknumber(L, 2);
	lua_Number xd = luaL_checknumber(L, 3);
	lua_Number yd = luaL_checknumber(L, 4);
	engine->display.draw_point_tex(xi, yi, xd, yd);
	return 0;
}

static int mlua_draw_line(lua_State* L)
{
	assert(L);

	lua_Number x1 = luaL_checknumber(L, 1);
	lua_Number y1 = luaL_checknumber(L, 2);
	lua_Number x2 = luaL_checknumber(L, 3);
	lua_Number y2 = luaL_checknumber(L, 4);
	engine->display.draw_line(x1, y1, x2, y2);
	return 0;
}

static int mlua_draw_triangle(lua_State* L)
{
	assert(L);

	lua_Number x1 = luaL_checknumber(L, 1);
	lua_Number y1 = luaL_checknumber(L, 2);
	lua_Number x2 = luaL_checknumber(L, 3);
	lua_Number y2 = luaL_checknumber(L, 4);
	lua_Number x3 = luaL_checknumber(L, 5);
	lua_Number y3 = luaL_checknumber(L, 6);
	engine->display.draw_triangle(x1, y1, x2, y2, x3, y3);
	return 0;
}

static int mlua_draw_surface(lua_State* L)
{
	assert(L);

	lua_Number i1 = luaL_checknumber(L, 1);
	lua_Number i2 = luaL_checknumber(L, 2);
	lua_Number i3 = luaL_checknumber(L, 3);
	lua_Number i4 = luaL_checknumber(L, 4);
	lua_Number i5 = luaL_checknumber(L, 5);
	lua_Number i6 = luaL_checknumber(L, 6);
	lua_Number o1 = luaL_checknumber(L, 7);
	lua_Number o2 = luaL_checknumber(L, 8);
	lua_Number o3 = luaL_checknumber(L, 9);
	lua_Number o4 = luaL_checknumber(L, 10);
	lua_Number o5 = luaL_checknumber(L, 11);
	lua_Number o6 = luaL_checknumber(L, 12);
	engine->display.draw_surface(i1, i2, i3, i4, i5, i6,
	                             o1, o2, o3, o4, o5, o6);
	return 0;
}

static int mlua_draw_quad(lua_State* L)
{
	assert(L);

	lua_Number i1 = luaL_checknumber(L, 1);
	lua_Number i2 = luaL_checknumber(L, 2);
	lua_Number i3 = luaL_checknumber(L, 3);
	lua_Number i4 = luaL_checknumber(L, 4);
	lua_Number i5 = luaL_checknumber(L, 5);
	lua_Number i6 = luaL_checknumber(L, 6);
	lua_Number i7 = luaL_checknumber(L, 7);
	lua_Number i8 = luaL_checknumber(L, 8);
	lua_Number o1 = luaL_checknumber(L, 9);
	lua_Number o2 = luaL_checknumber(L, 10);
	lua_Number o3 = luaL_checknumber(L, 11);
	lua_Number o4 = luaL_checknumber(L, 12);
	lua_Number o5 = luaL_checknumber(L, 13);
	lua_Number o6 = luaL_checknumber(L, 14);
	lua_Number o7 = luaL_checknumber(L, 15);
	lua_Number o8 = luaL_checknumber(L, 16);
	engine->display.draw_quad(i1, i2, i3, i4, i5, i6, i7, i8,
	                          o1, o2, o3, o4, o5, o6, o7, o8);
	return 0;
}

static int mlua_new_shader(lua_State* L)
{
	assert(L);

	const char *vert = NULL, *frag_color = NULL, *frag_tex = NULL;
	// strings can be nil
	if (lua_gettop(L) >= 1) { // one argument, it's the vertex shader
		vert = lua_tostring(L, 1);
	}
	if (lua_gettop(L) >= 2) {
		frag_color = lua_tostring(L, 2);
	}
	if (lua_gettop(L) >= 3) {
		frag_tex = lua_tostring(L, 3);
	}
	char* error;
	Shader* shader = engine->display.new_shader(vert, frag_color, frag_tex, &error);
	if (shader) {
		push_shader(L, shader);
		return 1;
	} else {
		lua_pushnil(L);
		lua_pushstring(L, error);
		delete[] error;
		return 2;
	}
}

static int mlua_use_shader(lua_State* L)
{
	assert(L);

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
	assert(L);

	Shader* shader = pop_shader(L, 1);
	const char* name = lua_tostring(L, 2);
	float value = luaL_checknumber(L, 3);
	engine->display.feed_shader(shader, name, value);
	return 0;
}

static int mlua_free_shader(lua_State* L)
{
	assert(L);

	DEBUG("");
	Shader* shader = pop_shader(L, 1);
	engine->display.free_shader(shader);
	return 0;
}

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
	assert(L);

	if (lua_gettop(L) == 0) { // use defaut buffer
		engine->display.use_buffer(NULL);
	} else {
		Buffer* buffer = pop_buffer(L, 1);
		engine->display.use_buffer(buffer);
	}
	return 0;
}

static int mlua_draw_buffer(lua_State* L)
{
	assert(L);

	Buffer* buffer = pop_buffer(L, 1);
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
	assert(L);

	Buffer* buffer = pop_buffer(L, 1);
	engine->display.reset_buffer(buffer);
	return 0;
}

static int mlua_upload_and_free_buffer(lua_State* L)
{
	assert(L);

	Buffer* buffer = pop_buffer(L, 1);
	engine->display.upload_and_free_buffer(buffer);
	return 0;
}

static int mlua_free_buffer(lua_State* L)
{
	assert(L);

	DEBUG("");
	Buffer* buffer = pop_buffer(L, 1);
	engine->display.free_buffer(buffer);
	return 0;
}

static int mlua_load_sound(lua_State *L)
{
	assert(L);

	const char* filename = lua_tostring(L, 1);
	Sound *chunk = engine->audio.load_sound(filename);
	if (chunk) {
		push_sound(L, chunk);
		return 1;
	}
	return luaL_fileresult(L, 0, filename);
}

static int mlua_create_sound(lua_State *L)
{
	assert(L);

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
	assert(L);

	Sound* chunk = pop_sound(L, 1);

	float volume = 1;
	float x = 0;
	float y = 0;
	if (!lua_isnone(L, 2))
		volume = luaL_checknumber(L, 2);
	if (!lua_isnone(L, 3))
		x = luaL_checknumber(L, 3);
	if (!lua_isnone(L, 4))
		y = luaL_checknumber(L, 4);

	engine->audio.play_sound(chunk, volume, x, y);
	return 0;
}

static int mlua_free_sound(lua_State *L)
{
	assert(L);

	DEBUG("");
	Sound* chunk = pop_sound(L, 1);
	engine->audio.free_sound(chunk);
	return 0;
}

static int mlua_free_music(lua_State *L)
{
	assert(L);

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
		table_ref(LUA_NOREF)
	{
	}

	unsigned int feed_buffer(unsigned short* buffer, unsigned int len)
	{
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

	~LuaMusicCallback()
	{
		luaL_unref(L, LUA_REGISTRYINDEX, ref);
		if (table_ref != LUA_NOREF)
			luaL_unref(L, LUA_REGISTRYINDEX, table_ref);
	}
private:
	LuaMusicCallback(const LuaMusicCallback&);
	LuaMusicCallback& operator=(const LuaMusicCallback&);
};

static int mlua_load_music(lua_State *L)
{
	assert(L);

	Music* music;
	if (lua_isstring(L, 1)) {
		const char* filename = lua_tostring(L, 1);
		music = engine->audio.load_music_from_file(filename);
		if (!music) {
			return luaL_fileresult(L, 0, filename);
		}
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
	assert(L);

	Music* music = pop_music(L, 1);
	engine->audio.play_music(music);
	return 0;
}

static int mlua_set_sound_volume(lua_State *L)
{
	assert(L);

	float volume = luaL_checknumber(L, 1);
	engine->audio.set_sound_volume(volume);
	return 0;
}

static int mlua_set_music_volume(lua_State *L)
{
	assert(L);

	float volume = luaL_checknumber(L, 1);
	engine->audio.set_music_volume(volume);
	return 0;
}

static int mlua_stop_music(lua_State* L)
{
	assert(L);

	Music* music = pop_music(L, 1);
	engine->audio.stop_music(music);
	return 0;
}

extern "C" {
	extern int json_encode(lua_State* L);
	extern int json_decode(lua_State* L);
	extern void lua_cjson_init();
}
static int mlua_store(lua_State* L)
{
	assert(L);

	const char* key = luaL_checkstring(L, 1);

	lua_pushcfunction(L, json_encode);
	lua_pushvalue(L, 2);

	CALL(1, 1); // table in param, returns json

	const char* value = luaL_checkstring(L, -1);
	engine->storage.store(key, value);
	return 0;
}

static int mlua_fetch(lua_State* L)
{
	assert(L);

	const char* key = luaL_checkstring(L, 1);
	const char* value = engine->storage.fetch(key);

	if (!value[0]) {
		return 0;
	}

	lua_pushcfunction(L, json_decode);
	lua_pushstring(L, value);
	CALL(1, 1);
	// table is returned by json_decode
	return 1;
}


//
// Lua load
//

int luaopen_drystal(lua_State* L)
{
	assert(L);

	static const luaL_Reg lib[] = {
		{"engine_stop", mlua_stop},
		DECLARE_FUNCTION(stop),
		DECLARE_FUNCTION(reload),

		DECLARE_FUNCTION(show_cursor),
		DECLARE_FUNCTION(set_relative_mode),

		DECLARE_FUNCTION(resize),
		DECLARE_FUNCTION(set_title),
		DECLARE_FUNCTION(screen2scene),

		DECLARE_FUNCTION(start_text),
		DECLARE_FUNCTION(stop_text),

		/* DISPLAY SURFACE */
		DECLARE_FUNCTION(load_surface),
		DECLARE_FUNCTION(new_surface),
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

		/* DISPLAY SHADER */
		DECLARE_FUNCTION(new_shader),
		DECLARE_FUNCTION(use_shader),

		/* DISPLAY BUFFER */
		DECLARE_FUNCTION(new_buffer),
		DECLARE_FUNCTION(use_buffer),

		/* AUDIO */
		DECLARE_FUNCTION(load_music),
		DECLARE_FUNCTION(set_music_volume),

		DECLARE_FUNCTION(load_sound),
		DECLARE_FUNCTION(create_sound),
		DECLARE_FUNCTION(set_sound_volume),

		/* STORAGE */
		DECLARE_FUNCTION(store),
		DECLARE_FUNCTION(fetch),

		/* SERIALIZER */
		{"serialize", json_encode},
		{"deserialize", json_decode},

		{NULL, NULL}
	};

	luaL_newlib(L, lib);

	BEGIN_CLASS(surface)
	DECLARE_FUNCTION(draw_on),
	DECLARE_FUNCTION(draw_from),
	DECLARE_FUNCTION(set_filter),
	ADD_GC(free_surface)
	END_CLASS();
	REGISTER_CLASS_WITH_INDEX(surface, "__Surface");

	BEGIN_CLASS(sound)
	ADD_METHOD(sound, play)
	ADD_GC(free_sound)
	END_CLASS();
	REGISTER_CLASS(sound, "__Sound");

	BEGIN_CLASS(music)
	ADD_METHOD(music, play)
	ADD_METHOD(music, stop)
	ADD_GC(free_music)
	END_CLASS();
	REGISTER_CLASS(music, "__Music");

	BEGIN_CLASS(buffer)
	ADD_METHOD(buffer, use)
	ADD_METHOD(buffer, draw)
	ADD_METHOD(buffer, reset)
	ADD_METHOD(buffer, upload_and_free)
	ADD_GC(free_buffer)
	END_CLASS();
	REGISTER_CLASS(buffer, "__Buffer");

	BEGIN_CLASS(shader)
	ADD_METHOD(shader, use)
	ADD_METHOD(shader, feed)
	ADD_GC(free_shader)
	END_CLASS();
	REGISTER_CLASS(shader, "__Shader");

	{
		// screen
		Surface* screen = engine->display.get_screen();
		if (screen)
			push_surface(L, screen);
		else
			lua_pushnil(L);
		lua_setfield(L, -2, "screen");
	}

	{
		// blend modes
		lua_pushnumber(L, DEFAULT);
		lua_setfield(L, -2, "BLEND_DEFAULT");
		lua_pushnumber(L, ALPHA);
		lua_setfield(L, -2, "BLEND_ALPHA");
		lua_pushnumber(L, ADD);
		lua_setfield(L, -2, "BLEND_ADD");
		lua_pushnumber(L, MULT);
		lua_setfield(L, -2, "BLEND_MULT");
	}
	{
		// filter modes
		lua_pushnumber(L, NEAREST);
		lua_setfield(L, -2, "NEAREST");
		lua_pushnumber(L, LINEAR);
		lua_setfield(L, -2, "LINEAR");
		lua_pushnumber(L, BILINEAR);
		lua_setfield(L, -2, "BILINEAR");
		lua_pushnumber(L, TRILINEAR);
		lua_setfield(L, -2, "TRILINEAR");
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
	}

	lua_pushvalue(L, -1);
	engine->lua.drystal_table_ref = luaL_ref(L, LUA_REGISTRYINDEX);

	lua_cjson_init();

	assert(lua_gettop(L) == 2);
	return 1;
}
