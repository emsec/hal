option(PL_MEDIA_VIEWER "Build the Media Viewer plugin" ON)

if(PL_MEDIA_VIEWER OR BUILD_ALL_PLUGINS)

    find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia MultimediaWidgets)

    file(GLOB_RECURSE MEDIA_VIEWER_INC "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
    file(GLOB_RECURSE MEDIA_VIEWER_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

    qt6_wrap_cpp(MOC_HDR ${MEDIA_VIEWER_INC})

    hal_add_plugin(media_viewer
        SHARED
        HEADER  ${MEDIA_VIEWER_INC}
        SOURCES ${MEDIA_VIEWER_SRC} ${MOC_HDR}
        PYDOC SPHINX_DOC_INDEX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/documentation/media_viewer.rst
    )

    target_include_directories(media_viewer PRIVATE
        ${PROJECT_SOURCE_DIR}/plugins/gui/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_link_libraries(media_viewer PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Multimedia
        Qt6::MultimediaWidgets
        gui
    )

endif()
