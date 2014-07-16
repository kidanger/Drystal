aux_source_directory(. SOURCES)

set(DRYSTAL_OUT drystal)
if(EMSCRIPTEN)
    set(DRYSTAL_OUT drystal.js)
endif()

if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ftrapv -Wall -Wextra -Wundef -Wunreachable-code -Wwrite-strings -Weffc++")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ftrapv -Wall -Wextra -Wundef -Wunreachable-code -Wwrite-strings -Wstrict-prototypes")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEBUG} -ftrapv -Wall -Wextra -Wundef -Wunreachable-code -Wwrite-strings -Weffc++")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_DEBUG} -ftrapv -Wall -Wextra -Wundef -Wunreachable-code -Wwrite-strings -Wstrict-prototypes")
endif()

add_executable(${DRYSTAL_OUT} ${SOURCES})

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

    if(BUILD_ENABLE_COVERAGE)
        find_program(COVERAGE_GCOV gcov)
        find_program(COVERAGE_LCOV lcov)
        find_program(COVERAGE_GENHTML genhtml)
        if (NOT COVERAGE_GCOV)
            message(FATAL_ERROR "Unable to find gcov")
        endif()
        if (NOT COVERAGE_LCOV)
            message(FATAL_ERROR "Unable to find lcov")
        endif()
        if (NOT COVERAGE_GENHTML)
            message(FATAL_ERROR "Unable to find genhtml")
        endif()
      
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
        set(CMAKE_LD_FLAGS_DEBUG "${CMAKE_LD_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
      
        add_custom_target(coverage-reset DEPENDS ${DRYSTAL_OUT})
        add_custom_command(TARGET coverage-reset
            COMMAND mkdir -p coverage
            COMMAND ${COVERAGE_LCOV} --directory . --zerocounters
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        )
        add_custom_target(coverage-report DEPENDS ${DRYSTAL_OUT})
        add_custom_command(TARGET coverage-report
            COMMAND ${COVERAGE_LCOV} --directory . --capture --output-file ./coverage/drystal.lcov
            COMMAND ${COVERAGE_LCOV} --remove ./coverage/drystal.lcov --output-file ./coverage/drystal_clean.lcov '/usr/*'
            COMMAND ${COVERAGE_GENHTML} -t "drystal coverage" -p "${CMAKE_SOURCE_DIR}" -o ./coverage ./coverage/drystal_clean.lcov
            COMMAND echo "Open ${CMAKE_BINARY_DIR}/coverage/index.html to view the coverage analysis results."
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        )
        target_link_libraries(${DRYSTAL_OUT} gcov)
    endif(BUILD_ENABLE_COVERAGE)
endif()

target_link_libraries(${DRYSTAL_OUT} lua-drystal)
target_link_libraries(${DRYSTAL_OUT} lua-cjson)
if(EMSCRIPTEN)
    target_link_libraries(${DRYSTAL_OUT} physic truetype particle net web)
endif(EMSCRIPTEN)

install(TARGETS ${DRYSTAL_OUT} DESTINATION /usr/bin)
