option(PL_BOOLEAN_INFLUENCE "PL_BOOLEAN_INFLUENCE" OFF)
if(PL_BOOLEAN_INFLUENCE OR BUILD_ALL_PLUGINS)

    if(APPLE AND CMAKE_HOST_APPLE AND NOT Qt5_DIR)
        set (Qt5_DIR "/usr/local/opt/qt@5/lib/cmake" )
    endif()

    find_package(Qt5 COMPONENTS Core REQUIRED)

    if(Qt5Core_FOUND)
        message(VERBOSE "Qt5Core_INCLUDE_DIRS: ${Qt5Core_INCLUDE_DIRS}")
    elseif(NOT Qt5Core_FOUND)
        message(STATUS "Qt5Core not found for boolean_influence")
    endif(Qt5Core_FOUND)

    file(GLOB_RECURSE BOOLEAN_INFLUENCE_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE BOOLEAN_INFLUENCE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE BOOLEAN_INFLUENCE_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)
    qt5_wrap_cpp(MOC_HDR ${BOOLEAN_INFLUENCE_INC})

    hal_add_plugin(boolean_influence
                   SHARED
                   HEADER ${BOOLEAN_INFLUENCE_INC}
                   SOURCES ${BOOLEAN_INFLUENCE_SRC} ${BOOLEAN_INFLUENCE_PYTHON_SRC}  ${MOC_HDR}
                   LINK_LIBRARIES PUBLIC ${Z3_LIBRARIES} z3_utils OpenMP::OpenMP_CXX  Qt5::Core
                   PYDOC SPHINX_DOC_INDEX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/documentation/boolean_influence.rst
                   )
endif()
