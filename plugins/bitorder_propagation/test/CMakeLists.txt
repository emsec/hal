if(BUILD_TESTS)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/plugins/bitorder_propagation/include)

    add_executable(runTest-bitorder_propagation bitorder_propagation.cpp)

    target_link_libraries(runTest-bitorder_propagation bitorder_propagation pthread gtest hal::core hal::netlist test_utils)

    add_test(runTest-bitorder_propagation ${CMAKE_BINARY_DIR}/bin/hal_plugins/runTest-bitorder_propagation --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(runTest-bitorder_propagation)
    endif()
endif()
