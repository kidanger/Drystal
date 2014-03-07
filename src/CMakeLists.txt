aux_source_directory(. SOURCES)

set(DRYSTAL_OUT drystal)
if(EMSCRIPTEN)
    set(DRYSTAL_OUT drystal.js)
endif()

if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ftrapv -Wall -Wextra -Wundef -Wunreachable-code -Wwrite-strings -Weffc++")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ftrapv -Wall -Wextra -Wundef -Wunreachable-code -Wwrite-strings -Wstrict-prototypes")
endif()

add_executable(${DRYSTAL_OUT} ${SOURCES})


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
endif()

target_link_libraries(${DRYSTAL_OUT} lua-drystal)
target_link_libraries(${DRYSTAL_OUT} lua-cjson)
if(EMSCRIPTEN)
    target_link_libraries(${DRYSTAL_OUT} physic truetype particle net web storage)
endif(EMSCRIPTEN)

install(TARGETS ${DRYSTAL_OUT} DESTINATION /usr/bin)
