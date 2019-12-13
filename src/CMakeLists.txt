add_subdirectory(core)
add_subdirectory(netlist)
add_subdirectory(python-binding)

if(WITH_GUI)
    set(GUI_NAME "hal_gui")

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

    install(TARGETS ${GUI_NAME} EXPORT ${GUI_NAME} LIBRARY DESTINATION ${LIBRARY_INSTALL_DIRECTORY} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE INCLUDES DESTINATION ${INCLUDE_INSTALL_DIRECTORY})
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(${GUI_NAME})
    endif()
endif(WITH_GUI)