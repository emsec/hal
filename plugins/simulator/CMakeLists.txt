option(PL_SIMULATOR "PL_SIMULATOR" ON)
if(PL_SIMULATOR OR BUILD_ALL_PLUGINS)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR})

    # Check if pybind11 is being used directly or via add_subdirectory
    set(HAL_PLUGINS_MASTER_PROJECT OFF)
    if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(HAL_PLUGINS_MASTER_PROJECT ON)
    endif()

    subdirlist(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})
    foreach(subdir ${SUBDIRS})
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/CMakeLists.txt")
            add_subdirectory(${subdir})
        endif()
    endforeach()
endif()
