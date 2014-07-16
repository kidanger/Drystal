CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

option(BUILD_ENABLE_COVERAGE "Enable code coverage" OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON) # -fPIC

project(drystal)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -march=native -DDODEBUG")
    set(CMAKE_C_FLAGS_DEBUG "-g -O0 -march=native -DDODEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -mtune=native")
    set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -mtune=native")
endif()

add_subdirectory(external)

include_directories(external)
include_directories(external/lua/src)
include_directories(external/box2d/Box2D)

add_subdirectory(src)

include_directories(src)

add_subdirectory(data)

add_custom_target(astyle COMMAND
    astyle --indent=tab --indent-preprocessor --style=kr
    --pad-oper --pad-header --unpad-paren -S -n
    ${CMAKE_SOURCE_DIR}"/src/*.cpp"
    ${CMAKE_SOURCE_DIR}"/src/*.hpp"
)

