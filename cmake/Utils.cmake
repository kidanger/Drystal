include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

function(check_compilers_unknown)
	if (NOT CMAKE_CXX_COMPILER)
		message(FATAL_ERROR "The C++ compiler was not found. Please add the C++ compiler in your $PATH and re-run CMake.")
	endif()
	if (NOT CMAKE_C_COMPILER)
		message(FATAL_ERROR "The C compiler was not found. Please add the C++ compiler in your $PATH and re-run CMake.")
	endif()
endfunction()

function(check_git_submodules_initialized)
	if (NOT EXISTS "${PROJECT_SOURCE_DIR}/external/box2d/.git"     OR
	    NOT EXISTS "${PROJECT_SOURCE_DIR}/external/libpng/.git"    OR
	    NOT EXISTS "${PROJECT_SOURCE_DIR}/external/lua/.git"       OR
	    NOT EXISTS "${PROJECT_SOURCE_DIR}/external/lua-cjson/.git" OR
	    NOT EXISTS "${PROJECT_SOURCE_DIR}/external/zlib/.git")
		message(FATAL_ERROR "The git submodules are not available. Please run
		git submodule update --init --recursive")
	endif()
endfunction()

function(add_c_flag_if_supported flags flag has)
        check_c_compiler_flag("${flag}" ${has}_C)
	if (${${has}_C})
		set(${has} TRUE PARENT_SCOPE)
		set(${flags} "${${flags}} ${flag}" PARENT_SCOPE)
	endif()
endfunction()

function(add_cxx_flag_if_supported flags flag has)
        check_cxx_compiler_flag("${flag}" ${has}_CXX)
	if (${${has}_CXX})
		set(${has} TRUE PARENT_SCOPE)
		set(${flags} "${${flags}} ${flag}" PARENT_SCOPE)
	endif()
endfunction()

function(add_c_cxx_flag_if_supported flags flag has)
        check_c_compiler_flag("${flag}" ${has}_C)
	if (${${has}_C})
		check_cxx_compiler_flag("${flag}" ${has}_CXX)
		if (${${has}_CXX})
			set(${has} TRUE PARENT_SCOPE)
			set(${flags} "${${flags}} ${flag}" PARENT_SCOPE)
		endif()
	endif()
endfunction()

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

