if (CMAKE_CXX_COMPILER MATCHES ".*clang")
	set(CMAKE_COMPILER_IS_CLANGXX 1)
endif()
if (CMAKE_C_COMPILER MATCHES ".*clang")
	set(CMAKE_COMPILER_IS_CLANGC  1)
endif()

add_custom_target(astyle COMMAND
    astyle --indent=tab --indent-preprocessor --style=kr
    --pad-oper --pad-header --unpad-paren -S -n
    --exclude="api.hpp" --exclude="api.cpp"
    --exclude="api.h" --exclude="api.c"
    --recursive ${CMAKE_SOURCE_DIR}"/src/*.cpp"
    ${CMAKE_SOURCE_DIR}"/src/*.hpp"
    ${CMAKE_SOURCE_DIR}"/src/*.c"
    ${CMAKE_SOURCE_DIR}"/src/*.h"
)

