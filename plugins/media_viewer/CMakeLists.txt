option(PL_MEDIA_VIEWER "Build the Media Viewer plugin" ON)

if(PL_MEDIA_VIEWER OR BUILD_ALL_PLUGINS)

    # Dependencies
    find_package(Qt5 REQUIRED COMPONENTS Core Widgets Multimedia MultimediaWidgets)

    # --- Source Collection ---
    file(GLOB_RECURSE MEDIA_VIEWER_INC "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
    file(GLOB_RECURSE MEDIA_VIEWER_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
    file(GLOB_RECURSE MEDIA_VIEWER_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)


    qt5_wrap_cpp(MOC_HDR ${MEDIA_VIEWER_INC})

    # --- Plugin Target ---
    hal_add_plugin(media_viewer
        SHARED
        HEADER  ${MEDIA_VIEWER_INC}
        SOURCES ${MEDIA_VIEWER_SRC} ${MEDIA_VIEWER_PYTHON_SRC} ${MOC_HDR}
        COMPILE_OPTIONS "-march=native"
    )

    target_sources(media_viewer
        PRIVATE
            ${MEDIA_VIEWER_SRC}
    )

    # --- Includes ---
    target_include_directories(media_viewer PRIVATE
        ${PROJECT_SOURCE_DIR}/plugins/gui/include
        ${CMAKE_CURRENT_SOURCE_DIR}/deps
    )

    # --- Link Libraries ---
    target_link_libraries(media_viewer PRIVATE
        Qt5::Core
        Qt5::Widgets
        Qt5::Multimedia
        Qt5::MultimediaWidgets
        gui
    )

endif()
