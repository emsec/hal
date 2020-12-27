if(BUILD_TESTS)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/plugins/hgl_parser/include)

    add_executable(runTest-hgl_parser  hgl_parser.cpp)

    target_link_libraries(runTest-hgl_parser   hgl_parser pthread gtest gtest_main hal::core hal::netlist test_utils)

    add_test(runTest-hgl_parser ${CMAKE_BINARY_DIR}/bin/hal_plugins/runTest-hgl_parser --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(runTest-hgl_parser)
    endif()
endif()