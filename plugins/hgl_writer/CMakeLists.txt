option(PL_HGL_WRITER "PL_HGL_WRITER" ON)
if(PL_HGL_WRITER OR BUILD_ALL_PLUGINS)
    file(GLOB_RECURSE HGL_WRITER_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE HGL_WRITER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

    hal_add_plugin(hgl_writer
                   SHARED
                   HEADER ${HGL_WRITER_INC}
                   SOURCES ${HGL_WRITER_SRC}
                   LINK_LIBRARIES PUBLIC ${IGRAPH_LIBRARIES} gate_library_specific_utils
                   )

    add_subdirectory(test)
endif()
