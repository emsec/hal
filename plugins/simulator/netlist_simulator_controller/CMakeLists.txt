option(PL_NETLIST_SIMULATOR_CONTROLLER "PL_NETLIST_SIMULATOR_CONTROLLER" OFF)
if(PL_NETLIST_SIMULATOR_CONTROLLER OR PL_SIMULATOR OR BUILD_ALL_PLUGINS)

    find_package(Qt5 COMPONENTS Core REQUIRED)

    if(Qt5Core_FOUND)
        message(VERBOSE "Qt5Core_INCLUDE_DIRS: ${Qt5Core_INCLUDE_DIRS}")
    elseif(NOT Qt5Core_FOUND)
        message(STATUS "Qt5Core not found for netlist_simulator_controller")
    endif(Qt5Core_FOUND)

    enable_cxx_compile_option_if_supported("-O1" "Perf" "PUBLIC")
    enable_cxx_compile_option_if_supported("-g" "Perf" "PUBLIC")
    enable_cxx_compile_option_if_supported("-fno-inline-functions" "Perf" "PUBLIC")
    enable_cxx_compile_option_if_supported("-fno-inline-functions-called-once" "Perf" "PUBLIC")
    enable_cxx_compile_option_if_supported("-fno-optimize-sibling-calls" "Perf" "PUBLIC")


    file(GLOB_RECURSE NETLIST_SIMULATOR_CONTROLLER_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE NETLIST_SIMULATOR_CONTROLLER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE NETLIST_SIMULATOR_CONTROLLER_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)
    file(GLOB_RECURSE NETLIST_SIMULATOR_CONTROLLER_QRC ${CMAKE_CURRENT_SOURCE_DIR}/resources/*.qrc)

    qt5_wrap_cpp(MOC_HDR ${NETLIST_SIMULATOR_CONTROLLER_INC})
    qt5_add_resources(MOC_QRC ${NETLIST_SIMULATOR_CONTROLLER_QRC})

    hal_add_plugin(netlist_simulator_controller
                   SHARED
                   #PYDOC SPHINX_DOC_INDEX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/documentation/graph_algorithm.rst
                   HEADER ${NETLIST_SIMULATOR_CONTROLLER_INC}
                   SOURCES ${NETLIST_SIMULATOR_CONTROLLER_SRC} ${NETLIST_SIMULATOR_CONTROLLER_PYTHON_SRC} ${MOC_HDR} ${MOC_QRC}
                   PYDOC SPHINX_DOC_INDEX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/documentation/netlist_simulator_controller.rst
                   LINK_LIBRARIES PUBLIC Qt5::Core)

    add_subdirectory(test)
    add_subdirectory(saleae_cli)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(netlist_simulator)
    endif()
                   

endif()
