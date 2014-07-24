CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(drystal)

option(BUILD_ENABLE_COVERAGE "Enable code coverage" OFF)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # -fPIC

if (CMAKE_CXX_COMPILER MATCHES ".*clang")
	set(CMAKE_COMPILER_IS_CLANGXX 1)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -march=native -DDODEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -mtune=native")
endif()

include(Utils)

add_subdirectory(external)

include_directories(external)
include_directories(external/lua/src)
include_directories(external/box2d/Box2D)

add_subdirectory(src)

include_directories(src)

