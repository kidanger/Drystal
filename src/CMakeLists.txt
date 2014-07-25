include_directories(.)
set(SOURCES
	engine.cpp
	lua_functions.cpp
	main.cpp
	util.c
)
if(BUILD_EVENT)
	add_definitions(-DBUILD_EVENT)
	aux_source_directory(event SOURCES)
endif()
if(BUILD_PHYSIC)
	add_definitions(-DBUILD_PHYSIC)
	aux_source_directory(physic SOURCES)
endif()
if(BUILD_FONT)
	add_definitions(-DBUILD_FONT)
	aux_source_directory(truetype SOURCES)
endif()
if(BUILD_PARTICLE)
	add_definitions(-DBUILD_PARTICLE)
	aux_source_directory(particle SOURCES)
endif()
if(BUILD_AUDIO)
	add_definitions(-DBUILD_AUDIO)
	aux_source_directory(audio SOURCES)
endif()
if(BUILD_NET)
	add_definitions(-DBUILD_NET)
	aux_source_directory(net SOURCES)
endif()
if(BUILD_WEB)
	add_definitions(-DBUILD_WEB)
	aux_source_directory(web SOURCES)
endif()
if(BUILD_STORAGE)
	add_definitions(-DBUILD_STORAGE)
	aux_source_directory(storage SOURCES)
endif()
if(BUILD_GRAPHICS)
	add_definitions(-DBUILD_GRAPHICS)
	aux_source_directory(graphics SOURCES)
endif()
if(BUILD_ENABLE_STATS)
	add_definitions(-DBUILD_ENABLE_STATS)
	list(APPEND SOURCES stats.cpp)
endif()
if(BUILD_UTILS)
	add_definitions(-DBUILD_UTILS)
	aux_source_directory(utils SOURCES)
endif()
if(NOT DEFINED EMSCRIPTEN AND BUILD_LIVECODING)
	add_definitions(-DBUILD_LIVECODING)
	list(APPEND SOURCES livecoding_linux.c)
endif()

include(GenerateLuaFiles)

set(DRYSTAL_OUT drystal)
if(EMSCRIPTEN)
    set(DRYSTAL_OUT drystal.js)
endif()

set(DRYSTAL_FLAGS " ")
set(DRYSTAL_CXX_FLAGS " ")
set(DRYSTAL_C_FLAGS " ")
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX OR CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_CLANGC)
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
-Werror")
# -Wcast-qual \
	if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
		set(DRYSTAL_FLAGS "${DRYSTAL_FLAGS} -Wlogical-op")
	endif()
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
    set(DRYSTAL_CXX_FLAGS "${DRYSTAL_CXX_FLAGS} -Weffc++ -std=c++11")
	if(NOT DEFINED EMSCRIPTEN AND BUILD_LIVECODING)
		# C++11 Needed by lua_functions for std::atomic<bool> for livecoding
		set(DRYSTAL_CXX_FLAGS "${DRYSTAL_CXX_FLAGS} -std=c++11")
	endif()
endif()

add_executable(${DRYSTAL_OUT} ${SOURCES})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DRYSTAL_FLAGS} ${DRYSTAL_CXX_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${DRYSTAL_FLAGS} ${DRYSTAL_C_FLAGS}")

if(NOT DEFINED EMSCRIPTEN)
	include(FindPkgConfig)

	# FIXME Engine always needs SDL for SDL_Delay
	pkg_search_module(SDL2 REQUIRED sdl2)
	include_directories(${SDL2_INCLUDE_DIRS})
	target_link_libraries(drystal ${SDL2_LIBRARIES})

	if(BUILD_GRAPHICS OR BUILD_FONT)
		pkg_search_module(GL REQUIRED gl)
		include_directories(${GL_INCLUDE_DIRS})
		target_link_libraries(drystal ${GL_LIBRARIES})
	endif()
	if(BUILD_AUDIO)
		pkg_search_module(OPENAL REQUIRED openal)
		include_directories(${OPENAL_INCLUDE_DIRS})
		target_link_libraries(drystal ${OPENAL_LIBRARIES})
	endif()

    include(BuildCoverage)
endif()

target_link_libraries(${DRYSTAL_OUT} lua)
if(BUILD_UTILS)
	target_link_libraries(${DRYSTAL_OUT} lua-cjson)
endif()
if(BUILD_GRAPHICS)
	target_link_libraries(${DRYSTAL_OUT} stb_image)
endif()
if(BUILD_AUDIO)
	target_link_libraries(${DRYSTAL_OUT} stb_vorbis)
	target_link_libraries(${DRYSTAL_OUT} wavloader)
endif()
if(BUILD_FONT)
	target_link_libraries(${DRYSTAL_OUT} stb_truetype)
endif()
if(BUILD_PHYSIC)
	target_link_libraries(${DRYSTAL_OUT} Box2D)
endif()
if(EMSCRIPTEN)
	target_link_libraries(${DRYSTAL_OUT} miniz)
else()
	if(BUILD_NET)
		target_link_libraries(${DRYSTAL_OUT} websocket)
	endif()
endif()

install(TARGETS ${DRYSTAL_OUT} DESTINATION /usr/bin)

