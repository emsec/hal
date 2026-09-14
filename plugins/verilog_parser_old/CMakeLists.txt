option(PL_VERILOG_PARSER_OLD "PL_VERILOG_PARSER_OLD" ON)
if(PL_VERILOG_PARSER_OLD OR PL_RESYNTHESIS OR BUILD_ALL_PLUGINS)
    file(GLOB_RECURSE VERILOG_PARSER_OLD_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE VERILOG_PARSER_OLD_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

    hal_add_plugin(verilog_parser_old
                   SHARED
                   HEADER ${VERILOG_PARSER_OLD_INC}
                   SOURCES ${VERILOG_PARSER_OLD_SRC}
                   )
    add_subdirectory(test)
endif()
