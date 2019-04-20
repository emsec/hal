option(PL_QUINE_MCCLUSKEY "PL_QUINE_MCCLUSKEY" OFF)
if(PL_QUINE_MCCLUSKEY OR BUILD_ALL_PLUGINS)
    include_directories(${BUDDY_INCLUDE_DIR})
    include_directories(AFTER "${CMAKE_CURRENT_SOURCE_DIR}/include")

    set(QUINE_MCCLUSKEY_INC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/factory_quine_mccluskey.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/plugin_quine_mccluskey.h
        )

    set(QUINE_MCCLUSKEY_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/factory_quine_mccluskey.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin_quine_mccluskey.cpp
        )

    add_library(quine_mccluskey SHARED ${QUINE_MCCLUSKEY_SRC} ${QUINE_MCCLUSKEY_INC})
    set_target_properties(quine_mccluskey PROPERTIES PREFIX "")
    set_target_properties(quine_mccluskey PROPERTIES DEFINE_SYMBOL PLUGIN)
    target_link_libraries(quine_mccluskey ${LINK_LIBS})
    install(TARGETS quine_mccluskey LIBRARY DESTINATION ${PLUGIN_LIBRARY_INSTALL_DIRECTORY} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE INCLUDES DESTINATION ${PLUGIN_INCLUDE_INSTALL_DIRECTORY})
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${PLUGIN_INCLUDE_INSTALL_DIRECTORY}/quine_mccluskey/)
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(quine_mccluskey)
    endif()
endif()
