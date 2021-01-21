option(PL_HGL_PARSER "PL_HGL_PARSER" ON)
if(PL_HGL_PARSER OR BUILD_ALL_PLUGINS)
    file(GLOB_RECURSE HGL_PARSER_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE HGL_PARSER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

    hal_add_plugin(hgl_parser
                   SHARED
                   HEADER ${HGL_PARSER_INC}
                   SOURCES ${HGL_PARSER_SRC}
                   )

    add_subdirectory(test)
endif()
