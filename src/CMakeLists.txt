include_directories(.)
aux_source_directory(. SOURCES)
aux_source_directory(event SOURCES)
aux_source_directory(physic SOURCES)
aux_source_directory(truetype SOURCES)
aux_source_directory(particle SOURCES)
aux_source_directory(audio SOURCES)
aux_source_directory(net SOURCES)
aux_source_directory(web SOURCES)
aux_source_directory(storage SOURCES)

# put all .lua files into a single .cpp files
# the function load_luafiles runs those codes and return 1 if success
file(GLOB_RECURSE LUAFILES_LIST *.lua)
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

set(DRYSTAL_OUT drystal)
if(EMSCRIPTEN)
    set(DRYSTAL_OUT drystal.js)
endif()

set(DRYSTAL_FLAGS " ")
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
    if(CMAKE_BUILD_TYPE MATCHES DEBUG)
        set(DRYSTAL_FLAGS "${DRYSTAL_FLAGS} -ftrapv")
    endif()
    set(DRYSTAL_FLAGS "${DRYSTAL_FLAGS} \
-fvisibility=hidden \
-Wall \
-Wextra \
-Wundef \
-Wunreachable-code \
-Wwrite-strings \
-Wcast-align \
-Wundef \
-Wformat=2 \
-Wformat-security \
-Wformat-nonliteral \
-Wpointer-arith \
-Wmissing-include-dirs \
-Wmissing-declarations \
-Wredundant-decls \
-Weffc++ \
-Werror")
# -Wcast-qual \
	if(CMAKE_COMPILER_IS_GNUCXX)
		set(DRYSTAL_FLAGS "${DRYSTAL_FLAGS} -Wlogical-op")
	endif()
endif()

add_executable(${DRYSTAL_OUT} ${SOURCES})
set_target_properties(${DRYSTAL_OUT} PROPERTIES COMPILE_FLAGS ${DRYSTAL_FLAGS})

if(NOT DEFINED EMSCRIPTEN)
    include(FindPkgConfig)

    pkg_search_module(SDL2 REQUIRED sdl2)
    pkg_search_module(OPENAL REQUIRED openal)
    pkg_search_module(GL REQUIRED gl)

    include_directories(${SDL2_INCLUDE_DIRS})
    include_directories(${OPENAL_INCLUDE_DIRS})
    include_directories(${GL_INCLUDE_DIRS})

    target_link_libraries(drystal ${SDL2_LIBRARIES})
    target_link_libraries(drystal ${OPENAL_LIBRARIES})
    target_link_libraries(drystal ${GL_LIBRARIES})

    if(BUILD_ENABLE_COVERAGE)
        find_program(COVERAGE_GCOV gcov)
        find_program(COVERAGE_LCOV lcov)
        find_program(COVERAGE_GENHTML genhtml)
        if (NOT COVERAGE_GCOV)
            message(FATAL_ERROR "Unable to find gcov")
        endif()
        if (NOT COVERAGE_LCOV)
            message(FATAL_ERROR "Unable to find lcov")
        endif()
        if (NOT COVERAGE_GENHTML)
            message(FATAL_ERROR "Unable to find genhtml")
        endif()

        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
        set(CMAKE_LD_FLAGS_DEBUG "${CMAKE_LD_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")

        add_custom_target(coverage-reset DEPENDS ${DRYSTAL_OUT})
        add_custom_command(TARGET coverage-reset
            COMMAND mkdir -p coverage
            COMMAND ${COVERAGE_LCOV} --directory . --zerocounters
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        )
        add_custom_target(coverage-report DEPENDS ${DRYSTAL_OUT})
        add_custom_command(TARGET coverage-report
            COMMAND ${COVERAGE_LCOV} --directory . --capture --output-file ./coverage/drystal.lcov
            COMMAND ${COVERAGE_LCOV} --remove ./coverage/drystal.lcov --output-file ./coverage/drystal_clean.lcov '/usr/*' 'box2d/*'
            COMMAND ${COVERAGE_GENHTML} -t "drystal coverage" -p "${CMAKE_SOURCE_DIR}" -o ./coverage ./coverage/drystal_clean.lcov
            COMMAND echo "Open ${CMAKE_BINARY_DIR}/coverage/index.html to view the coverage analysis results."
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        )
        target_link_libraries(${DRYSTAL_OUT} gcov)
    endif(BUILD_ENABLE_COVERAGE)
endif()

target_link_libraries(${DRYSTAL_OUT} stb_vorbis)
target_link_libraries(${DRYSTAL_OUT} stb_image)
target_link_libraries(${DRYSTAL_OUT} stb_truetype)
target_link_libraries(${DRYSTAL_OUT} lua)
target_link_libraries(${DRYSTAL_OUT} lua-cjson)
target_link_libraries(${DRYSTAL_OUT} Box2D)
if(EMSCRIPTEN)
    target_link_libraries(${DRYSTAL_OUT} miniz)
else()
    target_link_libraries(${DRYSTAL_OUT} websocket)
endif()

install(TARGETS ${DRYSTAL_OUT} DESTINATION /usr/bin)
