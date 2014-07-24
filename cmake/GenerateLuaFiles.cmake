# put all .lua files into a single .cpp files
# the function load_luafiles runs those codes and return 1 if success
set(LUAFILES_LIST "drystal.lua")
if(BUILD_NET)
	list(APPEND LUAFILES_LIST "net/net.lua")
endif()
if(BUILD_WEB)
	list(APPEND LUAFILES_LIST "web/web.lua")
endif()
if(BUILD_FONT)
	list(APPEND LUAFILES_LIST "truetype/truetype.lua")
endif()
if(BUILD_PARTICLE)
	list(APPEND LUAFILES_LIST "particle/particle.lua")
endif()
list(APPEND LUAFILES_LIST "postfx.lua" "draw.lua" "sprite.lua")

set(LUAFILES_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/luafiles.cpp)

# generate luafiles.cpp
file(WRITE ${LUAFILES_OUTPUT} "\
#include <cstdio>
#include <cstring>
#include <lua.hpp>

#include \"luafiles.hpp\"

int load_luafiles(lua_State* L)
{
    (void) L;")
foreach (luacode ${LUAFILES_LIST})
    file(READ ${luacode} FILE_CONTENT)
    get_filename_component(luacode_short ${luacode} NAME)
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
        if (luaL_loadbuffer(L, code, strlen(code), file) || lua_pcall(L, 0, 0, -2)) {
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
