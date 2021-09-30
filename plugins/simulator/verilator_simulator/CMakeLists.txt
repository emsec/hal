option(PL_VERILATOR_SIMULATOR "PL_VERILATOR_SIMULATOR" OFF)

if(PL_VERILATOR_SIMULATOR OR PL_SIMULATOR OR BUILD_ALL_PLUGINS)

    file(GLOB_RECURSE VERILATOR_SIMULATOR_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE VERILATOR_SIMULATOR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE VERILATOR_SIMULATOR_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)

    hal_add_plugin(verilator_simulator
                   SHARED
                   HEADER ${VERILATOR_SIMULATOR_INC}
                   SOURCES ${VERILATOR_SIMULATOR_SRC} ${VERILATOR_SIMULATOR_PYTHON_SRC}
                   PYDOC SPHINX_DOC_INDEX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/documentation/verilator_simulator.rst)


    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(verilog_simulator)
    endif()
    
endif()
