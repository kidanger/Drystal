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
    if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        add_definitions(-DLUA_USE_MACOSX)
        find_library(DL_LIBRARY NAMES dl)
        target_link_libraries(lua ${DL_LIBRARY})
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        add_definitions(-DLUA_USE_WIN)
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        add_definitions(-DLUA_USE_LINUX)
        find_library(DL_LIBRARY NAMES dl)
        target_link_libraries(lua ${DL_LIBRARY})
    endif()
endif()


###
### Lua_cjson
###
if(BUILD_UTILS)
	add_custom_command(OUTPUT lua_cjson_patched.c
	    COMMAND patch --quiet ${CMAKE_CURRENT_SOURCE_DIR}/lua-cjson/lua_cjson.c ${CMAKE_CURRENT_SOURCE_DIR}/cjson.patch -o lua_cjson_patched.c
	    COMMENT "Patching lua-cjson")

	include_directories(lua-cjson)
	add_library(lua-cjson STATIC lua-cjson/fpconv.c lua-cjson/strbuf.c lua_cjson_patched.c)
endif()


###
### Box2D
###
if(BUILD_PHYSIC)
	set(BOX2D_BUILD_STATIC ON CACHE BOOL "")
	set(BOX2D_BUILD_EXAMPLES OFF CACHE BOOL "")
	set(BOX2D_INSTALL_BY_DEFAULT OFF)
	set(BOX2D_INSTALL OFF CACHE BOOL "")
	add_subdirectory(box2d/Box2D)
endif()


###
### websocket
###
if(NOT DEFINED EMSCRIPTEN AND BUILD_NET)
    add_library(websocket STATIC websocket.c)
    find_library(CRYPTO_LIBRARY NAMES crypto)
    find_library(RESOLV_LIBRARY NAMES resolv)
    target_link_libraries(websocket ${CRYPTO_LIBRARY})
    target_link_libraries(websocket ${RESOLV_LIBRARY})
endif()

###
### STB
###
if(BUILD_AUDIO)
	add_library(stb_vorbis STATIC stb_vorbis.c)
	set_target_properties(stb_vorbis PROPERTIES COMPILE_DEFINITIONS "STB_VORBIS_NO_PUSHDATA_API")
endif()

if(BUILD_GRAPHICS)
	add_library(stb_image STATIC stb_image.c)
	set_target_properties(stb_image PROPERTIES COMPILE_FLAGS "-DSTB_IMAGE_IMPLEMENTATION -DSTB_IMAGE_NO_HDR")
endif()

if(BUILD_FONT)
	add_library(stb_truetype STATIC stb_truetype.c)
	set_target_properties(stb_truetype PROPERTIES COMPILE_DEFINITIONS "STB_TRUETYPE_IMPLEMENTATION")
endif()


###
### miniz
###
if(EMSCRIPTEN)
    add_library(miniz STATIC miniz.c)
endif()

###
### wavloader
###
if(BUILD_AUDIO)
	add_library(wavloader STATIC wavloader.c)
endif()
