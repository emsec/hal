if(BUILD_TESTS)
    if(HAL_PLUGINS_MASTER_PROJECT)
        project(tests)

        # Download and unpack googletest at configure time
        configure_file(googletest-CMakeLists.txt.in "${CMAKE_BINARY_DIR}/googletest-download/CMakeLists.txt")
        execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
                        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download" )
        execute_process(COMMAND "${CMAKE_COMMAND}" --build .
                        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download" )

        # Prevent GoogleTest from overriding our compiler/linker options
        # when building with Visual Studio
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

        # Add googletest directly to our build. This adds
        # the following targets: gtest, gtest_main, gmock
        # and gmock_main
        add_subdirectory("${CMAKE_BINARY_DIR}/googletest-src"
                        "${CMAKE_BINARY_DIR}/googletest-build")

        # The gtest/gmock targets carry header search path
        # dependencies automatically when using CMake 2.8.11 or
        # later. Otherwise we have to add them here ourselves.
        if(CMAKE_VERSION VERSION_LESS 2.8.11)
        include_directories("${gtest_SOURCE_DIR}/include"
                                "${gmock_SOURCE_DIR}/include")
        endif()
    endif()

    #copy files to cmake binary dir
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/netlists/counter/dump.vcd ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/counter/dump.vcd COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/netlists/counter/counternetlist_flattened_by_hal.vhd ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/counter/counternetlist_flattened_by_hal.vhd COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/netlists/half_adder/dump.vcd ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/half_adder/dump.vcd COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/netlists/half_adder/halfaddernetlist_flattened_by_hal.v ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/half_adder/halfaddernetlist_flattened_by_hal.v COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/netlists/toycipher/dump.vcd ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/toycipher/dump.vcd COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/netlists/toycipher/cipher_flat.vhd ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/toycipher/cipher_flat.vhd COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/netlists/sha256/dump.vcd ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/sha256/dump.vcd COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/netlists/sha256/sha256_flat.vhd ${CMAKE_BINARY_DIR}/bin/hal_plugins/test-files/sha256/sha256_flat.vhd COPYONLY)


    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/plugins/netlist_simulator/include)

    add_executable(runTest-netlist_simulator simulator_test.cpp)

    target_link_libraries(runTest-netlist_simulator netlist_simulator test_utils gtest gtest_main ${LINK_LIBS})

    add_test(runTest-netlist_simulator ${CMAKE_BINARY_DIR}/bin/hal_plugins/runTest-netlist_simulator --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runTest-netlist_simulator.xml)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(runTest-netlist_simulator)
    endif()
endif()
