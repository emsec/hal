option(PL_VERILOG_PARSER "PL_VERILOG_PARSER" ON)
if(PL_VERILOG_PARSER OR BUILD_ALL_PLUGINS)
    file(GLOB_RECURSE VERILOG_PARSER_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE VERILOG_PARSER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE VERILOG_PARSER_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)

    hal_add_plugin(verilog_parser
                   SHARED
                   HEADER ${VERILOG_PARSER_INC}
                   SOURCES ${VERILOG_PARSER_SRC} ${VERILOG_PARSER_PYTHON_SRC}
                   PYDOC SPHINX_DOC_INDEX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/documentation/verilog_parser.rst
                   )
    add_subdirectory(test)
endif()
