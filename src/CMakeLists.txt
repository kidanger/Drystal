aux_source_directory(. SOURCES)

set(DRYSTAL_OUT drystal)
if(EMSCRIPTEN)
    set(DRYSTAL_OUT drystal.js)
endif(EMSCRIPTEN)

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
endif(NOT DEFINED EMSCRIPTEN)

if(EMSCRIPTEN)
    set(EXTENSIONS physic truetype particle net web storage)
endif(EMSCRIPTEN)

target_link_libraries(${DRYSTAL_OUT} lua ${EXTENSIONS})

