option(PL_XILINX_TOOLBOX "PL_XILINX_TOOLBOX" OFF)

if(PL_XILINX_TOOLBOX OR BUILD_ALL_PLUGINS)

    file(GLOB_RECURSE XILINX_TOOLBOX_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE XILINX_TOOLBOX_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE XILINX_TOOLBOX_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)

    hal_add_plugin(xilinx_toolbox
                   SHARED
                   HEADER ${XILINX_TOOLBOX_INC}
                   SOURCES ${XILINX_TOOLBOX_SRC} ${XILINX_TOOLBOX_PYTHON_SRC}
                   PYDOC SPHINX_DOC_INDEX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/documentation/xilinx_toolbox.rst
                )
endif()
