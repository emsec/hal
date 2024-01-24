# if(BUILD_TESTS)
#     include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/plugins/module_idewuzzla_test/include)

#     add_executable(runTest-runTest-time runTest-time.cpp)
    

#     target_link_libraries(runTest-runTest-time runTest-time gtest hal::core hal::netlist test_utils)

#     add_test(runTest-time ${CMAKE_BINARY_DIR}/bin/hal_plugins/runTest-time --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
    



#     if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
#         add_sanitizers(runTest-module_identification)
#     endif()
# endif()