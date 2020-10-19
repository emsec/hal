option(PL_NETLIST_SIMULATOR "PL_NETLIST_SIMULATOR" OFF)

if(PL_NETLIST_SIMULATOR OR BUILD_ALL_PLUGINS)

    file(GLOB_RECURSE NETLIST_SIMULATOR_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE NETLIST_SIMULATOR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE NETLIST_SIMULATOR_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)

    hal_add_plugin(netlist_simulator
                   SHARED PYDOC
                   HEADER ${NETLIST_SIMULATOR_INC}
                   SOURCES ${NETLIST_SIMULATOR_SRC} ${NETLIST_SIMULATOR_PYTHON_SRC}
                   SPHINX_DOC_INDEX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/documentation/netlist_simulator.rst)

    add_subdirectory(test)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(netlist_simulator)
    endif()
    
endif()
