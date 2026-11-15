option(PL_CLOCK_TREE_VIEWER "PL_CLOCK_TREE_VIEWER" OFF)

if(PL_CLOCK_TREE_VIEWER OR BUILD_ALL_PLUGINS)

    # This plugin plots clock trees extracted by the clock_tree_extractor plugin and links against it.
    # Plugin directories are added in alphabetical order, so the clock_tree_extractor target exists at
    # this point if that plugin is enabled. Without it, the viewer is skipped instead of failing the
    # whole configuration.
    if(NOT TARGET clock_tree_extractor)
        message(WARNING "The clock_tree_viewer plugin needs the clock_tree_extractor plugin and is not built, "
                        "set PL_CLOCK_TREE_EXTRACTOR=ON to build it.")
        return()
    endif()

    find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

    file(GLOB_RECURSE CLOCK_TREE_VIEWER_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE CLOCK_TREE_VIEWER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE CLOCK_TREE_VIEWER_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)

    qt6_wrap_cpp(MOC_HDR ${CLOCK_TREE_VIEWER_INC})

    hal_add_plugin(clock_tree_viewer
            SHARED
            HEADER ${CLOCK_TREE_VIEWER_INC}
            SOURCES ${CLOCK_TREE_VIEWER_SRC} ${CLOCK_TREE_VIEWER_PYTHON_SRC} ${MOC_HDR}
            LINK_LIBRARIES PUBLIC gui clock_tree_extractor Qt6::Core Qt6::Gui Qt6::Widgets
    )
endif()
