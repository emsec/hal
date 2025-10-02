option(PL_HELIX "Build the Helix plugin" ON)

if(PL_HELIX OR BUILD_ALL_PLUGINS)

    # Dependencies
    add_subdirectory(deps)
    find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

    # Include-What-You-Use (optional)
    if(IWYU)
        set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "include-what-you-use")
        message(STATUS "include-what-you-use turned ON")
    else()
        message(STATUS "include-what-you-use turned OFF")
    endif()

    # --- Source Collection ---
    file(GLOB_RECURSE HELIX_INC "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
    file(GLOB_RECURSE HELIX_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc")
    file(GLOB_RECURSE HELIX_PYTHON_SRC "${CMAKE_CURRENT_SOURCE_DIR}/python/*.cc")

    # --- Plugin Target ---
    hal_add_plugin(helix
        SHARED
        HEADER  ${HELIX_INC}
        SOURCES ${HELIX_SRC} ${HELIX_PYTHON_SRC}
        COMPILE_OPTIONS "-march=native"
    )

    # --- Includes ---
    target_include_directories(helix PRIVATE
        "${PROJECT_SOURCE_DIR}/plugins/gui/include"
    )

    # --- Link Libraries ---
    target_link_libraries(helix PRIVATE
        Qt5::Core
        Qt5::Widgets
        pthread
        event
        event_pthreads
        hiredis
        gui
    )

endif()
