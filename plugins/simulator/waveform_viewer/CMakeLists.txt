option(PL_WAVEFORM_VIEWER "PL_WAVEFORM_VIEWER" ON)
if(PL_WAVEFORM_VIEWER OR BUILD_ALL_PLUGINS)

    find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

    if(Qt5Core_FOUND)
        message(VERBOSE "Qt5Core_INCLUDE_DIRS: ${Qt5Core_INCLUDE_DIRS}")
    elseif(NOT Qt5Core_FOUND)
        message(STATUS "Qt5Core not found for waveform_viewer")
    endif(Qt5Core_FOUND)


    file(GLOB_RECURSE WAVEFORM_VIEWER_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE WAVEFORM_VIEWER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE WAVEFORM_VIEWER_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)
    qt5_wrap_cpp(MOC_HDR ${WAVEFORM_VIEWER_INC})

    hal_add_plugin(waveform_viewer
                   SHARED
                   #PYDOC SPHINX_DOC_INDEX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/documentation/graph_algorithm.rst
                   HEADER ${WAVEFORM_VIEWER_INC}
                   SOURCES ${WAVEFORM_VIEWER_SRC} ${WAVEFORM_VIEWER_PYTHON_SRC}  ${MOC_HDR}
                   LINK_LIBRARIES PUBLIC gui netlist_simulator netlist_simulator_controller Qt5::Core Qt5::Gui Qt5::Widgets)

endif()
