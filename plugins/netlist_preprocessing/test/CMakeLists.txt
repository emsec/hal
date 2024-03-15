if(BUILD_TESTS)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/plugins/netlist_preprocessing/include)

    add_executable(runTest-netlist_preprocessing netlist_preprocessing.cpp)

    target_link_libraries(runTest-netlist_preprocessing netlist_preprocessing gtest hal::core hal::netlist test_utils)

    add_test(runTest-netlist_preprocessing ${CMAKE_BINARY_DIR}/bin/hal_plugins/runTest-netlist_preprocessing --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(runTest-netlist_preprocessing)
    endif()
endif()