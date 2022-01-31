option(PL_PERF_TEST "PL_PERF_TEST" OFF)
if(PL_PERF_TEST OR BUILD_ALL_PLUGINS)
    file(GLOB_RECURSE PERF_TEST_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE PERF_TEST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE PERF_TEST_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)

    hal_add_plugin(perf_test
                   SHARED
                   HEADER ${PERF_TEST_INC}
                   SOURCES ${PERF_TEST_SRC} ${PERF_TEST_PYTHON_SRC}
                   LINK_LIBRARIES PUBLIC netlist_simulator_controller
                   )
endif()
