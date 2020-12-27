if(BUILD_TESTS)
    #copy files to cmake binary dir
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gate_libraries/test1.lib ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/liberty_parser/test1.lib COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gate_libraries/test2.lib ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/liberty_parser/test2.lib COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gate_libraries/test3.lib ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/liberty_parser/test3.lib COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gate_libraries/test4.lib ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/liberty_parser/test4.lib COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gate_libraries/test5.lib ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/liberty_parser/test5.lib COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gate_libraries/invalid_test1.lib ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/liberty_parser/invalid_test1.lib COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gate_libraries/invalid_test2.lib ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/liberty_parser/invalid_test2.lib COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gate_libraries/invalid_test3.lib ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/liberty_parser/invalid_test3.lib COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gate_libraries/invalid_test4.lib ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/liberty_parser/invalid_test4.lib COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gate_libraries/invalid_test5.lib ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/liberty_parser/invalid_test5.lib COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gate_libraries/invalid_test6.lib ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/liberty_parser/invalid_test6.lib COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gate_libraries/invalid_test7.lib ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/liberty_parser/invalid_test7.lib COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gate_libraries/invalid_test8.lib ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/liberty_parser/invalid_test8.lib COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gate_libraries/invalid_test9.lib ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/liberty_parser/invalid_test9.lib COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gate_libraries/invalid_test10.lib ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/liberty_parser/invalid_test10.lib COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gate_libraries/invalid_test11.lib ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/liberty_parser/invalid_test11.lib COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gate_libraries/invalid_test12.lib ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/liberty_parser/invalid_test12.lib COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gate_libraries/invalid_test13.lib ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/liberty_parser/invalid_test13.lib COPYONLY)

    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/plugins/liberty_parser/include)

    add_executable(runTest-liberty_parser  liberty_parser.cpp)

    target_link_libraries(runTest-liberty_parser   liberty_parser pthread gtest gtest_main hal::core hal::netlist test_utils)

    add_test(runTest-liberty_parser ${CMAKE_BINARY_DIR}/bin/hal_plugins/runTest-liberty_parser --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(runTest-liberty_parser)
    endif()
endif()