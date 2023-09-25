if(BUILD_TESTS)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/plugins/verilog_parser/include)

    add_executable(runTest-verilog_parser  verilog_parser.cpp)

    target_link_libraries(runTest-verilog_parser  verilog_parser gtest hal::core hal::netlist test_utils)

    add_test(runTest-verilog_parser ${CMAKE_BINARY_DIR}/bin/hal_plugins/runTest-verilog_parser --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(runTest-verilog_parser)
    endif()

endif()