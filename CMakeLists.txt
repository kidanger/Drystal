CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

set(CMAKE_POSITION_INDEPENDENT_CODE ON) # -fPIC
if(EMSCRIPTEN)
endif(EMSCRIPTEN)

project(drystal)

add_subdirectory(external)

include_directories(external)
include_directories(external/lua)
include_directories(external/lua/src)

add_subdirectory(src)

include_directories(src)
add_subdirectory(extensions)

add_custom_target(astyle COMMAND
    astyle --indent=tab --indent-preprocessor --style=kr
    --pad-oper --pad-header --unpad-paren -S -n
    ${CMAKE_SOURCE_DIR}"/src/*.cpp"
    ${CMAKE_SOURCE_DIR}"/src/*.hpp"
)

