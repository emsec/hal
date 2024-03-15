if(BUILD_TESTS)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/plugins/bitwuzla_utils/include)

    add_executable(runTest-bitwuzla_utils bitwuzla_utils.cpp)

    target_link_libraries(runTest-bitwuzla_utils bitwuzla_utils gtest hal::core hal::netlist test_utils)

    add_test(runTest-bitwuzla_utils ${CMAKE_BINARY_DIR}/bin/hal_plugins/runTest-bitwuzla_utils --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(runTest-bitwuzla_utils)
    endif()
endif()