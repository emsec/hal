option(PL_LOGIC_EVALUATOR "PL_LOGIC_EVALUATOR" OFF)
if(PL_LOGIC_EVALUATOR OR BUILD_ALL_PLUGINS)

    find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

    if(Qt6Core_FOUND)
        message(VERBOSE "Qt6Core_INCLUDE_DIRS: ${Qt6Core_INCLUDE_DIRS}")
    elseif(NOT Qt6Core_FOUND)
        message(STATUS "Qt6Core not found for logic_evaluator")
    endif(Qt6Core_FOUND)


    file(GLOB_RECURSE LOGIC_EVALUATOR_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE LOGIC_EVALUATOR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE LOGIC_EVALUATOR_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)
    qt6_wrap_cpp(MOC_HDR ${LOGIC_EVALUATOR_INC})

    hal_add_plugin(logic_evaluator
                   SHARED
                   HEADER ${LOGIC_EVALUATOR_INC}
                   SOURCES ${LOGIC_EVALUATOR_SRC} ${LOGIC_EVALUATOR_PYTHON_SRC}  ${MOC_HDR}
                   LINK_LIBRARIES PUBLIC gui netlist_simulator_controller Qt6::Core Qt6::Gui Qt6::Widgets)

endif()
