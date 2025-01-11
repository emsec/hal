option(PL_DATAFLOW "PL_DATAFLOW" OFF)

if(PL_DATAFLOW OR BUILD_ALL_PLUGINS)

    file(GLOB_RECURSE DATAFLOW_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE DATAFLOW_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE DATAFLOW_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)

    hal_add_plugin(dataflow
                   SHARED
                   HEADER ${DATAFLOW_INC}
                   SOURCES ${DATAFLOW_SRC} ${DATAFLOW_PYTHON_SRC}
                   PYDOC SPHINX_DOC_INDEX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/documentation/dataflow.rst
                   )
		 
endif()
