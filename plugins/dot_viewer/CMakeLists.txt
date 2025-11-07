option(PL_DOT_VIEWER "Build the Dot Viewer plugin" ON)

if(PL_DOT_VIEWER OR BUILD_ALL_PLUGINS)

    # Dependencies
    find_package(Graphviz REQUIRED)
    find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/QGVCore)

    # --- Source Collection ---
    file(GLOB_RECURSE DOT_VIEWER_INC "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
    file(GLOB_RECURSE DOT_VIEWER_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
    file(GLOB_RECURSE DOT_VIEWER_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)


    qt5_wrap_cpp(MOC_HDR ${DOT_VIEWER_INC})

    # --- Plugin Target ---
    hal_add_plugin(dot_viewer
        SHARED
        HEADER  ${DOT_VIEWER_INC}
        SOURCES ${DOT_VIEWER_SRC} ${DOT_VIEWER_PYTHON_SRC} ${MOC_HDR}
        COMPILE_OPTIONS "-march=native"
    )

    target_sources(dot_viewer
        PRIVATE
            ${DOT_VIEWER_SRC}
    )

    # --- Includes ---
    target_include_directories(dot_viewer PRIVATE
        ${PROJECT_SOURCE_DIR}/plugins/gui/include
        ${CMAKE_CURRENT_SOURCE_DIR}/deps
    )

    # --- Link Libraries ---
    target_link_libraries(dot_viewer PRIVATE
        Qt5::Core
        Qt5::Widgets
        ${LIBGVC_LIBRARIES}
        ${LIBCGRAPH_LIBRARIES}
        gui
        qgvcore
    )

endif()
