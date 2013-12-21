
add_custom_command(OUTPUT lua_cjson_patched.c
    COMMAND patch ${CMAKE_CURRENT_SOURCE_DIR}/lua-cjson/lua_cjson.c ${CMAKE_CURRENT_SOURCE_DIR}/cjson.patch -o lua_cjson_patched.c
    COMMENT "Patching lua-cjson")

include_directories(lua-cjson)
add_library(lua-cjson STATIC lua-cjson/fpconv.c lua-cjson/strbuf.c lua_cjson_patched.c)
add_library(storage SHARED main.cpp)
target_link_libraries(storage lua-cjson)
