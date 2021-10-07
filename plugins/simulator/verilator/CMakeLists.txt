option(PL_VERILATOR "PL_VERILATOR" OFF)

if(PL_VERILATOR OR PL_SIMULATOR OR BUILD_ALL_PLUGINS)

    file(GLOB_RECURSE VERILATOR_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE VERILATOR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE VERILATOR_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)

    hal_add_plugin(verilator
                   SHARED
                   HEADER ${VERILATOR_INC}
                   SOURCES ${VERILATOR_SRC} ${VERILATOR_PYTHON_SRC}
                   PYDOC SPHINX_DOC_INDEX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/documentation/verilator.rst
                   LINK_LIBRARIES PUBLIC netlist_simulator_controller verilog_writer)


    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(verilator)
    endif()
    
endif()
