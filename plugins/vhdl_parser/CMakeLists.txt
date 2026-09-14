option(PL_VHDL_PARSER "PL_VHDL_PARSER" ON)
if(PL_VHDL_PARSER OR BUILD_ALL_PLUGINS)
    file(GLOB_RECURSE VHDL_PARSER_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE VHDL_PARSER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE VHDL_PARSER_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)

    hal_add_plugin(vhdl_parser
                   SHARED
                   HEADER ${VHDL_PARSER_INC}
                   SOURCES ${VHDL_PARSER_SRC} ${VHDL_PARSER_PYTHON_SRC}
                   PYDOC SPHINX_DOC_INDEX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/documentation/vhdl_parser.rst
                   )
    add_subdirectory(test)
endif()
