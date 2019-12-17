#
# netlist + hdl parser + hdl file writer library
#
file(GLOB_RECURSE NETLIST_LIB_HDR CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/include/netlist/*.h)
file(GLOB_RECURSE NETLIST_LIB_SRC CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/netlist/*.cpp)

add_library(netlist
            SHARED
                ${NETLIST_LIB_HDR}
                ${HDL_PARSER_LIB_HDR}
                ${HDL_WRITER_LIB_HDR}
                ${NETLIST_LIB_SRC}
                ${HDL_PARSER_LIB_SRC}
                ${HDL_WRITER_LIB_SRC}
                ${GATE_TYPE_SYSTEM_SRC}
                ${GATE_TYPE_SYSTEM_INC}
            )
set_target_properties(netlist PROPERTIES OUTPUT_NAME "hal_netlist")
add_library(hal::netlist ALIAS netlist)
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
set_target_properties(netlist PROPERTIES DEFINE_SYMBOL API_NETLIST)
target_link_libraries(netlist
                      PUBLIC
                        hal::core
                      )
install(TARGETS netlist
        EXPORT hal
        LIBRARY DESTINATION ${LIBRARY_INSTALL_DIRECTORY}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        INCLUDES DESTINATION ${INCLUDE_INSTALL_DIRECTORY})
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_sanitizers(netlist)
endif()