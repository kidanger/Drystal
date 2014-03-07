add_definitions(-DDRYSTAL_EXTENSION)

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
SET(CMAKE_SHARED_LIBRARY_PREFIX "")

add_subdirectory(truetype)
add_subdirectory(physic)
add_subdirectory(net)
add_subdirectory(web)
add_subdirectory(particle)
