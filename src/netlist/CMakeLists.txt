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
add_library(hal::netlist ALIAS netlist)

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