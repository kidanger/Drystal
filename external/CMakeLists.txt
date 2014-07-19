###
### Lua
###
set(CORE_C lua/src/lapi.c
        lua/src/lcode.c
        lua/src/lctype.c
        lua/src/ldebug.c
        lua/src/ldo.c
        lua/src/ldump.c
        lua/src/lfunc.c
        lua/src/lgc.c
        lua/src/llex.c
        lua/src/lmem.c
        lua/src/lobject.c
        lua/src/lopcodes.c
        lua/src/lparser.c
        lua/src/lstate.c
        lua/src/lstring.c
        lua/src/ltable.c
        lua/src/ltm.c
        lua/src/lundump.c
        lua/src/lvm.c
        lua/src/lzio.c)
set(LIB_C  lua/src/lauxlib.c
        lua/src/lbaselib.c
        lua/src/lbitlib.c
        lua/src/lcorolib.c
        lua/src/ldblib.c
        lua/src/liolib.c
        lua/src/lmathlib.c
        lua/src/loslib.c
        lua/src/lstrlib.c
        lua/src/ltablib.c
        lua/src/loadlib.c
        lua/src/linit.c)

include_directories(.)
include_directories(lua/src)

add_library(lua STATIC ${CORE_C} ${LIB_C})

add_definitions(-DLUA_COMPAT_ALL)
if(NOT DEFINED EMSCRIPTEN)
    add_definitions(-DLUA_USE_DLOPEN)
    find_library(DL_LIBRARY NAMES dl)
    target_link_libraries(lua ${DL_LIBRARY})
endif()


###
### Lua_cjson
###
add_custom_command(OUTPUT lua_cjson_patched.c
    COMMAND patch --quiet ${CMAKE_CURRENT_SOURCE_DIR}/lua-cjson/lua_cjson.c ${CMAKE_CURRENT_SOURCE_DIR}/cjson.patch -o lua_cjson_patched.c
    COMMENT "Patching lua-cjson")

include_directories(lua-cjson)
add_library(lua-cjson STATIC lua-cjson/fpconv.c lua-cjson/strbuf.c lua_cjson_patched.c)


###
### Box2D
###
set(BOX2D_BUILD_STATIC ON CACHE BOOL "")
set(BOX2D_BUILD_EXAMPLES OFF CACHE BOOL "")
set(BOX2D_INSTALL_BY_DEFAULT OFF)
set(BOX2D_INSTALL OFF CACHE BOOL "")
add_subdirectory(box2d/Box2D)


###
### websocket
###
if(NOT DEFINED EMSCRIPTEN)
    add_library(websocket STATIC websocket.c)
    find_library(CRYPTO_LIBRARY NAMES crypto)
    find_library(RESOLV_LIBRARY NAMES resolv)
    target_link_libraries(websocket ${CRYPTO_LIBRARY})
    target_link_libraries(websocket ${RESOLV_LIBRARY})
endif()

###
### STB
###
add_library(stb_vorbis STATIC stb_vorbis.c)

#add_library(stb_image STATIC stb_image.c)
#set_target_properties(stb_image PROPERTIES COMPILE_DEFINITIONS "STB_IMAGE_IMPLEMENTATION")
