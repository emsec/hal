if(BUILD_TESTS)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/plugins/solve_fsm/include)

    add_executable(runTest-solve_fsm solve_fsm.cpp)

    target_link_libraries(runTest-solve_fsm solve_fsm pthread gtest hal::core hal::netlist test_utils)

    add_test(runTest-solve_fsm ${CMAKE_BINARY_DIR}/bin/hal_plugins/runTest-solve_fsm --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(runTest-solve_fsm)
    endif()
endif()
