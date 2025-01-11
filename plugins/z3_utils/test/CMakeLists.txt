if(BUILD_TESTS)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/plugins/z3_utils/include)

    add_executable(runTest-z3_utils  z3_utils.cpp)

    target_link_libraries(runTest-z3_utils  z3_utils gtest hal::core hal::netlist test_utils)

    add_test(runTest-z3_utils ${CMAKE_BINARY_DIR}/bin/hal_plugins/runTest-z3_utils --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(runTest-z3_utils)
    endif()

endif()