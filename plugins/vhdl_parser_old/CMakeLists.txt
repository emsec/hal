option(PL_VHDL_PARSER_OLD "PL_VHDL_PARSER_OLD" ON)
if(PL_VHDL_PARSER_OLD OR BUILD_ALL_PLUGINS)
    file(GLOB_RECURSE VHDL_PARSER_OLD_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE VHDL_PARSER_OLD_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

    hal_add_plugin(vhdl_parser_old
                   SHARED
                   HEADER ${VHDL_PARSER_OLD_INC}
                   SOURCES ${VHDL_PARSER_OLD_SRC}
                   )
    add_subdirectory(test)
endif()
