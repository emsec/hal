if(BUILD_TESTS AND (PL_VHDL_VERILOG_PARSERS OR BUILD_ALL_PLUGINS))
    include_directories(
            ${gtest_SOURCE_DIR}/include
            ${gtest_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/tests
            ${CMAKE_SOURCE_DIR}/plugins/vhdl_verilog_writers/include
            ${CMAKE_SOURCE_DIR}/plugins/vhdl_verilog_parsers/include
    )

    add_executable(runTest-hdl_writer_verilog  hdl_writer_verilog.cpp)
    add_executable(runTest-hdl_writer_vhdl  hdl_writer_vhdl.cpp)

    target_link_libraries(runTest-hdl_writer_verilog   vhdl_verilog_writers vhdl_verilog_parsers pthread gtest gtest_main hal::core hal::netlist test_utils)
    target_link_libraries(runTest-hdl_writer_vhdl  vhdl_verilog_writers vhdl_verilog_parsers pthread gtest gtest_main hal::core hal::netlist test_utils)

    add_test(runTest-hdl_writer_verilog ${CMAKE_BINARY_DIR}/bin/hal_plugins/runTest-hdl_writer_verilog --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
    add_test(runTest-hdl_writer_vhdl ${CMAKE_BINARY_DIR}/bin/hal_plugins/runTest-hdl_writer_vhdl --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(runTest-hdl_writer_vhdl)
        add_sanitizers(runTest-hdl_writer_verilog)
    endif()

endif()