if(BUILD_DOCUMENTATION)
    find_package(Doxygen REQUIRED)

    set(DOXYGEN_IN ${CMAKE_SOURCE_DIR}/documentation/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_BINARY_DIR}/documentation/Doxyfile)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Building Doxygen documentation...")

    add_custom_target(doc ALL
                      COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                      COMMENT "Generating API documentation with Doxygen"
                      VERBATIM )


    find_package(Sphinx REQUIRED)
    if(NOT DEFINED SPHINX_THEME)
        set(SPHINX_THEME default)
    endif()

    if(NOT DEFINED SPHINX_THEME_DIR)
        set(SPHINX_THEME_DIR)
    endif()

    # configured documentation tools and intermediate build results
    set(BINARY_BUILD_DIR "${CMAKE_BINARY_DIR}/documentation/python-doc/_build")

    # Sphinx cache with pickled ReST documents
    set(SPHINX_CACHE_DIR "${CMAKE_BINARY_DIR}/documentation/python-doc/_doctrees")

    # HTML output directory
    set(SPHINX_HTML_DIR "${CMAKE_BINARY_DIR}/documentation/python-doc/html")

    configure_file(
            "sphinx_doc/conf.py.in"
            "${BINARY_BUILD_DIR}/conf.py"
            @ONLY)

    file(GLOB_RECURSE SPHINX_DOC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/sphinx_doc/*.rst)

    file(MAKE_DIRECTORY ${SPHINX_BUILD_DIR})
    message(STATUS "SPHINX_DOC_FILES: ${SPHINX_DOC_FILES}")

    foreach(file ${SPHINX_DOC_FILES})
        get_filename_component(f ${file} NAME)
        list(APPEND DESTINATION_DOC_FILES ${SPHINX_BUILD_DIR}/${f})
    endforeach()

    get_property(SPHINX_PLUGIN_INDEX_FILES_LOCAL GLOBAL PROPERTY SPHINX_PLUGIN_INDEX_FILES)
    get_property(SPHINX_PLUGIN_TARGETS_LOCAL GLOBAL PROPERTY SPHINX_PLUGIN_TARGETS)

    foreach(idx ${SPHINX_PLUGIN_TARGETS_LOCAL})
        set(PYDOC_PLUGINS "${PYDOC_PLUGINS}\n   ${idx}")
        set(PYDOC_PLUGINS_TARGETS "${PYDOC_PLUGINS_TARGETS} ${idx}")
    endforeach()

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sphinx_doc/plugins.rst.in ${SPHINX_BUILD_DIR}/plugins.rst)

    add_custom_target(copy_sphinx_doc_files ALL
                      BYPRODUCTS ${DESTINATION_DOC_FILES}
                      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/sphinx_doc/*.rst ${SPHINX_BUILD_DIR}/
                      DEPENDS ${SPHINX_DOC_FILES}
                      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                      COMMENT "Update sphinx doc files")

    add_custom_target(pydoc ALL
                      ${SPHINX_EXECUTABLE}
                      -q -b html
                      -c "${BINARY_BUILD_DIR}"
                      -d "${SPHINX_CACHE_DIR}"
                      "${SPHINX_BUILD_DIR}"
                      "${SPHINX_HTML_DIR}"
                      COMMENT "Building HTML documentation with Sphinx" VERBATIM
                      DEPENDS hal::py copy_sphinx_doc_files)

    add_dependencies(pydoc ${SPHINX_PLUGIN_TARGETS_LOCAL})

endif()