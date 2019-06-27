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

    include_directories(${BUDDY_INCLUDE_DIR})
    include_directories(AFTER "${CMAKE_CURRENT_SOURCE_DIR}/include")

    # Define the Header and sources files
    file(GLOB_RECURSE GATE_LIBRARIES_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE GATE_LIBRARIES_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

    add_library(gate_libraries SHARED ${GATE_LIBRARIES_SRC} ${GATE_LIBRARIES_INC})
    set_target_properties(gate_libraries PROPERTIES DEFINE_SYMBOL PLUGIN)
    if(APPLE AND CMAKE_HOST_APPLE)
        set_target_properties(gate_libraries PROPERTIES
                              INSTALL_NAME_DIR ${PLUGIN_LIBRARY_INSTALL_DIRECTORY}
                              )
    endif()
    target_link_libraries(gate_libraries  ${LINK_LIBS})
    install(TARGETS gate_libraries LIBRARY DESTINATION ${PLUGIN_LIBRARY_INSTALL_DIRECTORY} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE INCLUDES DESTINATION ${PLUGIN_INCLUDE_INSTALL_DIRECTORY})
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${PLUGIN_INCLUDE_INSTALL_DIRECTORY}/gate_libraries/include/)
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        install(FILES ${GATE_LIBS} DESTINATION §${SHARE_INSTALL_DIRECTORY})
        add_sanitizers(gate_libraries)
    endif()

endif()