option(PL_MODULE_IDENTIFICATION "PL_MODULE_IDENTIFICATION" OFF)

if(PL_MODULE_IDENTIFICATION OR BUILD_ALL_PLUGINS)
    file(GLOB_RECURSE MODULE_IDENTIFICATION_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE MODULE_IDENTIFICATION_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE MODULE_IDENTIFICATION_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)

    if(BITWUZLA_FOUND)
        add_compile_definitions(BITWUZLA_LIBRARY)
        message(STATUS "found bitwuzla, adding define for solver query to bitwuzla library")    
    endif()

    hal_add_plugin(module_identification
        SHARED
        HEADER ${MODULE_IDENTIFICATION_INC}
        SOURCES ${MODULE_IDENTIFICATION_SRC} ${MODULE_IDENTIFICATION_PYTHON_SRC}
        LINK_LIBRARIES boolean_influence dataflow
    )

    add_subdirectory(test)

endif()
