option(PL_HAWKEYE "PL_HAWKEYE" OFF)

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

    # Copy the S-box database into the build directory so that it can be found
    # at runtime, analogous to how the gate library definitions are handled.
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/share/hal/)
    add_custom_target(update_sbox_database ALL
                      BYPRODUCTS ${CMAKE_BINARY_DIR}/share/hal/sbox-database.json
                      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/sbox-database.json ${CMAKE_BINARY_DIR}/share/hal/
                      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/sbox-database.json
                      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                      COMMENT "Update S-box database")

endif()
