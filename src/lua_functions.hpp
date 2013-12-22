#ifndef LUA_FUNCTIONS_H
#define LUA_FUNCTIONS_H

#ifndef EMSCRIPTEN
#include <ctime>
#endif

#include "engine.hpp"

struct lua_State;

class LuaFunctions {
public:
	lua_State* L;
	int drystal_table_ref;

	LuaFunctions(Engine&, const char *filename);
	~LuaFunctions();

	void add_search_path(const char* path);

	bool load_code();
	bool reload_code();
	bool call_init();

	void call_update(float dt);
	void call_draw();

	void call_resize_event(int w, int h) const;
	void call_mouse_motion(int mx, int my, int dx, int dy) const;
	void call_mouse_press(int mx, int my, int button) const;
	void call_mouse_release(int mx, int my, int button) const;
	void call_key_press(const char* key_string) const;
	void call_key_release(const char* key_string) const;
	void call_key_text(const char* string) const;
	void call_atexit() const;

private:
	const char* filename;
	bool library_loaded;

	bool get_function(lua_State*, const char* name) const;
	void remove_userpackages(lua_State* L);
};

#endif

