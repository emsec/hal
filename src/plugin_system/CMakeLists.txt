file(GLOB_RECURSE PLUGIN_SYSTEM_LIB_INCLUDE ${CMAKE_SOURCE_DIR}/include/plugin_system/*.h)
file(GLOB_RECURSE PLUGIN_SYSTEM_LIB_SOURCE ${CMAKE_SOURCE_DIR}/src/plugin_system/*.cpp)

add_library(plugin_system SHARED ${PLUGIN_SYSTEM_LIB_INCLUDE} ${PLUGIN_SYSTEM_LIB_SOURCE} ${CMAKE_BINARY_DIR}/hal_version.h)
set_target_properties(plugin_system PROPERTIES OUTPUT_NAME "hal_plugin_system")
add_library(hal::plugin_system ALIAS plugin_system)
target_include_directories(plugin_system
                           PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
                           $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                           )
target_compile_options(plugin_system
                       PUBLIC ${COMPILE_OPTIONS_PUBLIC}
                       PRIVATE ${COMPILE_OPTIONS_PRIVATE}
                       INTERFACE ${COMPILE_OPTIONS_INTERFACE})
set_target_properties(PROPERTIES DEFINE_SYMBOL BUILDING_CORE)
target_link_libraries(plugin_system
                      PUBLIC
                        hal::utilities
                        hal::netlist
                        std::filesystem
                        spdlog::spdlog
                        ${CMAKE_DL_LIBS}
                        RapidJSON::RapidJSON
                      )
install(TARGETS plugin_system
        EXPORT hal
        LIBRARY DESTINATION ${LIBRARY_INSTALL_DIRECTORY}
        PERMISSIONS
            OWNER_READ
            OWNER_WRITE
            OWNER_EXECUTE
            GROUP_READ
            GROUP_EXECUTE
            WORLD_READ
            WORLD_EXECUTE
        )
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_sanitizers(plugin_system)
endif()


