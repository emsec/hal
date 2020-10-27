if(BUILD_TESTS)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/plugins/liberty_parser/include)

    add_executable(runTest-liberty_parser  liberty_parser.cpp)

    target_link_libraries(runTest-liberty_parser   liberty_parser pthread gtest gtest_main hal::core hal::netlist test_utils)

    add_test(runTest-liberty_parser ${CMAKE_BINARY_DIR}/bin/hal_plugins/runTest-liberty_parser --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(runTest-liberty_parser)
    endif()
endif()