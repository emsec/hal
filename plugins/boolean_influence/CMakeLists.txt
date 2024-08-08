option(PL_BOOLEAN_INFLUENCE "PL_BOOLEAN_INFLUENCE" OFF)
if(PL_BOOLEAN_INFLUENCE OR BUILD_ALL_PLUGINS)
    file(GLOB_RECURSE BOOLEAN_INFLUENCE_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE BOOLEAN_INFLUENCE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE BOOLEAN_INFLUENCE_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)

    hal_add_plugin(boolean_influence
                   SHARED
                   HEADER ${BOOLEAN_INFLUENCE_INC}
                   SOURCES ${BOOLEAN_INFLUENCE_SRC} ${BOOLEAN_INFLUENCE_PYTHON_SRC}
                   LINK_LIBRARIES PUBLIC ${Z3_LIBRARIES} z3_utils subprocess::subprocess
                   PYDOC SPHINX_DOC_INDEX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/documentation/boolean_influence.rst
                   )
endif()
