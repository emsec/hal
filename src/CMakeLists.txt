include_directories(${include_directories}
                    ${CMAKE_SOURCE_DIR}/include
                    ${BUDDY_INCLUDE_DIR}
                    SYSTEM ${Boost_INCLUDE_DIR})

#
# core library
#
configure_file(${CMAKE_SOURCE_DIR}/include/hal_version.h.in ${CMAKE_BINARY_DIR}/hal_version.h @ONLY)
include_directories(${CMAKE_BINARY_DIR})
file(GLOB CORE_LIB_HDR ${CMAKE_SOURCE_DIR}/include/*.h)
file(GLOB_RECURSE CORE_LIB_HDR ${CMAKE_SOURCE_DIR}/include/core/*.h)
file(GLOB_RECURSE CORE_LIB_SRC ${CMAKE_SOURCE_DIR}/src/core/*.cpp)

add_library(${CORE_LIB} SHARED ${CORE_LIB_HDR} ${CORE_LIB_HDR} ${CORE_LIB_SRC} ${CMAKE_BINARY_DIR}/hal_version.h)
set_target_properties( PROPERTIES DEFINE_SYMBOL BUILDING_CORE)
target_link_libraries(${CORE_LIB} ${CMAKE_DL_LIBS} -lstdc++fs)
install(TARGETS ${CORE_LIB} LIBRARY DESTINATION ${LIBRARY_INSTALL_DIRECTORY} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_sanitizers(${CORE_LIB})
endif()

#
# netlist + hdl parser + hdl file writer library
#
file(GLOB_RECURSE NETLIST_LIB_HDR ${CMAKE_SOURCE_DIR}/include/netlist/*.h)
file(GLOB_RECURSE NETLIST_LIB_SRC ${CMAKE_SOURCE_DIR}/src/netlist/*.cpp)
file(GLOB_RECURSE HDL_PARSER_LIB_HDR ${CMAKE_SOURCE_DIR}/include/hdl_parser/*.h)
file(GLOB_RECURSE HDL_PARSER_LIB_SRC ${CMAKE_SOURCE_DIR}/src/hdl_parser/*.cpp)
file(GLOB_RECURSE HDL_WRITER_LIB_HDR ${CMAKE_SOURCE_DIR}/include/hdl_riter/*.h)
file(GLOB_RECURSE HDL_WRITER_LIB_SRC ${CMAKE_SOURCE_DIR}/src/hdl_writer/*.cpp)
file(GLOB_RECURSE GATE_TYPE_SYSTEM_INC ${CMAKE_SOURCE_DIR}/include/gate_decorator_system/*.h)
file(GLOB_RECURSE GATE_TYPE_SYSTEM_SRC ${CMAKE_SOURCE_DIR}/src/gate_decorator_system/*.cpp)

add_library(${NETLIST_LIB} SHARED ${NETLIST_LIB_HDR} ${HDL_PARSER_LIB_HDR} ${HDL_WRITER_LIB_HDR} ${NETLIST_LIB_SRC} ${HDL_PARSER_LIB_SRC} ${HDL_WRITER_LIB_SRC} ${GATE_TYPE_SYSTEM_SRC} ${GATE_TYPE_SYSTEM_INC})
set_target_properties(${NETLIST_LIB} PROPERTIES DEFINE_SYMBOL API_NETLIST)
target_link_libraries(${NETLIST_LIB} ${CORE_LIB})
install(TARGETS ${NETLIST_LIB} LIBRARY DESTINATION ${LIBRARY_INSTALL_DIRECTORY} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
        GROUP_EXECUTE WORLD_READ WORLD_EXECUTE INCLUDES
        DESTINATION ${INCLUDE_INSTALL_DIRECTORY})
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_sanitizers(${NETLIST_LIB})
endif()

    set(PYTHON_BINDING_LIB hal_py)
    set(PYTHON_PLUGIN_LIB hal_python)
    #Python support
    set(Python_ADDITIONAL_VERSIONS 3.5 3.6)
    find_package(PythonInterp 3.5 REQUIRED)
    if(PythonInterp_FOUND)
        message(STATUS "PYTHON_INCLUDE_DIRS: ${PYTHON_INCLUDE_DIRS}")
        message(STATUS "PYTHON_LIBRARIES: ${PYTHON_LIBRARIES}")
        message(STATUS "PYTHON_MODULE_PREFIX: ${PYTHON_MODULE_PREFIX}")
        message(STATUS "PYTHON_MODULE_EXTENSION: ${PYTHON_MODULE_EXTENSION}")
    elseif(NOT PythonInterp_FOUND)
        set(Missing_package "TRUE")
        if(APPLE AND CMAKE_HOST_APPLE)
            message(STATUS "To install python3 on MacOS using homebrew run following command:")
            message(STATUS "    brew install python3")
        endif(APPLE AND CMAKE_HOST_APPLE)
    endif(PythonInterp_FOUND)

    include_directories(${include_directories}
                        ${BUDDY_INCLUDE_DIR}
                        ${CMAKE_SOURCE_DIR}/include
                        SYSTEM ${Boost_INCLUDE_DIR}
                        SYSTEM ${PYBIND11_INCLUDE_DIR}
                        SYSTEM ${PYTHON_INCLUDE_DIRS})

    file(GLOB_RECURSE PYTHON_BINDING_LIB_SRC ${CMAKE_SOURCE_DIR}/src/python-binding/python_definitions.cpp)

    add_library(${PYTHON_BINDING_LIB} SHARED ${PYTHON_BINDING_LIB_SRC})
    #Set shared library suffix for MacOS; Used for boost_python support
    set_target_properties(${PYTHON_BINDING_LIB} PROPERTIES PREFIX "")
    if(APPLE AND CMAKE_HOST_APPLE)
        set_target_properties(${PYTHON_BINDING_LIB} PROPERTIES SUFFIX ".so")
    endif(APPLE AND CMAKE_HOST_APPLE)
    target_link_libraries(${PYTHON_BINDING_LIB} ${LINK_LIBS} ${PYTHON_LIBRARIES} pybind11::module)
    install(TARGETS ${PYTHON_BINDING_LIB} LIBRARY DESTINATION ${LIBRARY_INSTALL_DIRECTORY} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE INCLUDES DESTINATION ${INCLUDE_INSTALL_DIRECTORY})
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(${PYTHON_BINDING_LIB})
    endif()

    file(GLOB_RECURSE PYTHON_PLUGIN_LIB_HDR ${CMAKE_SOURCE_DIR}/include/python-binding/*.h)
    file(GLOB_RECURSE PYTHON_PLUGIN_LIB_SRC ${CMAKE_SOURCE_DIR}/src/python-binding/factory_python_shell.cpp ${CMAKE_SOURCE_DIR}/src/python-binding/plugin_python_shell.cpp)

    add_library(${PYTHON_PLUGIN_LIB} SHARED ${PYTHON_PLUGIN_LIB_HDR} ${PYTHON_PLUGIN_LIB_SRC})
    set_target_properties(${PYTHON_PLUGIN_LIB} PROPERTIES DEFINE_SYMBOL API_PYTHON_PLUGIN)
    target_link_libraries(${PYTHON_PLUGIN_LIB} ${LINK_LIBS} ${PYTHON_LIBRARIES} ${BUDDY_LIBRARY})
    install(TARGETS ${PYTHON_PLUGIN_LIB} LIBRARY DESTINATION ${LIBRARY_INSTALL_DIRECTORY} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE INCLUDES DESTINATION ${INCLUDE_INSTALL_DIRECTORY})
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(${PYTHON_PLUGIN_LIB})
    endif()

if(WITH_GUI)
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT5_CMAKE_PATH})
    set(GUI_NAME "hal_gui")

    # We require qt5 widgets
    find_package(Qt5Widgets)
    if(Qt5Widgets_FOUND)
        message(STATUS "Qt5Widgets_INCLUDE_DIRS: ${Qt5Widgets_INCLUDE_DIRS}")
        message(STATUS "Qt5Widgets_LIBRARIES: ${Qt5Widgets_LIBRARIES}")
    elseif(NOT Qt5Widgets_FOUND)
        set(Missing_package "TRUE")
        if(APPLE AND CMAKE_HOST_APPLE)
            message(STATUS "To install qt5 on MacOS using homebrew run following command:")
            message(STATUS "    brew install qt")
        endif(APPLE AND CMAKE_HOST_APPLE)
    endif(Qt5Widgets_FOUND)
    find_package(Qt5Concurrent)
    if(Qt5Concurrent_FOUND)
        message(STATUS "Qt5Concurrent_INCLUDE_DIRS: ${Qt5Concurrent_INCLUDE_DIRS}")
        message(STATUS "Qt5Concurrent_LIBRARIES: ${Qt5Concurrent_LIBRARIES}")
    elseif(NOT Qt5Concurrent_FOUND)
        set(Missing_package "TRUE")
        if(APPLE AND CMAKE_HOST_APPLE)
            message(STATUS "To install qt5 on MacOS using homebrew run following command:")
            message(STATUS "    brew install qt5")
        endif(APPLE AND CMAKE_HOST_APPLE)
    endif(Qt5Concurrent_FOUND)
    find_package(Qt5Svg)
    if(Qt5Svg_FOUND)
        message(STATUS "Qt5Svg_INCLUDE_DIRS: ${Qt5Svg_INCLUDE_DIRS}")
        message(STATUS "Qt5Svg_LIBRARIES: ${Qt5Svg_LIBRARIES}")
    elseif(NOT Qt5Svg_FOUND)
        set(Missing_package "TRUE")
    endif(Qt5Svg_FOUND)

    set(CMAKE_INCLUDE_CURRENT_DIR ON)

    set(INCROOT "${CMAKE_SOURCE_DIR}/include/gui")
    set(SRCROOT "${CMAKE_SOURCE_DIR}/src/gui")
    set(UIROOT "${CMAKE_SOURCE_DIR}/ui")
    set(RSRCROOT "${CMAKE_SOURCE_DIR}/resources")

    include_directories(${include_directories}
                        ${CMAKE_SOURCE_DIR}/include
                        SYSTEM ${INCROOT}
                        )

    file(GLOB_RECURSE HDR ${INCROOT}/*.h)
    file(GLOB_RECURSE SRC ${SRCROOT}/*.cpp)
    aux_source_directory(${SRCROOT} SRC)
    file(GLOB_RECURSE UI ${UIROOT}/*.ui)
    file(GLOB_RECURSE RSRC ${RSRCROOT}/*.qrc)

    qt5_wrap_cpp(MOC_SRC ${SRC})
    qt5_wrap_cpp(MOC_HDR ${HDR})
    qt5_wrap_ui(MOC_UI ${UI})
    qt5_add_resources(MOC_RSRC ${RSRC})

    add_library(${GUI_NAME} SHARED
                ${HDR}
                ${SRC}
                #${MOC_SRC}
                ${MOC_HDR}
                ${MOC_UI}
                ${RSRC}
                ${MOC_RSRC}
                ${QM}
                )
    add_definitions(-DQT_NO_KEYWORDS)
    target_link_libraries(${GUI_NAME} ${LINK_LIBS} ${GRAPHVIZ_LIBRARIES} ${PYTHON_LIBRARIES} pybind11::embed Qt5::Widgets Qt5::Concurrent Qt5::Svg)

    install(TARGETS ${GUI_NAME} LIBRARY DESTINATION ${LIBRARY_INSTALL_DIRECTORY} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE INCLUDES DESTINATION ${INCLUDE_INSTALL_DIRECTORY})
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(${GUI_NAME})
    endif()
endif(WITH_GUI)

set(SRC main.cpp)

add_executable(${CMAKE_PROJECT_NAME} ${SRC})
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES ENABLE_EXPORTS 1)
if(WITH_GUI)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC WITH_GUI)
endif(WITH_GUI)
target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC SPDLOG_TRACE_ON)
target_link_libraries(${CMAKE_PROJECT_NAME} ${LINK_LIBS})
install(TARGETS ${CMAKE_PROJECT_NAME} RUNTIME DESTINATION ${BINARY_INSTALL_DIRECTORY} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE INCLUDES DESTINATION ${INCLUDE_INSTALL_DIRECTORY})
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_sanitizers(${CMAKE_PROJECT_NAME})
endif()
