option(PL_GUI "PL_GUI" ON)
if(PL_GUI OR BUILD_ALL_PLUGINS)
    cmake_minimum_required(VERSION 3.1.0)


    if(APPLE AND CMAKE_HOST_APPLE AND NOT Qt5_DIR)
        set (Qt5_DIR "/usr/local/opt/qt@5/lib/cmake" )
    endif(APPLE AND CMAKE_HOST_APPLE AND NOT Qt5_DIR)    

    find_package(Qt5 COMPONENTS Core REQUIRED )

    find_package(Qt5 COMPONENTS Widgets REQUIRED)
    if(Qt5Widgets_FOUND)
        message(VERBOSE "Qt5Widgets_INCLUDE_DIRS: ${Qt5Widgets_INCLUDE_DIRS}")
        message(VERBOSE "Qt5Widgets_LIBRARIES: ${Qt5Widgets_LIBRARIES}")
    elseif(NOT Qt5Widgets_FOUND)
        set(Missing_package "TRUE")
        if(APPLE AND CMAKE_HOST_APPLE)
            message(STATUS "To install qt5 on MacOS using homebrew run following command:")
            message(STATUS "    brew install qt@5")
        endif(APPLE AND CMAKE_HOST_APPLE)
    endif(Qt5Widgets_FOUND)

    find_package(Qt5 COMPONENTS Concurrent REQUIRED)
    if(Qt5Concurrent_FOUND)
        message(VERBOSE "Qt5Concurrent_INCLUDE_DIRS: ${Qt5Concurrent_INCLUDE_DIRS}")
        message(VERBOSE "Qt5Concurrent_LIBRARIES: ${Qt5Concurrent_LIBRARIES}")
    elseif(NOT Qt5Concurrent_FOUND)
        set(Missing_package "TRUE")
        if(APPLE AND CMAKE_HOST_APPLE)
            message(STATUS "To install qt5 on MacOS using homebrew run following command:")
            message(STATUS "    brew install qt@5")
        endif(APPLE AND CMAKE_HOST_APPLE)
    endif(Qt5Concurrent_FOUND)

    find_package(Qt5 COMPONENTS Svg REQUIRED)
    if(Qt5Svg_FOUND)
        message(VERBOSE "Qt5Svg_INCLUDE_DIRS: ${Qt5Svg_INCLUDE_DIRS}")
        message(VERBOSE "Qt5Svg_LIBRARIES: ${Qt5Svg_LIBRARIES}")
    elseif(NOT Qt5Svg_FOUND)
        set(Missing_package "TRUE")
    endif(Qt5Svg_FOUND)

    set(INCROOT ${CMAKE_CURRENT_SOURCE_DIR}/include)
    set(SRCROOT ${CMAKE_CURRENT_SOURCE_DIR}/src)
    set(UIROOT ${CMAKE_CURRENT_SOURCE_DIR}/ui)
    set(RSRCROOT ${CMAKE_CURRENT_SOURCE_DIR}/resources)

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/defaults/guidefaults.ini ${CMAKE_BINARY_DIR}/share/hal/defaults/guidefaults.ini @ONLY)

    include_directories(${include_directories}
                        ${CMAKE_CURRENT_SOURCE_DIR}/include
                        SYSTEM ${INCROOT}
                        )

    file(GLOB_RECURSE HDR ${INCROOT}/*.h)
    file(GLOB_RECURSE SRC ${SRCROOT}/*.cpp)
    aux_source_directory(${SRCROOT} SRC)
    file(GLOB_RECURSE RSRC ${RSRCROOT}/*.qrc)

    qt5_wrap_cpp(MOC_source ${SRC})

    foreach(i IN ITEMS "" "_DEBUG" "_RELEASE" "_MINSIZEREL" "_RELWITHDEBINFO")
        foreach(j IN ITEMS "RUNTIME" "ARCHIVE" "LIBRARY")
            set(STORE_${j}_OUTDIR${i} ${CMAKE_${j}_OUTPUT_DIRECTORY${i}})
            set(CMAKE_${j}_OUTPUT_DIRECTORY${i} "${CMAKE_BINARY_DIR}/lib")
        endforeach()
    endforeach()
    if(USE_VENDORED_QUAZIP)
        set (QUAZIP_SUBDIR "${CMAKE_SOURCE_DIR}/deps/quazip-1.3")
        add_subdirectory(${QUAZIP_SUBDIR} ${CMAKE_BINARY_DIR}/quazip)
        message(STATUS "Using quazip from ${QUAZIP_SUBDIR}")
    else()
        find_package(QuaZip-Qt5 REQUIRED)
        get_target_property(QUAZIP_LIBRARIES QuaZip::QuaZip IMPORTED_LOCATION_RELEASE)
        message(STATUS "Using system's quazip from ${QUAZIP_LIBRARIES}")
    endif()

    foreach(i IN ITEMS "" "_DEBUG" "_RELEASE" "_MINSIZEREL" "_RELWITHDEBINFO")
        foreach(j IN ITEMS "RUNTIME" "ARCHIVE" "LIBRARY")
            set(CMAKE_${j}_OUTPUT_DIRECTORY${i} ${STORE_${j}_OUTDIR${i}})
        endforeach()
    endforeach()

    qt5_wrap_cpp(MOC_HDR ${HDR})
    qt5_wrap_ui(MOC_UI ${UI})
    qt5_add_resources(MOC_RSRC ${RSRC})

    add_library(gui SHARED
                ${HDR}
                ${SRC}
                #${MOC_source}
                ${MOC_HDR}
                ${MOC_UI}
                ${RSRC}
                ${MOC_RSRC}
                ${QM}
                )
    add_library(hal::gui ALIAS gui)
    set_target_properties(gui PROPERTIES OUTPUT_NAME "hal_gui")
    set_target_properties(gui PROPERTIES PREFIX "")
    if(APPLE AND CMAKE_HOST_APPLE)
        set_target_properties(gui PROPERTIES SUFFIX ".so")
    endif(APPLE AND CMAKE_HOST_APPLE)
    target_include_directories(gui
                               PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                               $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
                               $<BUILD_INTERFACE:${QuaZip_Library_SOURCE_DIR}>
                               $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                               )
    target_compile_options(gui
                           PUBLIC ${COMPILE_OPTIONS_PUBLIC}
                           PRIVATE ${COMPILE_OPTIONS_PRIVATE}
                           INTERFACE ${COMPILE_OPTIONS_INTERFACE})
    target_compile_definitions(gui PUBLIC QT_NO_KEYWORDS)
    target_link_libraries(gui PUBLIC hal::core hal::netlist PRIVATE ${Python3_LIBRARIES} pybind11::embed Qt5::Widgets Qt5::Concurrent Qt5::Svg QuaZip::QuaZip patchelf)
    add_dependencies(gui update_internal_gate_library_definitions)
    get_property(GLDIR GLOBAL PROPERTY GATE_LIBS_DEFINITIONS)
    file(WRITE "${RSRCROOT}/path/gate_library_definitions.txt" "${GLDIR}")
    hal_plugin_documentation(gui SPHINX_DOC_INDEX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/documentation/gui.rst)

    install(TARGETS gui
            EXPORT hal
            LIBRARY DESTINATION ${PLUGIN_LIBRARY_INSTALL_DIRECTORY}
            PERMISSIONS
            OWNER_READ
            OWNER_WRITE
            OWNER_EXECUTE
            GROUP_READ
            GROUP_EXECUTE
            WORLD_READ
            WORLD_EXECUTE
            INCLUDES DESTINATION ${PLUGIN_INCLUDE_INSTALL_DIRECTORY})

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(gui)
    endif()

endif()
