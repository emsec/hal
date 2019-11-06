# Define the option to enable or disable the build of this plugin by a cmake flag.
option(PL_GATE_LIBRARIES "PL_GATE_LIBRARIES" ON)
if(PL_GATE_LIBRARIES OR BUILD_ALL_PLUGINS)
    # Add the include directory to the include search path
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/share/hal/gate_libraries/)
    file(GLOB_RECURSE GATE_LIBS ${CMAKE_CURRENT_SOURCE_DIR}/definitions/*.json)
    message(STATUS "GATE_LIBS: ${GATE_LIBS}")
    foreach(file ${GATE_LIBS})
        get_filename_component(f ${file} NAME)
        list(APPEND SOURCE_GATE_LIBS ${CMAKE_CURRENT_SOURCE_DIR}/definitions/${f})
        list(APPEND DESTINATION_GATE_LIBS ${CMAKE_BINARY_DIR}/share/hal/gate_libraries/${f})
    endforeach()
    add_custom_target(update_internal_gate_library_definitions ALL
                      BYPRODUCTS ${DESTINATION_GATE_LIBS}
                      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/definitions/*.json ${CMAKE_BINARY_DIR}/share/hal/gate_libraries/
                      DEPENDS ${SOURCE_GATE_LIBS}
                      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                      COMMENT "Update gate library definitions")
endif()