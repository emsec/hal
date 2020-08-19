option(PL_LIBERTY_PARSER "PL_LIBERTY_PARSER" ON)
if(PL_LIBERTY_PARSER OR BUILD_ALL_PLUGINS)
    file(GLOB_RECURSE LIBERTY_PARSER_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE LIBERTY_PARSER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

    hal_add_plugin(liberty_parser
                   SHARED
                   HEADER ${LIBERTY_PARSER_INC}
                   SOURCES ${LIBERTY_PARSER_SRC}
                   )
endif()
