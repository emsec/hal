file(GLOB_RECURSE UTILITIES_LIB_INCLUDE ${CMAKE_SOURCE_DIR}/include/utilities/*.h)
file(GLOB_RECURSE UTILITIES_LIB_SOURCE ${CMAKE_SOURCE_DIR}/src/utilities/*.cpp)

add_library(utilities SHARED ${UTILITIES_LIB_INCLUDE} ${UTILITIES_LIB_SOURCE} ${CMAKE_BINARY_DIR}/hal_version.h)
set_target_properties(utilities PROPERTIES OUTPUT_NAME "hal_utilities")
add_library(hal::utilities ALIAS utilities)
target_include_directories(utilities
                           PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
                           $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                           )
target_compile_options(utilities
                       PUBLIC ${COMPILE_OPTIONS_PUBLIC}
                       PRIVATE ${COMPILE_OPTIONS_PRIVATE}
                       INTERFACE ${COMPILE_OPTIONS_INTERFACE})
set_target_properties(PROPERTIES DEFINE_SYMBOL BUILDING_CORE)
target_link_libraries(utilities
                      PUBLIC
                        std::filesystem
                        spdlog::spdlog
                        ${CMAKE_DL_LIBS}
                        RapidJSON::RapidJSON
                      )
install(TARGETS utilities
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
    add_sanitizers(utilities)
endif()
