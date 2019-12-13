option(PL_GRAPH_ALGORITHM "PL_GRAPH_ALGORITHM" ON)
if(PL_GRAPH_ALGORITHM OR BUILD_ALL_PLUGINS)

    find_package(IGraph REQUIRED)
    if(HAVE_IGRAPH)
        message(STATUS "Found following IGRAPH Libraries")
        message(STATUS "    IGRAPH_INCLUDE_DIRS: ${IGRAPH_INCLUDES}")
        message(STATUS "    IGRAPH_LIBRARIES: ${IGRAPH_LIBRARIES}")
    endif(HAVE_IGRAPH)

    file(GLOB_RECURSE GRAPH_ALGORITHM_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE GRAPH_ALGORITHM_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE PYTHON_BINDING_LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/python_definitions.cpp)

    add_library(${GRAPH_ALGORITHM_LIB} SHARED ${GRAPH_ALGORITHM_SRC} ${PYTHON_BINDING_LIB_SRC} ${GRAPH_ALGORITHM_INC})
    target_include_directories(${GRAPH_ALGORITHM_LIB}
                               PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
                               $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
                               $<BUILD_INTERFACE:${IGRAPH_INCLUDES}>
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                               $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                               )
    target_compile_definitions(${GRAPH_ALGORITHM_LIB} PUBLIC
                               -DIGRAPH_VERSION_MAJOR_GUESS=${IGRAPH_VERSION_MAJOR_GUESS}
                               -DIGRAPH_VERSION_MINOR_GUESS=${IGRAPH_VERSION_MINOR_GUESS}
                               -DIGRAPH_VERSION_PATCH_GUESS=${IGRAPH_VERSION_PATCH_GUESS}
                               )
    #Set shared library suffix for MacOS
    if(APPLE AND CMAKE_HOST_APPLE)
        set_target_properties(${GRAPH_ALGORITHM_LIB} PROPERTIES SUFFIX ".so")
        if(APPLE AND CMAKE_HOST_APPLE)
            set_target_properties(${GRAPH_ALGORITHM_LIB} PROPERTIES
                                  INSTALL_NAME_DIR ${PLUGIN_LIBRARY_INSTALL_DIRECTORY}
                                  )
        endif()
    endif(APPLE AND CMAKE_HOST_APPLE)
    target_link_libraries(${GRAPH_ALGORITHM_LIB} hal::core hal::netlist ${PYTHON_LIBRARIES} pybind11::pybind11 ${IGRAPH_LIBRARIES})
    install(TARGETS ${GRAPH_ALGORITHM_LIB} LIBRARY DESTINATION ${PLUGIN_LIBRARY_INSTALL_DIRECTORY} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE INCLUDES DESTINATION ${PLUGIN_INCLUDE_INSTALL_DIRECTORY})
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${PLUGIN_INCLUDE_INSTALL_DIRECTORY}/graph_algorithm/include/)
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(${GRAPH_ALGORITHM_LIB})
    endif()
endif()
