if(BUILD_DOCUMENTATION)
    find_package(Doxygen REQUIRED)

    set(doxyfile "Doxyfile")

    configure_file(${doxyfile} ${doxyfile} @ONLY)

    add_custom_target(doc
                      COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                      COMMENT "Generating API documentation with Doxygen"
                      VERBATIM)

    find_package(Sphinx REQUIRED)
    if(NOT DEFINED SPHINX_THEME)
        set(SPHINX_THEME default)
    endif()

    if(NOT DEFINED SPHINX_THEME_DIR)
        set(SPHINX_THEME_DIR)
    endif()

    # configured documentation tools and intermediate build results
    set(BINARY_BUILD_DIR "${CMAKE_BINARY_DIR}/python-doc/_build")

    # Sphinx cache with pickled ReST documents
    set(SPHINX_CACHE_DIR "${CMAKE_BINARY_DIR}/python-doc/_doctrees")

    # HTML output directory
    set(SPHINX_HTML_DIR "${CMAKE_BINARY_DIR}/python-doc/html")

    configure_file(
            "sphinx_doc/conf.py.in"
            "${BINARY_BUILD_DIR}/conf.py"
            @ONLY)

    file(GLOB_RECURSE SPHINX_DOC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/sphinx_doc/*.rst)
    message(STATUS "SPHINX_DOC_FILES: ${SPHINX_DOC_FILES}")

    add_custom_target(pydoc ALL
                      ${SPHINX_EXECUTABLE}
                      -q -b html
                      -c "${BINARY_BUILD_DIR}"
                      -d "${SPHINX_CACHE_DIR}"
                      "${CMAKE_CURRENT_SOURCE_DIR}/sphinx_doc/"
                      "${SPHINX_HTML_DIR}"
                      COMMENT "Building HTML documentation with Sphinx" VERBATIM
                      DEPENDS hal::py hal::gui graph_algorithm ${SPHINX_DOC_FILES})
endif()