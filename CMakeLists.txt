# CMakeLists.txt -- Build system for HAL - the hardware analyzer
#
# Copyright (c) 2019 Sebastian Wallat <sebastian.wallat@rub.e>
#
# All rights reserved. Use of this source code is governed by a
# MIT-style license that can be found in the LICENSE file.
cmake_minimum_required(VERSION 3.10)

# Include common CMake modules
include(CMakeDependentOption)
include(CMakeToolsHelpers OPTIONAL)
include(CMakePrintHelpers)
include(CMakePackageConfigHelpers)

# Detect Build Type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo" "Perf")
endif()
set(AVAILABLE_BUILD_TYPES "Release" "MinSizeRel" "RelWithDebInfo" "Debug" "Perf")
if(CMAKE_BUILD_TYPE IN_LIST AVAILABLE_BUILD_TYPES)
    message(STATUS "Selected build type: ${CMAKE_BUILD_TYPE}")
else()
    message(FATAL_ERROR "Unsupported build type: ${CMAKE_BUILD_TYPE}, supported are: Debug, Perf, MinSizeRel, RelWithDebInfo, Release")
endif()

# Configure CCache if available
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_FOUND})
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE_FOUND})
    message(STATUS "ccache found!")
endif(CCACHE_FOUND)

#Add path to CMake script files and include Helper
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/" "${CMAKE_SOURCE_DIR}/deps/sanitizers-cmake/cmake/")

# Define Options
option(HAL_VERSION_MAJOR "Pass major version via cmake options" "")
option(HAL_VERSION_MINOR "Pass minor version via cmake options" "")
option(HAL_VERSION_PATCH "Pass patch version via cmake options" "")
option(USE_LIBCXX "Force the use of LIBCXX for e.g. gcc" FALSE)
option(WITH_GUI "Enable GUI build" ON)
option(BUILD_ALL_PLUGINS "Build all available plugins" OFF)
option(BUILD_TESTS "Enable test builds" OFF)
option(BUILD_COVERAGE "Enable code coverage build" OFF)
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation")
option(ENABLE_INSTALL_LDCONFIG "When installing via make/ninja install, also install and run the LDCONFIG post_install scripts" ON)
option(UPLOAD_PPA "Upload package to ppa" OFF)
option(PACKAGE_DEB "Package DEB Package" OFF)
option(ENABLE_PPA "Prepare PPA" ON )
option(PACKAGE_TGZ "Package TGZ archive" OFF)
option(PACKAGE_ZIP "Package ZIP archive" OFF)
option(PACKAGE_RPM "Package RPM package" OFF)
option(PACKAGE_MACOS "Package for macOS" OFF)

# Include Helper Modules
include(hal_cmake_tools) # Version helper
include(hal_plugin) # Definition of hal_add_plugin

# Create LINUX variable if needed ToDo
if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
    detect_distro()
endif()

# Detect Version Information
set(hal_GENVERSION_PATH "${CMAKE_SOURCE_DIR}/tools")
if((NOT ${HAL_VERSION_MAJOR}) AND ((NOT ${HAL_VERSION_MINOR}) AND (NOT ${HAL_VERSION_PATCH})))
    hal_get_version()
else()
    set(HAL_VERSION_RETURN ${HAL_VERSION_MAJOR}.${HAL_VERSION_MINOR}.${HAL_VERSION_PATCH})
endif()
configure_file(${CMAKE_SOURCE_DIR}/CURRENT_VERSION.in ${CMAKE_SOURCE_DIR}/CURRENT_VERSION @ONLY)
unset(HAL_VERSION_MAJOR_SELF CACHE)
message(STATUS "HAL_VERSION: ${HAL_VERSION_RETURN}")

################################
#####   The project statement
################################

project(hal
        VERSION ${HAL_VERSION_MAJOR}.${HAL_VERSION_MINOR}.${HAL_VERSION_PATCH}
        DESCRIPTION "HAL - the hardware analyzer"
        #HOMEPAGE_URL https://github.com/emsec/hal # not supported in cmake 3.10
        LANGUAGES CXX C)

# Common Project Information
set(PROJECT_VENDOR "hal")
set(PROJECT_WEBSITE "https://github.com/emsec/hal")
set(PROJECT_MAINTAINER "Sebastian Wallat <sebastian.wallat@rub.de>")
set(PROJECT_DESCRIPTION_SUMMARY "Hardware Reverse engineering framework")
set(PROJECT_DESCRIPTION "hal - Hardware Analyzer")
# set(CHANGELOG_MESSAGE ${CHANGELOG_LAST_MESSAGE})
set(PROJECT_PPA "ppa:sebastian-wallat/hal")
set(PROJECT_PPA_USER "sebastian-wallat")

# Use C11 and C++17 as minimum standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED on)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

# Set Build Timestamp (used in configured file)
string(TIMESTAMP BUILD_TIMESTAMP UTC)

# Setup Directories, e.g. CMAKE_LIBRARY_OUTPUT_DIRECTOR, INCLUDE_INSTALL_DIRECTORY, ...
setup_output_directories()

