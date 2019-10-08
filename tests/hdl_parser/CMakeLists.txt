include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests)

add_executable(runTest-hdl_parser_vhdl_old
        hdl_parser_vhdl_old.cpp)
add_executable(runTest-hdl_parser_verilog_old
        hdl_parser_verilog_old.cpp)


target_link_libraries(runTest-hdl_parser_vhdl_old    gtest gtest_main ${LINK_LIBS})
target_link_libraries(runTest-hdl_parser_verilog_old gtest gtest_main ${LINK_LIBS})


add_test(runTest-hdl_parser_vhdl_old ${CMAKE_BINARY_DIR}/bin/runTest-hdl_parser_vhdl_old --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
add_test(runTest-hdl_parser_verilog_old ${CMAKE_BINARY_DIR}/bin/runTest-hdl_parser_verilog_old --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
