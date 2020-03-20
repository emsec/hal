include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests)

add_executable(runTest-netlist
        netlist.cpp)
add_executable(runTest-gate
        gate.cpp)
add_executable(runTest-net
        net.cpp)
add_executable(runTest-data_container
        data_container.cpp)
add_executable(runTest-endpoint
        endpoint.cpp)
add_executable(runTest-netlist_internal_manager
        netlist_internal_manager.cpp)
add_executable(runTest-module
        module.cpp)
add_executable(runTest-netlist_factory
        netlist_factory.cpp)
add_executable(runTest-gate_library_manager
        gate_library_manager.cpp)
add_executable(runTest-netlist_serializer
        netlist_serializer.cpp)
add_executable(runTest-boolean_function
        boolean_function.cpp)
add_executable(runTest-gate_library
        gate_library.cpp)
add_executable(runTest-gate_library_parser_liberty
        gate_library_parser_liberty.cpp)


target_link_libraries(runTest-netlist    pthread gtest gtest_main hal::core hal::netlist  test_utils)
target_link_libraries(runTest-gate    pthread gtest gtest_main hal::core hal::netlist test_utils)
target_link_libraries(runTest-net    pthread gtest gtest_main hal::core hal::netlist test_utils)
target_link_libraries(runTest-data_container    pthread gtest gtest_main hal::core hal::netlist test_utils)
target_link_libraries(runTest-endpoint   pthread gtest gtest_main hal::core hal::netlist test_utils)
target_link_libraries(runTest-netlist_internal_manager   pthread gtest gtest_main hal::core hal::netlist test_utils)
target_link_libraries(runTest-module   pthread gtest gtest_main hal::core hal::netlist test_utils)
target_link_libraries(runTest-netlist_factory   pthread gtest gtest_main hal::core hal::netlist test_utils)
target_link_libraries(runTest-gate_library_manager   pthread gtest gtest_main hal::core hal::netlist test_utils)
target_link_libraries(runTest-netlist_serializer   pthread gtest gtest_main hal::core hal::netlist test_utils)
target_link_libraries(runTest-boolean_function   pthread gtest gtest_main hal::core hal::netlist test_utils)
target_link_libraries(runTest-gate_library   pthread gtest gtest_main hal::core hal::netlist test_utils)
target_link_libraries(runTest-gate_library_parser_liberty   pthread gtest gtest_main hal::core hal::netlist test_utils)

add_test(runTest-netlist ${CMAKE_BINARY_DIR}/bin/runTest-netlist --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
add_test(runTest-gate ${CMAKE_BINARY_DIR}/bin/runTest-gate --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
add_test(runTest-net ${CMAKE_BINARY_DIR}/bin/runTest-net --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
add_test(runTest-data_container ${CMAKE_BINARY_DIR}/bin/runTest-data_container --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
add_test(runTest-endpoint ${CMAKE_BINARY_DIR}/bin/runTest-endpoint --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
add_test(runTest-netlist_internal_manager ${CMAKE_BINARY_DIR}/bin/runTest-netlist_internal_manager --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
add_test(runTest-module ${CMAKE_BINARY_DIR}/bin/runTest-module --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
add_test(runTest-netlist_factory ${CMAKE_BINARY_DIR}/bin/runTest-netlist_factory --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
add_test(runTest-gate_library_manager ${CMAKE_BINARY_DIR}/bin/runTest-gate_library_manager --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
add_test(runTest-netlist_serializer ${CMAKE_BINARY_DIR}/bin/runTest-netlist_serializer --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
add_test(runTest-boolean_function ${CMAKE_BINARY_DIR}/bin/runTest-boolean_function --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
add_test(runTest-gate_library ${CMAKE_BINARY_DIR}/bin/runTest-gate_library --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
add_test(runTest-gate_library_parser_liberty ${CMAKE_BINARY_DIR}/bin/runTest-gate_library_parser_liberty --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)

