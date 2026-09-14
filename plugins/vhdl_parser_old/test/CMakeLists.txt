if(BUILD_TESTS)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/plugins/vhdl_parser_old/include)

    add_executable(runTest-vhdl_parser_old  vhdl_parser_old.cpp)

    target_link_libraries(runTest-vhdl_parser_old  vhdl_parser_old pthread gtest hal::core hal::netlist test_utils)

    add_test(runTest-vhdl_parser_old ${CMAKE_BINARY_DIR}/bin/hal_plugins/runTest-vhdl_parser_old --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(runTest-vhdl_parser_old)
    endif()

endif()