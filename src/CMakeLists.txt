include_directories(.)
aux_source_directory(. SOURCES)
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

include(GenerateLuaFiles)

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
	if(BUILD_AUDIO)
		pkg_search_module(OPENAL REQUIRED openal)
	endif()
	pkg_search_module(GL REQUIRED gl)

	include_directories(${SDL2_INCLUDE_DIRS})
	include_directories(${GL_INCLUDE_DIRS})
	if(BUILD_AUDIO)
		include_directories(${OPENAL_INCLUDE_DIRS})
	endif()

	target_link_libraries(drystal ${SDL2_LIBRARIES})
	target_link_libraries(drystal ${GL_LIBRARIES})
	if(BUILD_AUDIO)
		target_link_libraries(drystal ${OPENAL_LIBRARIES})
	endif()

    include(BuildCoverage)
endif()

target_link_libraries(${DRYSTAL_OUT} lua)
target_link_libraries(${DRYSTAL_OUT} lua-cjson)
target_link_libraries(${DRYSTAL_OUT} stb_image)
if(BUILD_AUDIO)
	target_link_libraries(${DRYSTAL_OUT} stb_vorbis)
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
	if(BUILD_WEB)
		target_link_libraries(${DRYSTAL_OUT} websocket)
	endif()
endif()

install(TARGETS ${DRYSTAL_OUT} DESTINATION /usr/bin)

