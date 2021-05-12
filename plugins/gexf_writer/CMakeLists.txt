option(PL_GEXF_WRITER "PL_GEXF_WRITER" ON)
if(PL_GEXF_WRITER OR BUILD_ALL_PLUGINS)

    find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

    if(Qt5Core_FOUND)
        message(VERBOSE "Qt5Core_INCLUDE_DIRS: ${Qt5Core_INCLUDE_DIRS}")
    elseif(NOT Qt5Core_FOUND)
        message(STATUS "Qt5Core not found for gexf")
    endif(Qt5Core_FOUND)

    file(GLOB_RECURSE GEXF_WRITER_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE GEXF_WRITER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

    # Create the '__init__.py' file in the hal_plugins directory as there might not be a any plugins available yet.
    file(WRITE ${CMAKE_BINARY_DIR}/lib/hal_plugins/__init__.py "")

    add_library(gexf_writer SHARED ${GEXF_WRITER_INC} ${GEXF_WRITER_SRC})

    set_target_properties(gexf_writer PROPERTIES PREFIX "")

    target_include_directories(gexf_writer
                               PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
                               $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
                               $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                               ${ARG_INCLUDES}
                               ${CMAKE_CURRENT_SOURCE_DIR}/../gui/include
                               )

    target_compile_options(gexf_writer
                           PUBLIC ${COMPILE_OPTIONS_PUBLIC}
                           PRIVATE ${COMPILE_OPTIONS_PRIVATE}
                           INTERFACE ${COMPILE_OPTIONS_INTERFACE}
                           ${ARG_COMPILE_OPTIONS})
    if (ARG_LINK_OPTIONS)
        target_link_options(gexf_writer ${ARG_LINK_OPTIONS})
    endif()
    #Set shared library suffix for MacOS
    if(APPLE AND CMAKE_HOST_APPLE)
        set_target_properties(gexf_writer PROPERTIES SUFFIX ".so")
        set_target_properties(gexf_writer PROPERTIES INSTALL_NAME_DIR ${PLUGIN_LIBRARY_INSTALL_DIRECTORY})
    endif(APPLE AND CMAKE_HOST_APPLE)

    target_link_libraries(gexf_writer
                          PUBLIC gui
                          Qt5::Core
                          Qt5::Gui
                          Qt5::Widgets
                          ${ARG_LINK_LIBRARIES})

    install(TARGETS gexf_writer LIBRARY DESTINATION ${PLUGIN_LIBRARY_INSTALL_DIRECTORY} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE INCLUDES DESTINATION ${PLUGIN_INCLUDE_INSTALL_DIRECTORY})

    if (ARG_INSTALL_INCLUDE_DIR)
        set(INSTALL_INCLUDE_DIR ${ARG_INSTALL_INCLUDE_DIR})
    else()
        set(INSTALL_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
    endif()
    if (NOT ARG_NO_INSTALL_INCLUDEDIR)
        install(DIRECTORY ${INSTALL_INCLUDE_DIR} DESTINATION ${PLUGIN_INCLUDE_INSTALL_DIRECTORY}/gexf_writer/include/)
    endif()

    if((${CMAKE_BUILD_TYPE} STREQUAL "Debug") AND (COMMAND add_sanitizers))
        add_sanitizers(gexf_writer)
    endif()

    if (ARG_PYDOC)
        hal_plugin_documentation(gexf_writer PYDOC
                                 SPHINX_DOC_INDEX_FILE ${ARG_SPHINX_DOC_INDEX_FILE}
                                 SPHINX_DOC_FILES ${ARG_SPHINX_DOC_FILES})
    endif()

endif()
