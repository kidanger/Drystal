set(BOX2D_BUILD_STATIC ON CACHE BOOL "")
set(BOX2D_BUILD_EXAMPLES OFF CACHE BOOL "")
set(BOX2D_INSTALL_BY_DEFAULT OFF CACHE BOOL "")
add_subdirectory(box2d/Box2D)

include_directories(box2d/Box2D)
add_library(physic SHARED main.cpp)
target_link_libraries(physic Box2D)
