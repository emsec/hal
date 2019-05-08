#-------------------------------------------------------------------------------
# Get version
#-------------------------------------------------------------------------------
function(hal_get_version)
    execute_process(
            COMMAND ${CMAKE_SOURCE_DIR}/tools/genversion.py
            OUTPUT_VARIABLE VERSION_LIST
            OUTPUT_STRIP_TRAILING_WHITESPACE
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

    if(VERSION_LIST STREQUAL "")
        file(READ ${CMAKE_SOURCE_DIR}/CURRENT_VERSION VERSION_FILE)
        message(STATUS "VERSION_FILE: ${VERSION_FILE}")
        string(REGEX MATCHALL "v([0-9]+)\.([0-9]+)\.([0-9]+)" match ${VERSION_FILE})
        if(NOT ${match} EQUAL "")
            set(HAL_VERSION_RETURN ${VERSION_FILE} PARENT_SCOPE)
            message(STATUS "HAL_VERSION_RETURN: ${HAL_VERSION_RETURN}")
            set(HAL_VERSION_MAJOR ${CMAKE_MATCH_1} PARENT_SCOPE)
            set(HAL_VERSION_MINOR ${CMAKE_MATCH_2} PARENT_SCOPE)
            set(HAL_VERSION_PATCH ${CMAKE_MATCH_3} PARENT_SCOPE)
            set(HAL_VERSION_TWEAK 0 PARENT_SCOPE)
            set(HAL_VERSION_ADDITIONAL_COMMITS 0 PARENT_SCOPE)
            set(HAL_VERSION_HASH "unknown" PARENT_SCOPE)
            set(HAL_VERSION_DIRTY false PARENT_SCOPE)
            set(HAL_VERSION_BROKEN false PARENT_SCOPE)
        endif()
    else()

        list(GET VERSION_LIST 0 RETURN)
        list(GET VERSION_LIST 1 MAJOR)
        list(GET VERSION_LIST 2 MINOR)
        list(GET VERSION_LIST 3 PATCH)
        list(GET VERSION_LIST 4 TWEAK)
        list(GET VERSION_LIST 5 ADDITIONAL_COMMITS)
        list(GET VERSION_LIST 6 HASH)
        list(GET VERSION_LIST 7 DIRTY)
        list(GET VERSION_LIST 8 BROKEN)

        set(HAL_VERSION_RETURN ${RETURN} PARENT_SCOPE)
        set(HAL_VERSION_MAJOR ${MAJOR} PARENT_SCOPE)
        set(HAL_VERSION_MINOR ${MINOR} PARENT_SCOPE)
        set(HAL_VERSION_PATCH ${PATCH} PARENT_SCOPE)
        set(HAL_VERSION_TWEAK ${TWEAK} PARENT_SCOPE)
        set(HAL_VERSION_ADDITIONAL_COMMITS ${ADDITIONAL_COMMITS} PARENT_SCOPE)
        set(HAL_VERSION_HASH ${HASH} PARENT_SCOPE)
        set(HAL_VERSION_DIRTY ${DIRTY} PARENT_SCOPE)
        set(HAL_VERSION_BROKEN ${BROKEN} PARENT_SCOPE)
    endif()
endfunction()
