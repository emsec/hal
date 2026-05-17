if(BUILD_TESTS)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/plugins/xilinx_toolbox/include)

    add_executable(runTest-xilinx_toolbox xilinx_preprocessing.cpp)

    target_link_libraries(runTest-xilinx_toolbox xilinx_toolbox pthread gtest hal::core hal::netlist test_utils)

    add_test(runTest-xilinx_toolbox ${CMAKE_BINARY_DIR}/bin/hal_plugins/runTest-xilinx_toolbox --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-xilinx_toolbox.xml)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(runTest-xilinx_toolbox)
    endif()
endif()
