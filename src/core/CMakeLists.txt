#
# core library
#

configure_file(${CMAKE_SOURCE_DIR}/include/hal_version.h.in ${CMAKE_BINARY_DIR}/hal_version.h @ONLY)

set(CORE_LIB_HDR
    ${CMAKE_SOURCE_DIR}/include/core/callback_hook.h
    ${CMAKE_SOURCE_DIR}/include/core/special_strings.h
    ${CMAKE_SOURCE_DIR}/include/core/hal_file_manager.h
    ${CMAKE_SOURCE_DIR}/include/core/interface_base.h
    ${CMAKE_SOURCE_DIR}/include/core/interface_cli.h
    ${CMAKE_SOURCE_DIR}/include/core/interface_gui.h
    ${CMAKE_SOURCE_DIR}/include/core/interface_interactive_ui.h
    ${CMAKE_SOURCE_DIR}/include/core/library_loader.h
    ${CMAKE_SOURCE_DIR}/include/core/log.h
    ${CMAKE_SOURCE_DIR}/include/core/plugin_manager.h
    ${CMAKE_SOURCE_DIR}/include/core/program_arguments.h
    ${CMAKE_SOURCE_DIR}/include/core/program_options.h
    ${CMAKE_SOURCE_DIR}/include/core/token_stream.h
    ${CMAKE_SOURCE_DIR}/include/core/utils.h
    ${CMAKE_SOURCE_DIR}/include/arch_linux.h
    ${CMAKE_SOURCE_DIR}/include/arch_mac.h
    ${CMAKE_SOURCE_DIR}/include/arch_win.h
    ${CMAKE_SOURCE_DIR}/include/def.h
    ${CMAKE_SOURCE_DIR}/include/doxy_groups.h
    )

set(CORE_LIB_source
    hal_file_manager.cpp
    interface_base.cpp
    library_loader.cpp
    log.cpp
    plugin_manager.cpp
    program_arguments.cpp
    program_options.cpp
    utils.cpp
    )

add_library(core SHARED ${CORE_LIB_HDR} ${CORE_LIB_source} ${CMAKE_BINARY_DIR}/hal_version.h)
set_target_properties(core PROPERTIES OUTPUT_NAME "hal_core")
add_library(hal::core ALIAS core)
target_include_directories(core
                           PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
                           $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                           )
target_compile_options(core
                       PUBLIC ${COMPILE_OPTIONS_PUBLIC}
                       PRIVATE ${COMPILE_OPTIONS_PRIVATE}
                       INTERFACE ${COMPILE_OPTIONS_INTERFACE})
set_target_properties(PROPERTIES DEFINE_SYMBOL BUILDING_CORE)
target_link_libraries(core
                      PUBLIC
                        std::filesystem
                        spdlog::spdlog
                        ${CMAKE_DL_LIBS}
                        RapidJSON::RapidJSON
                      PRIVATE
                        Boost::system
                      )
install(TARGETS core
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
    add_sanitizers(core)
endif()
