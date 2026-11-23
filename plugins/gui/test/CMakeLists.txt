if(BUILD_TESTS)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests)

    add_executable(runTest-gui graph_context.cpp)

    target_link_libraries(runTest-gui gui Qt6::Widgets pthread gtest hal::core hal::netlist test_utils)

    add_test(runTest-gui ${CMAKE_BINARY_DIR}/bin/hal_plugins/runTest-gui --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(runTest-gui)
    endif()
endif()
