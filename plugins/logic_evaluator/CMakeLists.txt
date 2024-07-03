option(PL_LOGIC_EVALUATOR "PL_LOGIC_EVALUATOR" ON)
if(PL_LOGIC_EVALUATOR OR BUILD_ALL_PLUGINS)

    find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

    if(Qt5Core_FOUND)
        message(VERBOSE "Qt5Core_INCLUDE_DIRS: ${Qt5Core_INCLUDE_DIRS}")
    elseif(NOT Qt5Core_FOUND)
        message(STATUS "Qt5Core not found for logic_evaluator")
    endif(Qt5Core_FOUND)


    file(GLOB_RECURSE LOGIC_EVALUATOR_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE LOGIC_EVALUATOR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE LOGIC_EVALUATOR_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)
    qt5_wrap_cpp(MOC_HDR ${LOGIC_EVALUATOR_INC})

    hal_add_plugin(logic_evaluator
                   SHARED
                   HEADER ${LOGIC_EVALUATOR_INC}
                   SOURCES ${LOGIC_EVALUATOR_SRC} ${LOGIC_EVALUATOR_PYTHON_SRC}  ${MOC_HDR}
                   LINK_LIBRARIES PUBLIC gui Qt5::Core Qt5::Gui Qt5::Widgets)

endif()
