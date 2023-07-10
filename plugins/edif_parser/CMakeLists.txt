option(PL_EDIF_PARSER "PL_EDIF_PARSER" ON)
if(PL_EDIF_PARSER OR BUILD_ALL_PLUGINS)
    file(GLOB_RECURSE EDIF_PARSER_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE EDIF_PARSER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

    hal_add_plugin(edif_parser
                   SHARED
                   HEADER ${EDIF_PARSER_INC}
                   SOURCES ${EDIF_PARSER_SRC}
                   )
endif()
