option(PL_VCD_VIEWER "PL_VCD_VIEWER" ON)
if(PL_VCD_VIEWER OR PL_SIMULATOR OR BUILD_ALL_PLUGINS)

    find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

    if(Qt5Core_FOUND)
        message(VERBOSE "Qt5Core_INCLUDE_DIRS: ${Qt5Core_INCLUDE_DIRS}")
    elseif(NOT Qt5Core_FOUND)
        message(STATUS "Qt5Core not found for vcd_viewer")
    endif(Qt5Core_FOUND)

    file(GLOB_RECURSE VCD_VIEWER_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE VCD_VIEWER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE VCD_VIEWER_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)
    qt5_wrap_cpp(MOC_HDR ${VCD_VIEWER_INC})

    # Create the '__init__.py' file in the hal_plugins directory as there might not be a any plugins available yet.
    file(WRITE ${CMAKE_BINARY_DIR}/lib/hal_plugins/__init__.py "")

    add_library(vcd_viewer SHARED ${VCD_VIEWER_INC} ${VCD_VIEWER_SRC} ${VCD_VIEWER_PYTHON_SRC} ${MOC_HDR})

    set_target_properties(vcd_viewer PROPERTIES PREFIX "")

    target_include_directories(vcd_viewer
                               PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
                               $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
                               $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                               ${ARG_INCLUDES}
                               ${CMAKE_CURRENT_SOURCE_DIR}/../gui/include
                               ${CMAKE_CURRENT_SOURCE_DIR}/../netlist_simulator/include
                               )

    target_compile_options(vcd_viewer
                           PUBLIC ${COMPILE_OPTIONS_PUBLIC}
                           PRIVATE ${COMPILE_OPTIONS_PRIVATE}
                           INTERFACE ${COMPILE_OPTIONS_INTERFACE}
                           ${ARG_COMPILE_OPTIONS})
    if (ARG_LINK_OPTIONS)
        target_link_options(vcd_viewer ${ARG_LINK_OPTIONS})
    endif()
    #Set shared library suffix for MacOS
    if(APPLE AND CMAKE_HOST_APPLE)
        set_target_properties(vcd_viewer PROPERTIES SUFFIX ".so")
        set_target_properties(vcd_viewer PROPERTIES INSTALL_NAME_DIR ${PLUGIN_LIBRARY_INSTALL_DIRECTORY})
    endif(APPLE AND CMAKE_HOST_APPLE)

    target_link_libraries(vcd_viewer
                          PUBLIC gui
                          Qt5::Core
                          Qt5::Gui
                          Qt5::Widgets
                          ${ARG_LINK_LIBRARIES})

    install(TARGETS vcd_viewer LIBRARY DESTINATION ${PLUGIN_LIBRARY_INSTALL_DIRECTORY} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE INCLUDES DESTINATION ${PLUGIN_INCLUDE_INSTALL_DIRECTORY})

    if (ARG_INSTALL_INCLUDE_DIR)
        set(INSTALL_INCLUDE_DIR ${ARG_INSTALL_INCLUDE_DIR})
    else()
        set(INSTALL_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
    endif()
    if (NOT ARG_NO_INSTALL_INCLUDEDIR)
        install(DIRECTORY ${INSTALL_INCLUDE_DIR} DESTINATION ${PLUGIN_INCLUDE_INSTALL_DIRECTORY}/vcd_viewer/include/)
    endif()

    if((${CMAKE_BUILD_TYPE} STREQUAL "Debug") AND (COMMAND add_sanitizers))
        add_sanitizers(vcd_viewer)
    endif()

    if (ARG_PYDOC)
        hal_plugin_documentation(vcd_viewer PYDOC
                                 SPHINX_DOC_INDEX_FILE ${ARG_SPHINX_DOC_INDEX_FILE}
                                 SPHINX_DOC_FILES ${ARG_SPHINX_DOC_FILES})
    endif()

endif()
