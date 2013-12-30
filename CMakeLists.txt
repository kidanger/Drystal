CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

set(CMAKE_POSITION_INDEPENDENT_CODE ON) # -fPIC
if(EMSCRIPTEN)
endif(EMSCRIPTEN)

project(drystal)

if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS_DEBUG "-g -march=native")
    set(CMAKE_C_FLAGS_DEBUG "-g -march=native")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -mtune=native")
    set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -mtune=native")
endif(CMAKE_COMPILER_IS_GNUCC)

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

