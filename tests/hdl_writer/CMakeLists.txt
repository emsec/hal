include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests)

# add_executable(runTest-hdl_writer_vhdl  hdl_writer_vhdl.cpp)
# target_link_libraries(runTest-hdl_writer_vhdl    pthread gtest gtest_main hal::core hal::netlist test_utils)
# add_test(runTest-hdl_writer_vhdl ${CMAKE_BINARY_DIR}/bin/runTest-hdl_writer_vhdl --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)


# add_executable(runTest-hdl_writer_verilog  hdl_writer_verilog.cpp)
# target_link_libraries(runTest-hdl_writer_verilog    pthread gtest gtest_main hal::core hal::netlist test_utils)
# add_test(runTest-hdl_writer_verilog ${CMAKE_BINARY_DIR}/bin/runTest-hdl_writer_verilog --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)


add_executable(runTest-hdl_writer_manager  hdl_writer_manager.cpp)
target_link_libraries(runTest-hdl_writer_manager    pthread gtest gtest_main hal::core hal::netlist test_utils)
add_test(runTest-hdl_writer_manager ${CMAKE_BINARY_DIR}/bin/runTest-hdl_writer_manager --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
