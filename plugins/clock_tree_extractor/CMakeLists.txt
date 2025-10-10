option(PL_CLOCK_TREE_EXTRACTOR "PL_CLOCK_TREE_EXTRACTOR" ON)

if(PL_CLOCK_TREE_EXTRACTOR OR BUILD_ALL_PLUGINS)

    if(IWYU)
        set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "include-what-you-use")
        message(STATUS "include-what-you-use turned ON")
    else()
        message(STATUS "include-what-you-use turned OFF")
    endif()

    file(GLOB_RECURSE CLOCK_TREE_EXTRACTOR_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE CLOCK_TREE_EXTRACTOR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)
    file(GLOB_RECURSE CLOCK_TREE_EXTRACTOR_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cc)

    hal_add_plugin(clock_tree_extractor
                    SHARED
                    HEADER ${CLOCK_TREE_EXTRACTOR_INC}
                    SOURCES ${CLOCK_TREE_EXTRACTOR_SRC} ${CLOCK_TREE_EXTRACTOR_PYTHON_SRC}
                    LINK_LIBRARIES graph_algorithm
                    COMPILE_OPTIONS "-march=native"
                )

endif()
