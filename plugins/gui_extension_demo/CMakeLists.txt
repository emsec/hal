option(PL_GUI_EXTENSION_DEMO "PL_GUI_EXTENSION_DEMO" OFF)
if(PL_GUI_EXTENSION_DEMO OR BUILD_ALL_PLUGINS)
    file(GLOB_RECURSE GUI_EXTENSION_DEMO_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE GUI_EXTENSION_DEMO_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE GUI_EXTENSION_DEMO_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)

    hal_add_plugin(gui_extension_demo
                   SHARED
                   HEADER ${GUI_EXTENSION_DEMO_INC}
                   SOURCES ${GUI_EXTENSION_DEMO_SRC} ${GUI_EXTENSION_DEMO_PYTHON_SRC}
                   LINK_LIBRARIES PUBLIC netlist_simulator_controller
                   )
endif()
