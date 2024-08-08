if(BUILD_TESTS)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/plugins/module_identification/include)

    add_executable(runTest-module_identification module_identification.cpp)
    
    add_executable(runTest-module_identification_check module_identification_check.cpp)

    target_link_libraries(runTest-module_identification module_identification gtest hal::core hal::netlist test_utils)
    target_link_libraries(runTest-module_identification_check module_identification gtest hal::core hal::netlist test_utils)

    add_test(runTest-module_identification ${CMAKE_BINARY_DIR}/bin/hal_plugins/runTest-module_identification --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
    
    add_test(runTest-module_identification_check ${CMAKE_BINARY_DIR}/bin/hal_plugins/runTest-module_identification_check --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)

    # xilinx
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_xilinx/ADDER_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/ADDER_1.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_xilinx/ADDER_WEIRD_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/ADDER_WEIRD_1.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_xilinx/ADDER_WITH_MUX_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/ADDER_WITH_MUX_1.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_xilinx/SUBTRACT_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/SUBTRACT_1.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_xilinx/COMPARISON_LET_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/COMPARISON_LET_1.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_xilinx/COMPARISON_LET_ENABLE_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/COMPARISON_LET_ENABLE_1.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_xilinx/ADDER_CARRY_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/ADDER_CARRY_1.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_xilinx/ADDER_CARRY_ENABLE_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/ADDER_CARRY_ENABLE_1.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_xilinx/ADDER_WITH_DIFFERENT_LENGTHS_OPERANDS_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/ADDER_WITH_DIFFERENT_LENGTHS_OPERANDS_1.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_xilinx/SUBTRACT_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/SUBTRACT_1.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_xilinx/SUBTRACT_2.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/SUBTRACT_2.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_xilinx/COMPARISON_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/COMPARISON_1.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_xilinx/COMPARISON_2.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/COMPARISON_2.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_xilinx/COMPARISON_EQ_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/COMPARISON_EQ_1.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_xilinx/COMPARISON_EQ_ENABLE_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/COMPARISON_EQ_ENABLE_1.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_xilinx/CONST_MUL_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/CONST_MUL_1.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_xilinx/COUNTER_INV_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/COUNTER_INV_1.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_xilinx/COUNTER_INV_2.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/COUNTER_INV_2.v COPYONLY)
    
    # lattice
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_lattice/CONST_MUL_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_lattice/CONST_MUL_1.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_lattice/COUNTER_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_lattice/COUNTER_1.v COPYONLY)

    # nangate
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_nangate/ADDER_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_nangate/ADDER_1.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_nangate/ADDER_CARRY_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_nangate/ADDER_CARRY_1.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_nangate/ADDER_CARRY_ENABLE_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_nangate/ADDER_CARRY_ENABLE_1.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_nangate/ADDER_CARRY_SIGN_EXTEND_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_nangate/ADDER_CARRY_SIGN_EXTEND_1.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_nangate/ADDER_CARRY_SIGN_EXTEND_TWO_ENABLE_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_nangate/ADDER_CARRY_SIGN_EXTEND_TWO_ENABLE_1.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_nangate/ADDER_TWO_ENABLE_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_nangate/ADDER_TWO_ENABLE_1.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_nangate/COUNTER_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_nangate/COUNTER_1.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_nangate/COUNTER_ENABLE_RESET_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_nangate/COUNTER_ENABLE_RESET_1.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_nangate/SUBTRACTION_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_nangate/SUBTRACTION_1.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_lib_nangate/SUBTRACTION_ENABLE_1.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/test_lib_nangate/SUBTRACTION_ENABLE_1.v COPYONLY)



    # testslibs for verification
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xilinx/ibex.hal ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/xilinx/ibex.hal COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xilinx/simple_risc_v.hal ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/xilinx/simple_risc_v.hal COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xilinx/icicle.hal ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/xilinx/icicle.hal COPYONLY)
    
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lattice/ibex.hal ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/lattice/ibex.hal COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lattice/simple_risc_v.hal ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/lattice/simple_risc_v.hal COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lattice/icicle.hal ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/lattice/icicle.hal COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lattice/fft64.hal ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/lattice/fft64.hal COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lattice/sha256.hal ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/module_identification/lattice/sha256.hal COPYONLY)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(runTest-module_identification)
    endif()
endif()