# For all ExternalProject_ADD handle configure build and test as different stages
set_property(DIRECTORY PROPERTY EP_STEP_TARGETS configure build test)

#---Enable Folders in IDE like Visual Studio----------------------------------------------------
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#####################################################################
#####   Find depending packages (See cmake/detect_dependencies.cmake)
#####################################################################

include(detect_dependencies)

################################
#####   Manage Compiler Flags
################################

#Set UNIX (macOS or Linux) compiler flags (clang or gcc). Set optimization level for debug or release.
if(UNIX)
    message(STATUS "Checking supported compiler flags...")

    enable_c_compile_option_if_supported("-Wall" "" "PRIVATE")
    enable_c_compile_option_if_supported("-Wextra" "" "PRIVATE")
    enable_c_compile_option_if_supported("-pedantic" "" "PRIVATE")
    enable_c_compile_option_if_supported("-Wshadow" "" "PRIVATE")

    enable_c_compile_option_if_supported("-g" "DEBUG" "PUBLIC")

    enable_cxx_compile_option_if_supported("-Wall" "" "PRIVATE")
    enable_cxx_compile_option_if_supported("-Wextra" "" "PRIVATE")
    enable_cxx_compile_option_if_supported("-pedantic" "" "PRIVATE")
    enable_cxx_compile_option_if_supported("-Wshadow" "" "PRIVATE")
    enable_cxx_compile_option_if_supported("-Wno-undef" "" "PRIVATE")
    enable_cxx_compile_option_if_supported("-fcolor-diagnostics" "" "PRIVATE")
    enable_cxx_compile_option_if_supported("-fdiagnostics-color=always" "" "PRIVATE")
    enable_cxx_compile_option_if_supported("-fsized-deallocation" "" "PRIVATE")

    enable_cxx_compile_option_if_supported("-O0" "Debug" "PUBLIC")
    enable_cxx_compile_option_if_supported("-g" "Debug" "PUBLIC")

    enable_cxx_compile_option_if_supported("-O3" "Release" "PUBLIC")
    enable_cxx_compile_option_if_supported("-DNDEBUG" "Release" "PUBLIC")
    #enable_cxx_compile_option_if_supported("-flto" "RELEASE" "PUBLIC")

    enable_cxx_compile_option_if_supported("-Os" "MinSizeRel" "PUBLIC")
    enable_cxx_compile_option_if_supported("-DNDEBUG" "MinSizeRel" "PUBLIC")
    #enable_cxx_compile_option_if_supported("-flto" "MinSizeRel" "PUBLIC")

    enable_cxx_compile_option_if_supported("-O2" "RelWithDebInfo" "PUBLIC")
    enable_cxx_compile_option_if_supported("-g" "RelWithDebInfo" "PUBLIC")

    enable_cxx_compile_option_if_supported("-O1" "Perf" "PUBLIC")
    enable_cxx_compile_option_if_supported("-g" "Perf" "PUBLIC")
    enable_cxx_compile_option_if_supported("-pg" "Perf" "PUBLIC")
    enable_cxx_compile_option_if_supported("-fno-inline-functions" "Perf" "PUBLIC")
    enable_cxx_compile_option_if_supported("-fno-inline-functions-called-once" "Perf" "PUBLIC")
    enable_cxx_compile_option_if_supported("-fno-optimize-sibling-calls" "Perf" "PUBLIC")

    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -pthread")
    SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -pthread")

    if(USE_LIBCXX)
        enable_cxx_compile_option_if_supported("-stdlib=libc++" "" "PUBLIC")
        enable_c_compile_option_if_supported("-stdlib=libc++" "" "PUBLIC")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
    endif()

    message(VERBOSE "COMPILE_OPTIONS_PRIVATE: ${COMPILE_OPTIONS_PRIVATE}")
    message(VERBOSE "COMPILE_OPTIONS_PUBLIC: ${COMPILE_OPTIONS_PUBLIC}")
    message(VERBOSE "COMPILE_OPTIONS_INTERFACE: ${COMPILE_OPTIONS_INTERFACE}")
    message(VERBOSE " ")
endif(UNIX)

# Enable Debug Symbols for Libcpp on macOS
if(${CMAKE_BUILD_TYPE} EQUAL "Debug" AND APPLE AND CMAKE_HOST_APPLE)
    add_definitions(-D_LIBCPP_DEBUG)
endif(${CMAKE_BUILD_TYPE} EQUAL "Debug" AND APPLE AND CMAKE_HOST_APPLE)

####################################################################
#####   Setup main target to hold all build definitions and options
####################################################################


################################
#####   Enable Testing
################################

