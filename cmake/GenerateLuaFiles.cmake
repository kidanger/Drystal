# put all .lua files into a single .cpp files
# the function load_luafiles runs those codes and return 1 if success
set(LUAFILES_LIST "drystal.lua" "traceback.lua")
if(BUILD_WEB)
	list(APPEND LUAFILES_LIST "web/web.lua")
endif()
if(BUILD_PARTICLE)
	list(APPEND LUAFILES_LIST "particle/particle.lua")
endif()
if(BUILD_PHYSICS)
	list(APPEND LUAFILES_LIST "physics/raycast.lua")
endif()
if(BUILD_GRAPHICS)
	list(APPEND LUAFILES_LIST "graphics/postfx.lua" "graphics/draw.lua" "graphics/sprite.lua" "graphics/colors.lua")
endif()
if(BUILD_LIVECODING)
	list(APPEND LUAFILES_LIST "reload.lua")
endif()

set(LUAFILES_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/luafiles.c)

# generate luafiles.c
file(WRITE ${LUAFILES_OUTPUT} "
#include <stdio.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>

#include \"luafiles.h\"
#include \"lua_util.h\"
#include \"log.h\"
log_category(\"luafiles\");

int load_luafiles(lua_State* L, int traceback_index)
{
    (void) L;")
foreach (luacode ${LUAFILES_LIST})
    file(READ ${luacode} FILE_CONTENT)
    get_filename_component(luacode_short ${luacode} NAME)
    # \\ => \\\\
    string(REGEX REPLACE "\\\\\\\\" "\\\\\\\\\\\\\\\\" FILE_CONTENT "${FILE_CONTENT}")
    # \n => \\n
    string(REGEX REPLACE "\\\\\\n" "\\\\\\\\n" FILE_CONTENT "${FILE_CONTENT}")
    # " => \"
    string(REGEX REPLACE "\\\"" "\\\\\"" FILE_CONTENT "${FILE_CONTENT}")
    # <eol> => \n\<eol>
    string(REGEX REPLACE "\n" "\\\\n\\\\\n" FILE_CONTENT "${FILE_CONTENT}")
    file(APPEND ${LUAFILES_OUTPUT} "
    {
        const char* file = \"${luacode_short}\";
        const char* code = \"${FILE_CONTENT}\";
        if (luaL_loadbuffer(L, code, strlen(code), file)) {
            log_error(\"%s\", lua_tostring(L, -1));
            return 0;
        }
        if (lua_pcall(L, 0, 0, traceback_index)) {
            return 0;
        }
    }
")
endforeach (luacode)
file(APPEND ${LUAFILES_OUTPUT} "
    return 1;
}")

add_custom_command(
    OUTPUT ${LUAFILES_OUTPUT}
    COMMAND cmake .. # re-run code generation
    DEPENDS ${LUAFILES_LIST}
    VERBATIM)
list(APPEND SOURCES ${LUAFILES_OUTPUT})

