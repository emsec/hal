if(BUILD_TESTS)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/plugins/xilinx_toolbox/include ${CMAKE_SOURCE_DIR}/plugins/hgl_parser/include)

    add_executable(runTest-xilinx_toolbox xilinx_toolbox.cpp)

    target_link_libraries(runTest-xilinx_toolbox xilinx_toolbox hgl_parser pthread gtest hal::core hal::netlist test_utils)

    add_test(runTest-xilinx_toolbox ${CMAKE_BINARY_DIR}/bin/hal_plugins/runTest-xilinx_toolbox --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(runTest-xilinx_toolbox)
    endif()
endif()
