include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests)

add_executable(runTest-hdl_parser_manager  hdl_parser_manager.cpp)
target_link_libraries(runTest-hdl_parser_manager    pthread gtest hal::core hal::netlist test_utils)
add_test(runTest-hdl_parser_manager ${CMAKE_BINARY_DIR}/bin/runTest-hdl_parser_manager --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
add_sanitizers(runTest-hdl_parser_manager)
endif()
