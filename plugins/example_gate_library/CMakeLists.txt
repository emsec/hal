option(PL_DECORATOR_EXAMPLE_LIB "PL_DECORATOR_EXAMPLE_LIB" ON)
if(PL_DECORATOR_EXAMPLE_LIB OR BUILD_ALL_PLUGINS)

    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/share/hal/gate_libraries/)
    file(GLOB_RECURSE GATE_LIBS ${CMAKE_CURRENT_SOURCE_DIR}/gate_libraries/*.json)
    message(STATUS "GATE_LIBS: ${GATE_LIBS}")
    foreach(file ${GATE_LIBS})
        get_filename_component(f ${file} NAME)
        list(APPEND SOURCE_GATE_LIBS ${CMAKE_CURRENT_SOURCE_DIR}/gate_libraries/${f})
        list(APPEND DESTINATION_GATE_LIBS ${CMAKE_BINARY_DIR}/share/hal/gate_libraries/${f})
    endforeach()
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gate_libraries/EXAMPLE_LIB.json ${CMAKE_BINARY_DIR}/share/hal/gate_libraries/EXAMPLE_LIB.json)
    add_custom_target(update_decorator_example_lib_libraries ALL
                    BYPRODUCTS ${DESTINATION_GATE_LIBS}
                    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/gate_libraries/*.json ${CMAKE_BINARY_DIR}/share/hal/gate_libraries/
                    DEPENDS ${SOURCE_GATE_LIBS}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    COMMENT "Update gate libraries")

    include_directories(${BUDDY_INCLUDE_DIR})
    include_directories(AFTER "${CMAKE_CURRENT_SOURCE_DIR}/include")

    file(GLOB_RECURSE DECORATOR_EXAMPLE_LIB_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE DECORATOR_EXAMPLE_LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

    add_library(decorator_example_lib SHARED ${DECORATOR_EXAMPLE_LIB_SRC} ${DECORATOR_EXAMPLE_LIB_INC})
    add_dependencies(decorator_example_lib update_decorator_example_lib_libraries)
    set_target_properties(decorator_example_lib PROPERTIES DEFINE_SYMBOL PLUGIN)
    if(APPLE AND CMAKE_HOST_APPLE)
        set_target_properties(decorator_example_lib PROPERTIES
                              INSTALL_NAME_DIR ${PLUGIN_LIBRARY_INSTALL_DIRECTORY}
                              )
    endif()
    target_link_libraries(decorator_example_lib ${LINK_LIBS})
    install(TARGETS decorator_example_lib LIBRARY DESTINATION ${PLUGIN_LIBRARY_INSTALL_DIRECTORY} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE INCLUDES DESTINATION ${PLUGIN_INCLUDE_INSTALL_DIRECTORY})
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${PLUGIN_INCLUDE_INSTALL_DIRECTORY}/decorator_example_lib/)
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(decorator_example_lib)
    endif()
endif()
