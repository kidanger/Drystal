CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(drystal)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Utils)

check_compilers_unknown()
check_git_submodules_initialized()

option(BUILD_MANPAGES        "Build manpages" ON)
option(BUILD_ENABLE_COVERAGE "Enable code coverage" OFF)
option(BUILD_PHYSICS         "Enable physics module" ON)
option(BUILD_PARTICLE        "Enable particle module (needs BUILD_GRAPHICS as dependency)" ON)
option(BUILD_WEB             "Enable web module" ON)
option(BUILD_FONT            "Enable font module (needs BUILD_GRAPHICS as dependency)" ON)
option(BUILD_AUDIO           "Enable audio module" ON)
option(BUILD_STORAGE         "Enable storage module (needs BUILD_UTILS as dependency)" ON)
option(BUILD_GRAPHICS        "Enable graphics module" ON)
option(BUILD_UTILS           "Enable utils module" ON)
option(BUILD_LIVECODING      "Enable livecoding (available only on Linux)" ON)

if(BUILD_FONT AND NOT BUILD_GRAPHICS)
	message(FATAL_ERROR "Cannot enable BUILD_FONT: BUILD_GRAPHICS is needed as a dependency")
endif()
if(BUILD_PARTICLE AND NOT BUILD_GRAPHICS)
	message(FATAL_ERROR "Cannot enable BUILD_PARTICLE: BUILD_GRAPHICS is needed as a dependency")
endif()
if(BUILD_STORAGE AND NOT BUILD_UTILS)
	message(FATAL_ERROR "Cannot enable BUILD_STORAGE: BUILD_UTILS is needed as a dependency")
endif()
if(BUILD_LIVECODING AND NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	message(FATAL_ERROR "Cannot enable BUILD_LIVECODING: Linux only")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON) # -fPIC

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
	set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -march=native")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -mtune=native")
endif()
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_CLANGC)
	set(CMAKE_C_FLAGS_DEBUG "-g -O0 -march=native")
	set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -mtune=native")
endif()

if(BUILD_MANPAGES)
	add_subdirectory(doc/man)
endif()
add_subdirectory(external)

include_directories(external)
include_directories(external/lua/src)

if(BUILD_PHYSICS)
	include_directories(external/box2d/Box2D)
endif()
if(BUILD_GRAPHICS)
	include_directories(external/libpng)
	include_directories(${CMAKE_CURRENT_BINARY_DIR}/external/) # for pngconf.h
endif()

add_subdirectory(src)
