option(PL_CUSTOM_SIM "PL_CUSTOM_SIM" OFF)

if(PL_CUSTOM_SIM OR PL_SIMULATOR OR BUILD_ALL_PLUGINS)

    file(GLOB_RECURSE CUSTOM_SIM_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE CUSTOM_SIM_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE CUSTOM_SIM_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)

    hal_add_plugin(custom_sim
                   SHARED
                   HEADER ${CUSTOM_SIM_INC}
                   SOURCES ${CUSTOM_SIM_SRC} ${CUSTOM_SIM_PYTHON_SRC}
                   PYDOC SPHINX_DOC_INDEX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/documentation/custom_sim.rst
                   LINK_LIBRARIES PUBLIC netlist_simulator_controller)

    #add_subdirectory(test)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(custom_sim)
    endif()
    
endif()
