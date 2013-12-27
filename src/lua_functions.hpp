#ifndef LUA_FUNCTIONS_H
#define LUA_FUNCTIONS_H

#ifndef EMSCRIPTEN
#include <ctime>
#endif

#include "engine.hpp"

struct lua_State;

class LuaFunctions
{
public:
	lua_State* L;
	int drystal_table_ref;

	LuaFunctions(Engine&, const char *filename);
	~LuaFunctions();

	void add_search_path(const char* path) const;

	bool load_code();
	bool reload_code();
	bool call_init() const;

	void call_update(float dt) const;
	void call_draw() const;

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

	bool get_function(const char* name) const;
	void remove_userpackages() const;
};

#define DECLARE_PUSHPOP(T, name) \
	static void push_ ## name(lua_State *L, T *name)\
	{\
		assert(L);\
		assert(name);\
		T **p = static_cast<T **>(lua_newuserdata(L, sizeof(T **)));\
		*p = name;\
		luaL_getmetatable(L, #name);\
		lua_setmetatable(L, -2);\
	}\
	static T *pop_ ## name(lua_State *L, int index)\
	{\
		assert(L);\
		luaL_checktype(L, index, LUA_TUSERDATA);\
		T **p = static_cast<T **>(luaL_checkudata(L, index, #name));\
		if (p == NULL) luaL_argerror(L, index, #name" expected");\
		assert(p);\
		return *p;\
	}

#define BEGIN_CLASS(name) static const luaL_Reg __ ## name ## _class[] = {

#define ADD_GC(func) { "__gc", mlua_ ## func},
#define ADD_METHOD(class, name) { #name, mlua_ ## name ## _ ## class },
#define END_CLASS() {NULL, NULL} }

#define REGISTER_CLASS(name) \
	luaL_newmetatable(L, #name);\
	luaL_setfuncs(L, __ ## name ## _class, 0); \
	lua_pushvalue(L, -1); \
	lua_setfield(L, -2, "__index")

#define REGISTER_CLASS_WITH_INDEX(name) \
	luaL_newmetatable(L, #name);\
	luaL_setfuncs(L, __ ## name ## _class, 0); \
	lua_pushcfunction(L, __ ## name ## _class_index); \
	lua_setfield(L, -2, "__index")

#endif

