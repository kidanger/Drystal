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
        loadlib_patched.c
        lua/src/linit.c)

include_directories(.)
include_directories(lua/src)

add_custom_command(OUTPUT loadlib_patched.c
    COMMAND patch ${CMAKE_CURRENT_SOURCE_DIR}/lua/src/loadlib.c ${CMAKE_CURRENT_SOURCE_DIR}/dlopen_emscripten.patch -o loadlib_patched.c
    COMMENT "Patching lua")

add_definitions(-DLUA_COMPAT_ALL -DLUA_USE_DLOPEN)
add_library(lua SHARED ${CORE_C} ${LIB_C})

