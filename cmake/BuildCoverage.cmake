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
        COMMAND ${COVERAGE_LCOV} --remove ./coverage/drystal.lcov --output-file ./coverage/drystal_clean.lcov '/usr/*' 'box2d/*'
        COMMAND ${COVERAGE_GENHTML} -t "drystal coverage" -p "${CMAKE_SOURCE_DIR}" -o ./coverage ./coverage/drystal_clean.lcov
        COMMAND echo "Open ${CMAKE_BINARY_DIR}/coverage/index.html to view the coverage analysis results."
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
    target_link_libraries(${DRYSTAL_OUT} gcov)
endif(BUILD_ENABLE_COVERAGE)

