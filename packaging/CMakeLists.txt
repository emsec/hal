if (UPLOAD_PPA)
    set(PACKAGE_DEB ON)
    set(ENABLE_PPA ON)
endif()

set(CPACK_GENERATORS "")
if(PACKAGE_TGZ)
    list(APPEND CPACK_GENERATORS "TGZ")
endif()
if(PACKAGE_ZIP)
    list(APPEND CPACK_GENERATORS "ZIP")
endif()
if(PACKAGE_DEB)
    list(APPEND CPACK_GENERATORS "DEB")
endif()
if(PACKAGE_RPM)
    list(APPEND CPACK_GENERATORS "RPM")
endif()
if(PACKAGE_MACOS)
    list(APPEND CPACK_GENERATORS "productbuild")
    set(PACKAGE_APPENDIX "macOS_${ARCHITECTURE}")
endif()
message(STATUS "CPACK_GENERATOR: ${CPACK_GENERATOR}")

if(CPACK_GENERATORS)
    set(CPACK_GENERATOR "${CPACK_GENERATORS}")
    set(CPACK_SOURCE_IGNORE_FILES
        "/mxe/"
        "/\\\\.vscode/"
        "/\\\\.codelite/"
        "/\\\\.idea/"
        "/cmake-.*/"
        "\\\\.project$"
        "\\\\.workspace$"
        "\\\\.iml$"
        "\\\\.mk$"
        "\\\\.astylerc$"
        "\\\\.editorconfig$"
        "/Makefile$"
        )
    set(PROJECT_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
    message(STATUS "PROJECT_ARCH: ${PROJECT_ARCH}")

    set(CPACK_PACKAGE_NAME "hal-reverse")
    set(CPACK_PACKAGE_CONTACT "${PROJECT_MAINTAINER}")
    set(CPACK_PACKAGE_VERSION ${HAL_VERSION_MAJOR}.${HAL_VERSION_MINOR}.${HAL_VERSION_PATCH})
    set(CPACK_PACKAGE_VERSION_MAJOR ${HAL_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${HAL_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${HAL_VERSION_PATCH})
    set(CPACK_PACKAGE_VENDOR ${PROJECT_VENDOR})
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION_SUMMARY})
    set(CPACK_PACKAGE_DESCRIPTION ${PROJECT_DESCRIPTION})
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
    set(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_SOURCE_DIR}/README.md")
    set(CPACK_PACKAGE_FILE_NAME
        "${PROJECT_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_BUILD_TYPE}-${CMAKE_SYSTEM_NAME}-${PROJECT_ARCH}")
    set(CPACK_SET_DESTDIR false)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CPACK_STRIP_FILES ON)
    endif()

    if(PACKAGE_DEB)
        execute_process(
                COMMAND /usr/bin/dpkg --print-architecture
                OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
                RESULT_VARIABLE EXECUTE_RESULT
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
        )
        if(EXECUTE_RESULT)
            message(FATAL_ERROR "dpkg not found: No package generation.")
        endif()


        set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_SOURCE_DIR}/packaging/deb/postinst;${CMAKE_SOURCE_DIR}/packaging/deb/postrm")
        set(CPACK_DEBIAN_PACKAGE_DESCRIPTION ${PROJECT_DESCRIPTION})
        set(CPACK_DEBIAN_PACKAGE_HOMEPAGE ${PROJECT_WEBSITE})
        set(CPACK_DEBIAN_PACKAGE_SECTION libs)
        set(CPACK_DEBIAN_PACKAGE_PRIORITY optional)
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libstdc++6 (>= 6.3)" "libgomp1 (>= 6.3)" "libc6 (>= 2.24)" "libqt5widgets5 (>= 5.7)" "libqt5concurrent5 (>= 5.7)" "python3 (>= 3.6)" "python3-pybind11 (>= 2)" "libqt5svg5-dev (>= 5.9.5)" "python3-jedi (>= 0.11)")
        set(CPACK_DEBIAN_PACKAGE_BUILD_DEPENDS "pkg-config" "git" "devscripts" "dh-make" "pkgconf (>= 0.9)" "cmake (>= 3.7)" "g++ (>= 6.3)" "qt5-default (>= 5.7)" "libpython3-dev (>= 3.7)" "libqt5svg5-dev (>= 5.9.5)" "python3-sphinx (>= 1.6)" "python3-sphinx-rtd-theme (>= 0.2)" "pybind11-dev (>=2.0)" "python3-pybind11 (>=2.0)" "rapidjson-dev (>=1.1.0)" "libspdlog-dev (>= 0.16.3)" "libigraph0-dev (>= 0.7.1)")
        set(CPACK_DEBIAN_PACKAGE_CONFLICTS "hal-plugins (<= 0.0.22)")
        if(NOT PPA_DEBIAN_VERSION)
            set(PPA_DEBIAN_VERSION ppa1)
        endif()
        if(PROJECT_PPA)
            set(DPUT_HOST ${PROJECT_PPA})
        endif()
    endif(PACKAGE_DEB)

    if(PACKAGE_TGZ)
        set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
    endif(PACKAGE_TGZ)

    include(CPack)

    if(ENABLE_PPA)
        if(NOT PROJECT_PPA_DISTRIB_TARGET)
            set(PROJECT_PPA_DISTRIB_TARGET focal)
        endif()
        set(DPUT_CONFIG_IN ${CMAKE_CURRENT_SOURCE_DIR}/deb/dput.cf.in)
        include(UploadPPA)
    endif()

endif(CPACK_GENERATORS)


