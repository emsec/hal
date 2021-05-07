include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests)

add_executable(runTest-netlist_writer_manager  netlist_writer_manager.cpp)
target_link_libraries(runTest-netlist_writer_manager    pthread gtest hal::core hal::netlist test_utils)
add_test(runTest-netlist_writer_manager ${CMAKE_BINARY_DIR}/bin/runTest-netlist_writer_manager --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
add_sanitizers(runTest-netlist_writer_manager)
endif()
