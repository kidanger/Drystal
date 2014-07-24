include_directories(.)
aux_source_directory(. SOURCES)
aux_source_directory(event SOURCES)
aux_source_directory(physic SOURCES)
aux_source_directory(truetype SOURCES)
aux_source_directory(particle SOURCES)
aux_source_directory(audio SOURCES)
aux_source_directory(net SOURCES)
aux_source_directory(web SOURCES)
aux_source_directory(storage SOURCES)

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
    pkg_search_module(OPENAL REQUIRED openal)
    pkg_search_module(GL REQUIRED gl)

    include_directories(${SDL2_INCLUDE_DIRS})
    include_directories(${OPENAL_INCLUDE_DIRS})
    include_directories(${GL_INCLUDE_DIRS})

    target_link_libraries(drystal ${SDL2_LIBRARIES})
    target_link_libraries(drystal ${OPENAL_LIBRARIES})
    target_link_libraries(drystal ${GL_LIBRARIES})

    include(BuildCoverage)
endif()

target_link_libraries(${DRYSTAL_OUT} stb_vorbis)
target_link_libraries(${DRYSTAL_OUT} stb_image)
target_link_libraries(${DRYSTAL_OUT} stb_truetype)
target_link_libraries(${DRYSTAL_OUT} lua)
target_link_libraries(${DRYSTAL_OUT} lua-cjson)
target_link_libraries(${DRYSTAL_OUT} Box2D)
if(EMSCRIPTEN)
    target_link_libraries(${DRYSTAL_OUT} miniz)
else()
    target_link_libraries(${DRYSTAL_OUT} websocket)
endif()

install(TARGETS ${DRYSTAL_OUT} DESTINATION /usr/bin)

