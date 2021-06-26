
add_subdirectory(utilities)
add_subdirectory(netlist)
add_subdirectory(plugin_system)
add_subdirectory(python_bindings)


add_library(core SHARED ${CMAKE_BINARY_DIR}/hal_version.h)
set_target_properties(core PROPERTIES LINKER_LANGUAGE CXX)
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

set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED COMPONENTS system)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
elseif(NOT Boost_FOUND)
    set(Missing_package "TRUE")
    if(APPLE AND CMAKE_HOST_APPLE)
        message(STATUS "To install boost on MacOS using homebrew run following command:")
        message(STATUS "    brew install boost --c++11")
    endif(APPLE AND CMAKE_HOST_APPLE)
endif(Boost_FOUND)

target_link_libraries(core
                      PUBLIC
                      hal::utilities
                      hal::netlist
                      hal::plugin_system
                      hal::py
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
# core has no own sources so no sanitizers needed
# if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
#     add_sanitizers(core)
# endif()
