include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests)

add_executable(runTest-hdl_writer_vhdl
        hdl_writer_vhdl.cpp)


target_link_libraries(runTest-hdl_writer_vhdl   gtest gtest_main ${LINK_LIBS})


add_test(runTest-hdl_writer_vhdl ${CMAKE_BINARY_DIR}/bin/runTest-hdl_writer_vhdl --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
