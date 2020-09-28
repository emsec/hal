option(PL_PYTHON_SHELL "PL_PYTHON_SHELL" ON)
if(PL_PYTHON_SHELL OR BUILD_ALL_PLUGINS)
    file(GLOB_RECURSE PYTHON_SHELL_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE PYTHON_SHELL_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

    hal_add_plugin(python_shell
                   SHARED
                   HEADER ${PYTHON_SHELL_INC}
                   SOURCES ${PYTHON_SHELL_SRC}
                   )
endif()
