include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests)

add_executable(runTest-hdl_parser_vhdl
        hdl_parser_vhdl.cpp)
add_executable(runTest-hdl_parser_verilog
        hdl_parser_verilog.cpp)

target_link_libraries(runTest-hdl_parser_vhdl   gtest gtest_main hal::core hal::netlist test_utils)
target_link_libraries(runTest-hdl_parser_verilog   gtest gtest_main hal::core hal::netlist test_utils)


add_test(runTest-hdl_parser_vhdl ${CMAKE_BINARY_DIR}/bin/runTest-hdl_parser_vhdl --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
add_test(runTest-hdl_parser_verilog ${CMAKE_BINARY_DIR}/bin/runTest-hdl_parser_verilog --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
