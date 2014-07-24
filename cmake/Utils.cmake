add_custom_target(astyle COMMAND
    astyle --indent=tab --indent-preprocessor --style=kr
    --pad-oper --pad-header --unpad-paren -S -n
    ${CMAKE_SOURCE_DIR}"/src/*.cpp"
    ${CMAKE_SOURCE_DIR}"/src/*.hpp"
)

