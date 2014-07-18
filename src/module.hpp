#undef BEGIN_CLASS
#undef BEGIN_MODULE
#undef DECLARE_FUNCTION
#undef END_MODULE
#undef BEGIN_CLASS
#undef ADD_GC
#undef ADD_METHOD
#undef ADD_GETSET
#undef END_CLASS
#undef REGISTER_CLASS
#undef REGISTER_CLASS_WITH_INDEX
#undef REGISTER_CLASS_WITH_INDEX_AND_NEWINDEX

#ifdef IMPLEMENT_MODULE

#define BEGIN_MODULE(name) \
	void register_##name(lua_State* L) {
#define DECLARE_FUNCTION(name) \
		lua_pushcfunction(L, mlua_##name); \
		lua_setfield(L, -2, #name);
#define END_MODULE() \
	}

#define BEGIN_CLASS(name) static const luaL_Reg __ ## name ## _class[] = {
#define ADD_GC(func) { "__gc", mlua_ ## func},
#define ADD_METHOD(class, name) { #name, mlua_ ## name ## _ ## class },
#define ADD_GETSET(class, name) \
	ADD_METHOD(class, get_##name) \
	ADD_METHOD(class, set_##name)
#define END_CLASS() {NULL, NULL} };

#define REGISTER_CLASS(name, name_in_module) \
	luaL_newmetatable(L, #name); \
	luaL_setfuncs(L, __ ## name ## _class, 0); \
	lua_pushvalue(L, -1); \
	lua_setfield(L, -2, "__index"); \
	lua_setfield(L, -2, name_in_module);

#define REGISTER_CLASS_WITH_INDEX(name, name_in_module) \
	luaL_newmetatable(L, #name); \
	luaL_setfuncs(L, __ ## name ## _class, 0); \
	lua_pushcfunction(L, __ ## name ## _class_index); \
	lua_setfield(L, -2, "__index"); \
	lua_setfield(L, -2, name_in_module);

#define REGISTER_CLASS_WITH_INDEX_AND_NEWINDEX(name, name_in_module) \
	luaL_newmetatable(L, #name); \
	luaL_setfuncs(L, __ ## name ## _class, 0); \
	lua_pushcfunction(L, mlua_##name##_class_index); \
	lua_setfield(L, -2, "__index"); \
	lua_pushcfunction(L, mlua_## name## _class_newindex); \
	lua_setfield(L, -2, "__newindex"); \
	lua_setfield(L, -2, name_in_module);

#else // IMPLEMENT_MODULE is not defined

#define BEGIN_MODULE(name) \
	void register_##name(lua_State* L);
#define DECLARE_FUNCTION(name) \
	int mlua_##name(lua_State* L);
#define END_MODULE()

#define BEGIN_CLASS(name)
#define ADD_GC(func) \
	DECLARE_FUNCTION(func)
#define ADD_METHOD(class, name) \
	DECLARE_FUNCTION(name##_##class)
#define ADD_GETSET(class, name) \
	ADD_METHOD(class, get_##name) \
	ADD_METHOD(class, set_##name)
#define END_CLASS()

#define REGISTER_CLASS(name, name_in_module)
#define REGISTER_CLASS_WITH_INDEX(name, name_in_module) \
	DECLARE_FUNCTION(__##name##_class_index)
#define REGISTER_CLASS_WITH_INDEX_AND_NEWINDEX(name, name_in_module) \
	DECLARE_FUNCTION(name##_class_index) \
	DECLARE_FUNCTION(name##_class_newindex)

#endif // IMPLEMENT_MODULE
