if(BUILD_TESTS)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/plugins/vhdl_verilog_parsers/include)

    add_executable(runTest-hdl_parser_verilog  hdl_parser_verilog.cpp)
    add_executable(runTest-hdl_parser_vhdl  hdl_parser_vhdl.cpp)

    target_link_libraries(runTest-hdl_parser_verilog   vhdl_verilog_parsers pthread gtest hal::core hal::netlist test_utils)
    target_link_libraries(runTest-hdl_parser_vhdl  vhdl_verilog_parsers pthread gtest hal::core hal::netlist test_utils)

    add_test(runTest-hdl_parser_verilog ${CMAKE_BINARY_DIR}/bin/hal_plugins/runTest-hdl_parser_verilog --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
    add_test(runTest-hdl_parser_vhdl ${CMAKE_BINARY_DIR}/bin/hal_plugins/runTest-hdl_parser_vhdl --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(runTest-hdl_parser_vhdl)
        add_sanitizers(runTest-hdl_parser_verilog)
    endif()

endif()