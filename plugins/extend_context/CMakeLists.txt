option(PL_EXTEND_CONTEXT "PL_EXTEND_CONTEXT" OFF)
if(PL_EXTEND_CONTEXT OR BUILD_ALL_PLUGINS)
    file(GLOB_RECURSE EXTEND_CONTEXT_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE EXTEND_CONTEXT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE EXTEND_CONTEXT_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)

    hal_add_plugin(extend_context
                   SHARED
                   HEADER ${EXTEND_CONTEXT_INC}
                   SOURCES ${EXTEND_CONTEXT_SRC} ${EXTEND_CONTEXT_PYTHON_SRC}
                   LINK_LIBRARIES PUBLIC netlist_simulator_controller
                   )
endif()
