if(BUILD_TESTS)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/plugins/clock_tree_extractor/include ${CMAKE_SOURCE_DIR}/plugins/graph_algorithm/include)

    add_executable(runTest-clock_tree_extractor clock_tree_extractor.cpp)

    target_link_libraries(runTest-clock_tree_extractor clock_tree_extractor graph_algorithm pthread gtest hal::core hal::netlist test_utils)

    add_test(runTest-clock_tree_extractor ${CMAKE_BINARY_DIR}/bin/hal_plugins/runTest-clock_tree_extractor --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(runTest-clock_tree_extractor)
    endif()
endif()
