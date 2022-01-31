option(PL_PERF_TEST "PL_PERF_TEST" OFF)
if(PL_PERF_TEST OR BUILD_ALL_PLUGINS)
    file(GLOB_RECURSE PERF_TEST_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE PERF_TEST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE PERF_TEST_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)

    enable_cxx_compile_option_if_supported("-O1" "Perf" "PUBLIC")
    enable_cxx_compile_option_if_supported("-g" "Perf" "PUBLIC")
    enable_cxx_compile_option_if_supported("-fno-inline-functions" "Perf" "PUBLIC")
    enable_cxx_compile_option_if_supported("-fno-inline-functions-called-once" "Perf" "PUBLIC")
    enable_cxx_compile_option_if_supported("-fno-optimize-sibling-calls" "Perf" "PUBLIC")

    hal_add_plugin(perf_test
                   SHARED
                   HEADER ${PERF_TEST_INC}
                   SOURCES ${PERF_TEST_SRC} ${PERF_TEST_PYTHON_SRC}
                   LINK_LIBRARIES PUBLIC netlist_simulator_controller
                   )
endif()
