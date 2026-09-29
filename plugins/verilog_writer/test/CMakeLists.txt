if(BUILD_TESTS AND ((PL_VERILOG_PARSER) OR BUILD_ALL_PLUGINS))
    include_directories(
            ${gtest_SOURCE_DIR}/include
            ${gtest_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/tests
            ${CMAKE_SOURCE_DIR}/plugins/verilog_writer/include
    )

    add_executable(runTest-verilog_writer  verilog_writer.cpp)

    target_link_libraries(runTest-verilog_writer   verilog_writer verilog_parser pthread gtest hal::core hal::netlist test_utils)

    add_test(runTest-verilog_writer ${CMAKE_BINARY_DIR}/bin/hal_plugins/runTest-verilog_writer --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(runTest-verilog_writer)
    endif()

endif()