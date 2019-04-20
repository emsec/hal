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

        list(GET VERSION_LIST 0 RETURN              )
        list(GET VERSION_LIST 1 MAJOR               )
        list(GET VERSION_LIST 2 MINOR               )
        list(GET VERSION_LIST 3 PATCH               )
        list(GET VERSION_LIST 4 TWEAK               )
        list(GET VERSION_LIST 5 ADDITIONAL_COMMITS  )
        list(GET VERSION_LIST 6 HASH                )
        list(GET VERSION_LIST 7 DIRTY               )
        list(GET VERSION_LIST 8 BROKEN              )

    set(HAL_VERSION_RETURN              ${RETURN} PARENT_SCOPE)
    set(HAL_VERSION_MAJOR               ${MAJOR} PARENT_SCOPE)
    set(HAL_VERSION_MINOR               ${MINOR} PARENT_SCOPE)
    set(HAL_VERSION_PATCH               ${PATCH} PARENT_SCOPE)
    set(HAL_VERSION_TWEAK               ${TWEAK} PARENT_SCOPE)
    set(HAL_VERSION_ADDITIONAL_COMMITS  ${ADDITIONAL_COMMITS} PARENT_SCOPE)
    set(HAL_VERSION_HASH                ${HASH} PARENT_SCOPE)
    set(HAL_VERSION_DIRTY               ${DIRTY} PARENT_SCOPE)
    set(HAL_VERSION_BROKEN              ${BROKEN} PARENT_SCOPE)
endfunction()
