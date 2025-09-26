option(PL_CROSS_PROBING "PL_CROSS_PROBING" OFF)

if(PL_CROSS_PROBING OR BUILD_ALL_PLUGINS)

    find_package(Qt5 COMPONENTS Widgets Network REQUIRED)

    file(GLOB_RECURSE CROSS_PROBING_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE CROSS_PROBING_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

    qt5_wrap_cpp(MOC_INC ${CROSS_PROBING_INC})

    hal_add_plugin(cross_probing
        SHARED
        HEADER ${CROSS_PROBING_INC}
        SOURCES ${CROSS_PROBING_SRC} ${MOC_INC}
        LINK_LIBRARIES PUBLIC gui Qt5::Widgets Qt5::Network QuaZip::QuaZip
    )
endif()
