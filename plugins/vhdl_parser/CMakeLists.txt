option(PL_VHDL_PARSER "PL_VHDL_PARSER" ON)
if(PL_VHDL_PARSER OR BUILD_ALL_PLUGINS)
    file(GLOB_RECURSE VHDL_PARSER_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE VHDL_PARSER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

    hal_add_plugin(vhdl_parser
                   SHARED
                   HEADER ${VHDL_PARSER_INC}
                   SOURCES ${VHDL_PARSER_SRC}
                   )
    add_subdirectory(test)
endif()
