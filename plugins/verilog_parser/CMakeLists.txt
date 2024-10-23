option(PL_VERILOG_PARSER "PL_VERILOG_PARSER" ON)
if(PL_VERILOG_PARSER OR PL_RESYNTHESIS OR BUILD_ALL_PLUGINS)
    file(GLOB_RECURSE VERILOG_PARSER_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE VERILOG_PARSER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

    hal_add_plugin(verilog_parser
                   SHARED
                   HEADER ${VERILOG_PARSER_INC}
                   SOURCES ${VERILOG_PARSER_SRC}
                   )
    add_subdirectory(test)
endif()