# Enable test collection
if(BUILD_TESTS)
    enable_testing()
    include(CTest)

    # Setuo Codse Coverage
    if(${BUILD_COVERAGE})
        include(CodeCoverage)
        append_coverage_compiler_flags()
        set(COVERAGE_EXCLUDES
            '/usr/*'
            '${CMAKE_SOURCE_DIR}/tests/lib/googletest/*'
            '${CMAKE_SOURCE_DIR}/plugins/*'
            '${CMAKE_SOURCE_DIR}/deps/*'
            '${CMAKE_BINARY_DIR}/*'
            '${CMAKE_CURRENT_BINARY_DIR}/*'
            '${CMAKE_SOURCE_DIR}/include/gui/*'
            '${CMAKE_SOURCE_DIR}/src/gui/*'
            )

        message(VERBOSE "COVERAGE_EXCLUDES: ${COVERAGE_EXCLUDES}")
        setup_target_for_coverage(NAME ${PROJECT_NAME}_coverage
                                  EXECUTABLE ctest
                                  DEPENDENCY runTest-*
                                  )
    endif()
endif()

##################################
#####   Include Code Directories
##################################
add_subdirectory("src")
add_subdirectory("app")
add_subdirectory("plugins")
if(${BUILD_TESTS})
    add_subdirectory("tests")
endif(${BUILD_TESTS})

####################################
#####   Configure Pkgconfig for HAL
####################################

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/hal.pc.in ${CMAKE_BINARY_DIR}/hal.pc @ONLY)

#########################################
#####   Install Header + Auxilary  files
#########################################

set(HAL_CMAKECONFIG_INSTALL_DIR "share/cmake/${PROJECT_NAME}" CACHE STRING "install path for halConfig.cmake")

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION ${INCLUDE_INSTALL_DIRECTORY})
install(DIRECTORY ${CMAKE_BINARY_DIR}/share/hal/gate_libraries DESTINATION ${SHARE_INSTALL_DIRECTORY} PATTERN "*.json")
install(DIRECTORY ${CMAKE_BINARY_DIR}/share/hal/gate_libraries DESTINATION ${SHARE_INSTALL_DIRECTORY} PATTERN "*.lib")
configure_file("defaults/guidefaults.ini" ${CMAKE_BINARY_DIR}/share/hal/defaults/guidefaults.ini @ONLY)
install(DIRECTORY ${CMAKE_BINARY_DIR}/share/hal/defaults DESTINATION ${SHARE_INSTALL_DIRECTORY} PATTERN "*.ini")
install(FILES ${CMAKE_BINARY_DIR}/hal.pc DESTINATION "${PKGCONFIG_INSTALL_DIRECTORY}")
message(VERBOSE "PKGCONFIG_INSTALL_DIRECTORY ${PKGCONFIG_INSTALL_DIRECTORY}")

configure_package_config_file(cmake/halConfig.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/halConfig.cmake"
                              INSTALL_DESTINATION ${HAL_CMAKECONFIG_INSTALL_DIR})

write_basic_package_version_file(
        halConfigVersion.cmake
        VERSION ${HAL_VERSION_RETURN}
        COMPATIBILITY AnyNewerVersion
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/halConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/halConfigVersion.cmake
        cmake/hal_plugin.cmake
        cmake/hal_cmake_tools.cmake
        cmake/FindFilesystem.cmake
        tools/genversion.py
        DESTINATION ${HAL_CMAKECONFIG_INSTALL_DIR})

install(FILES tools/genversion.py
        DESTINATION ${HAL_CMAKECONFIG_INSTALL_DIR}
        PERMISSIONS
            OWNER_READ
            OWNER_WRITE
            OWNER_EXECUTE
            GROUP_READ
            GROUP_EXECUTE
            WORLD_READ
            WORLD_EXECUTE)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/deps/pybind11 DESTINATION ${HAL_CMAKECONFIG_INSTALL_DIR})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/deps/spdlog-1.4.2 DESTINATION ${HAL_CMAKECONFIG_INSTALL_DIR})

if(NOT (CMAKE_VERSION VERSION_LESS 3.0))
    install(EXPORT hal
            FILE halTargets.cmake
            NAMESPACE hal::
            DESTINATION ${HAL_CMAKECONFIG_INSTALL_DIR}
            )
endif()


# Configure LDCONFIG for HAL plugins
if(LINUX)
    configure_file(
            "${CMAKE_SOURCE_DIR}/packaging/deb/hal.conf.in"
            "${CMAKE_BINARY_DIR}/hal.conf"
            @ONLY)
    install(FILES "${CMAKE_BINARY_DIR}/hal.conf" DESTINATION "${SHARE_INSTALL_DIRECTORY}/ld_conf/")
endif()

# on make/ninja install run configure and run ldconfig
if(ENABLE_INSTALL_LDCONFIG AND LINUX)
    configure_file(
            "${CMAKE_SOURCE_DIR}/cmake/post_install.cmake.in"
            "${CMAKE_BINARY_DIR}/post_install.cmake"
            @ONLY)
    install(SCRIPT "${CMAKE_BINARY_DIR}/post_install.cmake")
endif()

# add uninstall target
configure_file(
        "${CMAKE_SOURCE_DIR}/cmake/uninstall.cmake.in"
        "${CMAKE_BINARY_DIR}/uninstall.cmake"
        @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_BINARY_DIR}/uninstall.cmake")

################################
#####   Packaging
################################
add_subdirectory(packaging)

################################
#####   Documentation
################################
add_subdirectory(documentation)
