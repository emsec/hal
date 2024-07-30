# Define the option to enable or disable the build of this plugin by a cmake flag.
option(PL_GATE_LIBRARIES "PL_GATE_LIBRARIES" ON)
if(PL_GATE_LIBRARIES OR BUILD_ALL_PLUGINS)
    # Add the include directory to the include search path
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/share/hal/gate_libraries/)
    file(GLOB GATE_LIBS ${CMAKE_CURRENT_SOURCE_DIR}/definitions/*)
    set_property(GLOBAL PROPERTY GATE_LIBS_DEFINITIONS "${CMAKE_CURRENT_SOURCE_DIR}/definitions")
    message(STATUS "GATE_LIBS: ${GATE_LIBS}")
    foreach(file ${GATE_LIBS})
        get_filename_component(f ${file} NAME)
        list(APPEND SOURCE_GATE_LIBS ${CMAKE_CURRENT_SOURCE_DIR}/definitions/${f})
        list(APPEND DESTINATION_GATE_LIBS ${CMAKE_BINARY_DIR}/share/hal/gate_libraries/${f})
    endforeach()


    file(GLOB HELPER_GATE_LIBS ${CMAKE_CURRENT_SOURCE_DIR}/definitions/helper_libs/*)
    message(STATUS "HELPER_GATE_LIBS: ${HELPER_GATE_LIBS}")
    foreach(file ${HELPER_GATE_LIBS})
        get_filename_component(f ${file} NAME)
        list(APPEND SOURCE_HELPER_GATE_LIBS ${CMAKE_CURRENT_SOURCE_DIR}/definitions/helper_libs/${f})
        list(APPEND DESTINATION_HELPER_GATE_LIBS ${CMAKE_BINARY_DIR}/share/hal/gate_libraries/${f})
    endforeach()


    add_custom_target(update_internal_gate_library_definitions ALL
                      BYPRODUCTS ${DESTINATION_GATE_LIBS} ${DESTINATION_HELPER_GATE_LIBS}
                      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/definitions/* ${CMAKE_BINARY_DIR}/share/hal/gate_libraries/
                      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/definitions/helper_libs/* ${CMAKE_BINARY_DIR}/share/hal/gate_libraries/
                      DEPENDS ${SOURCE_GATE_LIBS}  ${SOURCE_HELPER_GATE_LIBS}
                      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                      COMMENT "Update gate library definitions")
endif()
