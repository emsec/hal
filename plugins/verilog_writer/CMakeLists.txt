option(PL_VERILOG_WRITER "PL_VERILOG_WRITER" ON)
if(PL_VERILOG_WRITER OR BUILD_ALL_PLUGINS)
    file(GLOB_RECURSE VERILOG_WRITER_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE VERILOG_WRITER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

    hal_add_plugin(verilog_writer
                   SHARED
                   HEADER ${VERILOG_WRITER_INC}
                   SOURCES ${VERILOG_WRITER_SRC}
                   )

    add_subdirectory(test)
endif()
