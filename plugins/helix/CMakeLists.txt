option(PL_HELIX "Build the Helix plugin" ON)

if(PL_HELIX OR BUILD_ALL_PLUGINS)

    # Dependencies
    find_package(absl REQUIRED)
    find_package(hiredis REQUIRED)
    find_package(Libevent REQUIRED)
    find_package(Protobuf CONFIG REQUIRED)   
    find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

    # Include-What-You-Use (optional)
    if(IWYU)
        set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "include-what-you-use")
    else()
        message(STATUS "include-what-you-use turned OFF")
    endif()

    # --- Protocol buffer library ---
    set(PROTO_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}")
    set(PROTO_OUT_DIR "${PROTO_SRC_DIR}/protobuf/protocol")
    file(MAKE_DIRECTORY "${PROTO_OUT_DIR}")

    # Define protocol library
    add_library(protocol SHARED "${PROTO_SRC_DIR}/helix.proto")
    target_include_directories(protocol PUBLIC "${PROTO_OUT_DIR}")
    target_link_libraries(protocol PUBLIC protobuf::libprotobuf)

    protobuf_generate(
        TARGET protocol
        IMPORT_DIRS "${Protobuf_INCLUDE_DIRS}"
        PROTOC_OUT_DIR "${PROTO_OUT_DIR}"
    )

    # --- Source Collection ---
    file(GLOB_RECURSE HELIX_INC "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h" "${PROTO_OUT_DIR}/*.h")
    file(GLOB_RECURSE HELIX_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc" "${PROTO_OUT_DIR}/*.cc")
    file(GLOB_RECURSE HELIX_PYTHON_SRC "${CMAKE_CURRENT_SOURCE_DIR}/python/*.cc")

    # --- Plugin Target ---
    hal_add_plugin(helix
        SHARED
        HEADER  ${HELIX_INC}
        SOURCES ${HELIX_SRC} ${HELIX_PYTHON_SRC}
        COMPILE_OPTIONS "-march=native"
    )

    # --- Includes ---
    target_include_directories(helix PRIVATE
        "${PROJECT_SOURCE_DIR}/plugins/gui/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/protobuf"
    )

    # --- Link Libraries ---
    target_link_libraries(helix PRIVATE
        Qt5::Core
        Qt5::Widgets
        pthread
        event
        event_pthreads
        hiredis
        protocol
        absl::log_internal_check_op
        absl::log
        absl::strings
        gui
    )

endif()
