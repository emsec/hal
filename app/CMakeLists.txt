set(SRC main.cpp)

add_executable(hal ${SRC})
set_target_properties(hal PROPERTIES OUTPUT_NAME "hal")
set_target_properties(hal PROPERTIES ENABLE_EXPORTS 1)
set_target_properties(hal PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_RPATH}")
target_compile_options(hal
                       PUBLIC ${COMPILE_OPTIONS_PUBLIC}
                       PRIVATE ${COMPILE_OPTIONS_PRIVATE}
                       INTERFACE ${COMPILE_OPTIONS_INTERFACE})
target_compile_definitions(hal PUBLIC SPDLOG_TRACE_ON)
target_link_libraries(hal hal::core)
install(TARGETS hal RUNTIME DESTINATION ${BINARY_INSTALL_DIRECTORY} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE INCLUDES DESTINATION ${INCLUDE_INSTALL_DIRECTORY})
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_sanitizers(hal)
endif()
