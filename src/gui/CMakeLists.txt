if(WITH_GUI)
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

    qt5_wrap_cpp(MOC_source ${SRC})
    qt5_wrap_cpp(MOC_HDR ${HDR})
    qt5_wrap_ui(MOC_UI ${UI})
    qt5_add_resources(MOC_RSRC ${RSRC})

    add_library(hal_gui SHARED
                ${HDR}
                ${SRC}
                #${MOC_source}
                ${MOC_HDR}
                ${MOC_UI}
                ${RSRC}
                ${MOC_RSRC}
                ${QM}
                )
    target_include_directories(hal_gui
                               PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
                               $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
                               $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                               )
    target_compile_options(hal_gui
                           PUBLIC ${COMPILE_OPTIONS_PUBLIC}
                           PRIVATE ${COMPILE_OPTIONS_PRIVATE}
                           INTERFACE ${COMPILE_OPTIONS_INTERFACE})
    target_compile_definitions(hal_gui PUBLIC QT_NO_KEYWORDS)
    target_link_libraries(hal_gui PUBLIC hal::core hal::netlist Boost::system ${PYTHON_LIBRARIES} pybind11::embed Qt5::Widgets Qt5::Concurrent Qt5::Svg)

    install(TARGETS hal_gui LIBRARY DESTINATION ${LIBRARY_INSTALL_DIRECTORY} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE INCLUDES DESTINATION ${INCLUDE_INSTALL_DIRECTORY})
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(hal_gui)
    endif()
endif(WITH_GUI)
