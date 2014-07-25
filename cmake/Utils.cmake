add_custom_target(astyle COMMAND
    astyle --indent=tab --indent-preprocessor --style=kr
    --pad-oper --pad-header --unpad-paren -S -n
    --exclude="api.hpp api.cpp"
    --recursive ${CMAKE_SOURCE_DIR}"/src/*.cpp"
    ${CMAKE_SOURCE_DIR}"/src/*.hpp"
    ${CMAKE_SOURCE_DIR}"/src/*.c"
    ${CMAKE_SOURCE_DIR}"/src/*.h"
)

