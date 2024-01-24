option(PL_BITWUZLA_UTILS "PL_BITWUZLA_UTILS" ON)
if(PL_BITWUZLA_UTILS OR BUILD_ALL_PLUGINS)
    file(GLOB_RECURSE BITWUZLA_UTILS_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE BITWUZLA_UTILS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE BITWUZLA_UTILS_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)

    hal_add_plugin(bitwuzla_utils
                   SHARED
                   HEADER ${BITWUZLA_UTILS_INC}
                   SOURCES ${BITWUZLA_UTILS_SRC} ${BITWUZLA_UTILS_PYTHON_SRC}
                   LINK_LIBRARIES PUBLIC ${BITWUZLA_LIBRARIES} OpenMP::OpenMP_CXX
                   )
    
    add_subdirectory(test)
endif()
