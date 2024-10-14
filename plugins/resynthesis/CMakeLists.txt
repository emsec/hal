option(PL_RESYNTHESIS "PL_RESYNTHESIS" ON)

if(PL_RESYNTHESIS OR BUILD_ALL_PLUGINS)

    file(GLOB_RECURSE RESYNTHESIS_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE RESYNTHESIS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE RESYNTHESIS_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)

    hal_add_plugin(resynthesis
                   SHARED
                   HEADER ${RESYNTHESIS_INC}
                   SOURCES ${RESYNTHESIS_SRC} ${RESYNTHESIS_PYTHON_SRC}
                   PYDOC SPHINX_DOC_INDEX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/documentation/resynthesis.rst
                   LINK_LIBRARIES genlib_writer
                )
endif()
