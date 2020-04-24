include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests)

add_executable(runTest-hdl_parser_vhdl
        hdl_parser_vhdl.cpp)
add_executable(runTest-hdl_parser_verilog
        hdl_parser_verilog.cpp)
add_executable(runTest-hdl_parser_dispatcher
        hdl_parser_dispatcher.cpp)

target_link_libraries(runTest-hdl_parser_vhdl   pthread gtest gtest_main hal::core hal::netlist test_utils)
target_link_libraries(runTest-hdl_parser_verilog    pthread gtest gtest_main hal::core hal::netlist test_utils)
target_link_libraries(runTest-hdl_parser_dispatcher    pthread gtest gtest_main hal::core hal::netlist test_utils)


add_test(runTest-hdl_parser_vhdl ${CMAKE_BINARY_DIR}/bin/runTest-hdl_parser_vhdl --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
add_test(runTest-hdl_parser_verilog ${CMAKE_BINARY_DIR}/bin/runTest-hdl_parser_verilog --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
add_test(runTest-hdl_parser_dispatcher ${CMAKE_BINARY_DIR}/bin/runTest-hdl_parser_dispatcher --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
