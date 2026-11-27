if(BUILD_TESTS)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/plugins/dataflow_analysis/include)

    add_executable(runTest-dataflow dataflow.cpp)

    target_link_libraries(runTest-dataflow dataflow pthread gtest hal::core hal::netlist test_utils)

    add_test(runTest-dataflow ${CMAKE_BINARY_DIR}/bin/hal_plugins/runTest-dataflow --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(runTest-dataflow)
    endif()
endif()
