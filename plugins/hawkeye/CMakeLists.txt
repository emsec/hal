option(PL_HAWKEYE "PL_HAWKEYE" ON)

if(PL_HAWKEYE OR BUILD_ALL_PLUGINS)

    file(GLOB_RECURSE HAWKEYE_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE HAWKEYE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE HAWKEYE_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)

    hal_add_plugin(hawkeye
                    SHARED
                    HEADER ${HAWKEYE_INC}
                    SOURCES ${HAWKEYE_SRC} ${HAWKEYE_PYTHON_SRC}
                    PYDOC SPHINX_DOC_INDEX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/documentation/hawkeye.rst
                    LINK_LIBRARIES graph_algorithm
                    COMPILE_OPTIONS "-march=native"
                )
		 
endif()
