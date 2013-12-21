CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

set(CMAKE_POSITION_INDEPENDENT_CODE ON) # -fPIC
if(EMSCRIPTEN)
set(CMAKE_USE_RELATIVE_PATHS ON)
set(LINK_FLAGS "-O2 --switchify -s ASM_JS=1 -s DLOPEN_SUPPORT=1 -s FORCE_ALIGNED_MEMORY=1 -s RELOOP=1 -s OUTLINING_LIMIT=10000 -s DISABLE_EXCEPTION_CATCHING=1 -s TOTAL_MEMORY=67108864")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINK_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LINK_FLAGS}")
endif(EMSCRIPTEN)

project(drystal)

add_subdirectory(external)

include_directories(external)
include_directories(external/lua)
include_directories(external/lua/src)

add_subdirectory(src)

include_directories(src)
add_subdirectory(extensions)

