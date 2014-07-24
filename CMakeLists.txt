CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(drystal)

option(BUILD_ENABLE_COVERAGE "Enable code coverage" OFF)
option(BUILD_ENABLE_STATS    "Enable statistics (needs BUILD_GRAPHICS as dependency)" OFF)
option(BUILD_PHYSIC          "Enable physic module" ON)
option(BUILD_PARTICLE        "Enable particle module (needs BUILD_GRAPHICS as dependency)" ON)
option(BUILD_WEB             "Enable web module" ON)
option(BUILD_NET             "Enable net module" ON)
option(BUILD_FONT            "Enable font module (needs BUILD_GRAPHICS as dependency)" ON)
option(BUILD_AUDIO           "Enable audio module" ON)
option(BUILD_STORAGE         "Enable storage module" ON)
option(BUILD_EVENT           "Enable event module (needs BUILD_GRAPHICS as dependency)" ON)
option(BUILD_GRAPHICS        "Enable graphics module" ON)

if(BUILD_FONT AND NOT BUILD_GRAPHICS)
	message(FATAL_ERROR "Cannot enable BUILD_FONT: BUILD_GRAPHICS is needed as a dependency")
endif()
if(BUILD_EVENT AND NOT BUILD_GRAPHICS)
	message(FATAL_ERROR "Cannot enable BUILD_EVENT: BUILD_GRAPHICS is needed as a dependency")
endif()
if(BUILD_PARTICLE AND NOT BUILD_GRAPHICS)
	message(FATAL_ERROR "Cannot enable BUILD_PARTICLE: BUILD_GRAPHICS is needed as a dependency")
endif()
if(BUILD_ENABLE_STATS AND NOT BUILD_GRAPHICS)
	message(FATAL_ERROR "Cannot enable BUILD_ENABLE_STATS: BUILD_GRAPHICS is needed as a dependency")
endif()

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
if(BUILD_PHYSIC)
	include_directories(external/box2d/Box2D)
endif()

add_subdirectory(src)

include_directories(src)

