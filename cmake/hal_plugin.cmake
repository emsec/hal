# Build a hal plugin:
# hal_add_plugin(<name> [MODULE | SHARED] [EXCLUDE_FROM_ALL]
#                     [NO_EXTRAS] [SYSTEM] [THIN_LTO] source1 [source2 ...])
#
function(hal_add_plugin target_name)
    set(options  MODULE SHARED EXCLUDE_FROM_ALL INSTALL NO_INSTALL_INCLUDEDIR PYDOC)
    set(oneValueArgs RENAME INSTALL_INCLUDE_DIR SPHINX_DOC_INDEX_FILE)
    set(multiValueArgs HEADER SOURCES LINK_LIBRARIES INCLUDES DEFINITIONS COMPILE_OPTIONS LINK_OPTIONS SPHINX_DOC_FILES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN})


    if(ARG_MODULE AND ARG_SHARED)
        message(FATAL_ERROR "Can't be both MODULE and SHARED")
    elseif(ARG_SHARED)
        set(lib_type SHARED)
    else()
        set(lib_type MODULE)
    endif()

    if(ARG_EXCLUDE_FROM_ALL)
        set(exclude_from_all EXCLUDE_FROM_ALL)
    endif()

    # Create the '__init__.py' file in the hal_plugins directory as there might not be a any plugins available yet.
    file(WRITE ${CMAKE_BINARY_DIR}/lib/hal_plugins/__init__.py "")

    add_library(${target_name} ${lib_type} ${exclude_from_all} ${ARG_HEADER} ${ARG_SOURCES} ${ARG_PY_SOURCES})

    set_target_properties(${target_name} PROPERTIES PREFIX "")

    target_include_directories(${target_name}
                               PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
                               $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
                               $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                               ${ARG_INCLUDES}
                               )
    if (ARG_DEFINITIONS)
        target_compile_definitions(${target_name}
                                   ${ARG_DEFINITIONS}
                                   )
    endif()

    target_compile_options(${target_name}
                           PUBLIC ${COMPILE_OPTIONS_PUBLIC}
                           PRIVATE ${COMPILE_OPTIONS_PRIVATE}
                           INTERFACE ${COMPILE_OPTIONS_INTERFACE}
                           ${ARG_COMPILE_OPTIONS})
    if (ARG_LINK_OPTIONS)
        target_link_options(${target_name} ${ARG_LINK_OPTIONS})
    endif()
    #Set shared library suffix for MacOS
    if(APPLE AND CMAKE_HOST_APPLE)
        set_target_properties(${target_name} PROPERTIES SUFFIX ".so")
        set_target_properties(${target_name} PROPERTIES INSTALL_NAME_DIR ${PLUGIN_LIBRARY_INSTALL_DIRECTORY})
    endif(APPLE AND CMAKE_HOST_APPLE)

    target_link_libraries(${target_name}
                          PUBLIC hal::core hal::netlist ${PYTHON_LIBRARIES} pybind11::pybind11
                          ${ARG_LINK_LIBRARIES})

    install(TARGETS ${target_name} LIBRARY DESTINATION ${PLUGIN_LIBRARY_INSTALL_DIRECTORY} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE INCLUDES DESTINATION ${PLUGIN_INCLUDE_INSTALL_DIRECTORY})

    if (ARG_INSTALL_INCLUDE_DIR)
        set(INSTALL_INCLUDE_DIR ${ARG_INSTALL_INCLUDE_DIR})
    else()
        set(INSTALL_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
    endif()
    if (NOT ARG_NO_INSTALL_INCLUDEDIR)
        install(DIRECTORY ${INSTALL_INCLUDE_DIR} DESTINATION ${PLUGIN_INCLUDE_INSTALL_DIRECTORY}/${target_name}/include/)
    endif()
    
    if((${CMAKE_BUILD_TYPE} STREQUAL "Debug") AND (COMMAND add_sanitizers))
        add_sanitizers(${target_name})
    endif()

    if (ARG_PYDOC)
        hal_plugin_documentation(${target_name} PYDOC
                                 SPHINX_DOC_INDEX_FILE ${ARG_SPHINX_DOC_INDEX_FILE}
                                 SPHINX_DOC_FILES ${ARG_SPHINX_DOC_FILES})
    endif()
endfunction()

function(hal_plugin_documentation target_name)
    set(oneValueArgs SPHINX_DOC_INDEX_FILE)
    set(multiValueArgs SPHINX_DOC_FILES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN})

    get_filename_component(idxf ${ARG_SPHINX_DOC_INDEX_FILE} NAME)
    configure_file(${ARG_SPHINX_DOC_INDEX_FILE} ${SPHINX_BUILD_DIR}/${idxf} COPYONLY)

    get_property(SPHINX_PLUGIN_INDEX_FILES_TMP GLOBAL PROPERTY SPHINX_PLUGIN_INDEX_FILES)
    list(APPEND SPHINX_PLUGIN_INDEX_FILES_TMP ${ARG_SPHINX_DOC_INDEX_FILE})
    set_property(GLOBAL PROPERTY SPHINX_PLUGIN_INDEX_FILES "${SPHINX_PLUGIN_INDEX_FILES_TMP}")

    foreach(file ${ARG_SPHINX_DOC_FILES})
        get_filename_component(f ${file} NAME)
        configure_file(${file} ${SPHINX_BUILD_DIR}/${f} COPYONLY)
    endforeach()

endfunction()