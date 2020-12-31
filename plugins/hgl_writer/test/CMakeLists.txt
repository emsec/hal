if(BUILD_TESTS)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/hgl_writer)

    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/plugins/hgl_writer/include)

    add_executable(runTest-hgl_writer  hgl_writer.cpp)

    target_link_libraries(runTest-hgl_writer hgl_writer hgl_parser pthread gtest gtest_main hal::core hal::netlist test_utils)

    add_test(runTest-hgl_writer ${CMAKE_BINARY_DIR}/bin/hal_plugins/runTest-hgl_writer --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(runTest-hgl_writer)
    endif()
endif()