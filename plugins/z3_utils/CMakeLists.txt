option(PL_Z3_UTILS "PL_Z3_UTILS" ON)
if(PL_Z3_UTILS OR BUILD_ALL_PLUGINS)
    file(GLOB_RECURSE Z3_UTILS_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE Z3_UTILS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE Z3_UTILS_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)

    hal_add_plugin(z3_utils
                   SHARED
                   HEADER ${Z3_UTILS_INC}
                   SOURCES ${Z3_UTILS_SRC} ${Z3_UTILS_PYTHON_SRC}
                   LINK_LIBRARIES PUBLIC ${Z3_LIBRARIES} OpenMP::OpenMP_CXX nlohmann_json::nlohmann_json
                   )
    
    add_subdirectory(test)
endif()
