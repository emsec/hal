file(GLOB_RECURSE NETLIST_LIB_INCLUDE ${CMAKE_SOURCE_DIR}/include/netlist/*.h)
file(GLOB_RECURSE NETLIST_LIB_SOURCE ${CMAKE_SOURCE_DIR}/src/netlist/*.cpp)

add_library(netlist SHARED ${NETLIST_LIB_INCLUDE} ${NETLIST_LIB_SOURCE} ${CMAKE_BINARY_DIR}/hal_version.h)
set_target_properties(netlist PROPERTIES OUTPUT_NAME "hal_netlist")
add_library(hal::netlist ALIAS netlist)

find_package(Z3 REQUIRED)
if(Z3_FOUND)
    message(STATUS "Found z3")
else()
    message(STATUS "Could not find z3")
endif(Z3_FOUND)

target_include_directories(netlist
                           PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
                           $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                           )
target_compile_options(netlist
                       PUBLIC ${COMPILE_OPTIONS_PUBLIC}
                       PRIVATE ${COMPILE_OPTIONS_PRIVATE}
                       INTERFACE ${COMPILE_OPTIONS_INTERFACE})
set_target_properties(PROPERTIES DEFINE_SYMBOL BUILDING_CORE)
target_link_libraries(netlist
                      PUBLIC
                        hal::utilities
                        std::filesystem
                        spdlog::spdlog
                        ${CMAKE_DL_LIBS}
                        RapidJSON::RapidJSON
                        ${Z3_LIBRARIES}
                      )
install(TARGETS netlist
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
    add_sanitizers(netlist)
endif()


