cmake_minimum_required(VERSION 3.15)

# Include common CMake modules
include(CMakeDependentOption)
include(CMakeToolsHelpers OPTIONAL)
include(CMakePrintHelpers)

# Detect Build Type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()
set(AVAILABLE_BUILD_TYPES "Release" "MinSizeRel" "RelWithDebInfo" "Debug")
if(CMAKE_BUILD_TYPE IN_LIST AVAILABLE_BUILD_TYPES)
    message(VERBOSE "Selected build type: ${CMAKE_BUILD_TYPE}")
else()
    message(FATAL_ERROR "Unsupported build type: ${CMAKE_BUILD_TYPE}, supported are: Debug, MinSizeRel, RelWithDebInfo, Release")
endif()

# Configure CCache if available
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_FOUND})
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE_FOUND})
    message(STATUS "ccache found!")
endif(CCACHE_FOUND)

# Create LINUX variable if needed ToDo
if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

#Add path to CMake script files and include Helper
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/" "${CMAKE_SOURCE_DIR}/deps/sanitizers-cmake/cmake/")

# Check for valid Changelog file ToDo
set(DEB_CHANGELOG_REQUIRED ON)
set(DEB_CHANGELOG "${CMAKE_CURRENT_SOURCE_DIR}/Changelog")
unset(CHANGELOG_LAST_VERSION)
unset(CHANGELOG_LAST_MESSAGE)
include(DebChangelog)

#Set the project name
project(hal)

# Include Helper Modules
include(HalUtils) # Version helper
include(DetectDistro) # Detect distribution helper
include(Subdirs) # Get a lost of all subdirs in specific directory
include(find_packages)

# Common Project Information
set(PROJECT_VENDOR "hal")
set(PROJECT_WEBSITE "https://github.com/emsec/hal")
set(PROJECT_MAINTAINER "Sebastian Wallat <sebastian.wallat@rub.de>")
set(PROJECT_DESCRIPTION_SUMMARY "Hardware Reverse engineering framework")
set(PROJECT_DESCRIPTION "hal - Hardware Analyzer")
set(CHANGELOG_MESSAGE ${CHANGELOG_LAST_MESSAGE})
set(PROJECT_PPA "ppa:sebastian-wallat/hal")
set(PROJECT_PPA_USER "sebastian-wallat")

# Use C11 and C++17 as minimum standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED on)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

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

# Detect Version Information
if((NOT ${HAL_VERSION_MAJOR}) AND ((NOT ${HAL_VERSION_MINOR}) AND (NOT ${HAL_VERSION_PATCH})))
    hal_get_version()
else()
    set(HAL_VERSION_RETURN ${HAL_VERSION_MAJOR}.${HAL_VERSION_MINOR}.${HAL_VERSION_PATCH})
endif()
unset(HAL_VERSION_MAJOR_SELF CACHE)
message(STATUS "HAL_VERSION: ${HAL_VERSION_RETURN}")

# Set Build Timestamp (used in configured file)
string(TIMESTAMP BUILD_TIMESTAMP UTC)

# Setup Directories, e.g. CMAKE_LIBRARY_OUTPUT_DIRECTOR, INCLUDE_INSTALL_DIRECTORY, ...
setup_output_directories()

# For all ExternalProject_ADD handle configure build and test as different stages
set_property(DIRECTORY PROPERTY EP_STEP_TARGETS configure build test)

################################
#####   Find depending packages
################################

# PkgConfig
find_package(PkgConfig REQUIRED)
if(PkgConfig_FOUND)
    message(STATUS "PKG_CONFIG_EXECUTABLE: ${PKG_CONFIG_EXECUTABLE}")
elseif(NOT PkgConfig_FOUND)
    set(missing_package "TRUE")
    if(LINUX)
        message(STATUS "Please install PkgConfig (https://linux.die.net/man/1/pkg-config)")
    endif(LINUX)
    if(APPLE AND CMAKE_HOST_APPLE)
        message(STATUS "To install pkgconfig on MacOS using homebrew run following command:")
        message(STATUS "    brew install pkgconfig")
    endif(APPLE AND CMAKE_HOST_APPLE)
endif(PkgConfig_FOUND)

find_package(Sanitizers)

# See cmake/find_packages.cmake
find_openmp()

find_package(Filesystem REQUIRED Final Experimental)

set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.58.0 COMPONENTS system REQUIRED)
if(Boost_FOUND)
    message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
    message(STATUS "Boost_VERSION: ${Boost_VERSION}")

    include_directories(${Boost_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS})
elseif(NOT Boost_FOUND)
    set(missing_package "TRUE")
    if(APPLE AND CMAKE_HOST_APPLE)
        message(STATUS "To install boost on MacOS using homebrew run following command:")
        message(STATUS "    brew install boost --c++11")
    endif(APPLE AND CMAKE_HOST_APPLE)
endif(Boost_FOUND)

find_package(RapidJSON REQUIRED)
message(STATUS "Found rapidjson ${RAPIDJSON_INCLUDEDIR}")
include_directories(${RAPIDJSON_INCLUDEDIR})

# Need Version 2.2.4 Not available in ubuntu bionic
find_package(pybind11 2.4.3 CONFIG)
if(${pybind11_FOUND})
    message(STATUS "Found pybind11 v${pybind11_VERSION}: ${pybind11_INCLUDE_DIRS}")
    message(STATUS "Found pybind11 >= 2.4.3")
    include_directories(pybind11_INCLIDE_DIRS)
else()
    message(STATUS "pybind11 >= 2.4.3 not found")
    add_subdirectory(deps/pybind11)
endif()

find_package(spdlog 1.4.2 EXACT CONFIG)
message(STATUS "Found spdlog v${spdlog_VERSION}")
if(${spdlog_FOUND})
    message(STATUS "Found spdlog >= 1.4.2")
else()
    message(STATUS "spdlog >= 1.4.2 not found")
    include_directories(${include_directories} "${CMAKE_CURRENT_SOURCE_DIR}/deps/spdlog-1.4.2/include")
    set(spdlog_VERSION 1.4.2)
endif()

################################
#####   Manage Compiler Flags
################################

#Set UNIX (macOS or Linux) compiler flags (clang or gcc). Set optimization level for debug or release.
if(UNIX)
    message(STATUS "Checking supported compiler flags...")

    include(CompilerFlagsCheck)
    enable_c_compiler_flag_if_supported("-Wall" "")
    enable_c_compiler_flag_if_supported("-Wextra" "")
    enable_c_compiler_flag_if_supported("-pedantic" "")
    enable_c_compiler_flag_if_supported("-Wshadow" "")

    enable_c_compiler_flag_if_supported("-g" "_DEBUG")

    enable_cxx_compiler_flag_if_supported("-Wall" "")
    enable_cxx_compiler_flag_if_supported("-Wextra" "")
    enable_cxx_compiler_flag_if_supported("-pedantic" "")
    enable_cxx_compiler_flag_if_supported("-Wshadow" "")
    enable_cxx_compiler_flag_if_supported("-Wno-undef" "")
    enable_cxx_compiler_flag_if_supported("-fcolor-diagnostics" "")
    enable_cxx_compiler_flag_if_supported("-fdiagnostics-color=always" "")
    enable_cxx_compiler_flag_if_supported("-fsized-deallocation" "")

    enable_cxx_compiler_flag_if_supported("-O0" "_DEBUG")
    enable_cxx_compiler_flag_if_supported("-g" "_DEBUG")

    enable_cxx_compiler_flag_if_supported("-O3" "_RELEASE")
    enable_cxx_compiler_flag_if_supported("-DNDEBUG" "_RELEASE")
    #enable_cxx_compiler_flag_if_supported("-flto" "_RELEASE")

    enable_cxx_compiler_flag_if_supported("-Os" "_MINSIZEREL")
    enable_cxx_compiler_flag_if_supported("-DNDEBUG" "_MINSIZEREL")
    #enable_cxx_compiler_flag_if_supported("-flto" "_MINSIZEREL")

    enable_cxx_compiler_flag_if_supported("-O2" "_RELWITHDEBINFO")
    enable_cxx_compiler_flag_if_supported("-g" "_RELWITHDEBINFO")
    #enable_cxx_compiler_flag_if_supported("-flto" "_RELWITHDEBINFO")

    if(USE_LIBCXX)
        enable_cxx_compiler_flag_if_supported("-stdlib=libc++" "")
        enable_c_compiler_flag_if_supported("-stdlib=libc++" "")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
    endif()

    message(VERBOSE "C flags: ${CMAKE_C_FLAGS${build_type}}")
    message(VERBOSE "CXX flags: ${CMAKE_CXX_FLAGS${build_type}}")
    message(VERBOSE " ")
endif(UNIX)

# Enable Debug Symbols for Libcpp on macOS
if(${CMAKE_BUILD_TYPE} EQUAL "Debug" AND APPLE AND CMAKE_HOST_APPLE)
    add_definitions(-D_LIBCPP_DEBUG)
endif(${CMAKE_BUILD_TYPE} EQUAL "Debug" AND APPLE AND CMAKE_HOST_APPLE)

#set names for major libraries
set(NETLIST_LIB hal_netlist)
set(CORE_LIB hal_core)
set(GRAPH_ALGORITHM_LIB graph_algorithm)
set(QUINE_MCCLUSKY_LIB quine_mccluskey)

#define default linker target
set(LINK_LIBS ${CMAKE_DL_LIBS}
    ${NETLIST_LIB}
    ${CORE_LIB}
    )

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
        set(COVERAGE_EXCLUDES '/usr/*' '${CMAKE_SOURCE_DIR}/tests/lib/googletest/*' '${CMAKE_SOURCE_DIR}/plugins/*' '${CMAKE_SOURCE_DIR}/deps/*' '${CMAKE_BINARY_DIR}/*' '${CMAKE_CURRENT_BINARY_DIR}/*' '${CMAKE_SOURCE_DIR}/include/gui/*' '${CMAKE_SOURCE_DIR}/src/gui/*')
        message(STATUS "COVERAGE_EXCLUDES: ${COVERAGE_EXCLUDES}")
        setup_target_for_coverage(NAME ${PROJECT_NAME}_coverage EXECUTABLE ctest DEPENDENCY runTest-log runTest-program_arguments runTest-program_options)
    endif()
endif()

##################################
#####   Include Code Directories
##################################
add_subdirectory("src")
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

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION ${INCLUDE_INSTALL_DIRECTORY})
install(DIRECTORY ${CMAKE_BINARY_DIR}/share/hal/gate_libraries DESTINATION ${SHARE_INSTALL_DIRECTORY} PATTERN "*.json")
configure_file("defaults/guidefaults.ini" ${CMAKE_BINARY_DIR}/share/hal/defaults/guidefaults.ini @ONLY)
install(DIRECTORY ${CMAKE_BINARY_DIR}/share/hal/defaults DESTINATION ${SHARE_INSTALL_DIRECTORY} PATTERN "*.ini")
install(FILES ${CMAKE_BINARY_DIR}/hal.pc DESTINATION "${PKGCONFIG_INSTALL_DIRECTORY}")
message(STATUS "PKGCONFIG_INSTALL_DIRECTORY ${PKGCONFIG_INSTALL_DIRECTORY}")

# Configure LDCONFIG for HAL plugins
if(LINUX)
    configure_file(
            "installer/deb/hal.conf.in"
            "${CMAKE_BINARY_DIR}/hal.conf"
            @ONLY)
    install(FILES "${CMAKE_BINARY_DIR}/hal.conf" DESTINATION "${SHARE_INSTALL_DIRECTORY}/ld_conf/")
endif()

# on make/ninja install run configure and run ldconfig
if(ENABLE_INSTALL_LDCONFIG AND LINUX)
        configure_file(
                "cmake/post_install.cmake.in"
                "${CMAKE_BINARY_DIR}/post_install.cmake"
                @ONLY)
        install(SCRIPT "${CMAKE_BINARY_DIR}/post_install.cmake")
endif()

# add uninstall target
configure_file(
        "cmake/uninstall.cmake.in"
        "${CMAKE_BINARY_DIR}/uninstall.cmake"
        @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_BINARY_DIR}/uninstall.cmake")

################################
#####   Packaging
################################

if(UPLOAD_PPA)
    set(PACKAGE_DEB ON)
    set(ENABLE_PPA ON)
endif()

set(CPACK_GENERATORS)
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
    add_subdirectory(installer)
endif(CPACK_GENERATORS)

################################
#####   Documentation
################################
if(BUILD_DOCUMENTATION)
    find_package(Doxygen REQUIRED)

    set(doxyfile "documentation/Doxyfile")

    configure_file(${doxyfile} ${doxyfile} @ONLY)

    add_custom_target(doc
                      COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                      COMMENT "Generating API documentation with Doxygen"
                      VERBATIM)

    find_package(Sphinx REQUIRED)
    if(NOT DEFINED SPHINX_THEME)
        set(SPHINX_THEME default)
    endif()

    if(NOT DEFINED SPHINX_THEME_DIR)
        set(SPHINX_THEME_DIR)
    endif()

    # configured documentation tools and intermediate build results
    set(BINARY_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/python-doc/_build")

    # Sphinx cache with pickled ReST documents
    set(SPHINX_CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/python-doc/_doctrees")

    # HTML output directory
    set(SPHINX_HTML_DIR "${CMAKE_CURRENT_BINARY_DIR}/python-doc/html")

    configure_file(
            "documentation/sphinx_doc/conf.py.in"
            "${BINARY_BUILD_DIR}/conf.py"
            @ONLY)

    file(GLOB_RECURSE SPHINX_DOC_FILES ${CMAKE_SOURCE_DIR}/documentation/sphinx_doc/*.rst)
    message(STATUS "SPHINX_DOC_FILES: ${SPHINX_DOC_FILES}")

    add_custom_target(hal_py_docs ALL
                      ${SPHINX_EXECUTABLE}
                      -q -b html
                      -c "${BINARY_BUILD_DIR}"
                      -d "${SPHINX_CACHE_DIR}"
                      "${CMAKE_CURRENT_SOURCE_DIR}/documentation/sphinx_doc/"
                      "${SPHINX_HTML_DIR}"
                      COMMENT "Building HTML documentation with Sphinx" VERBATIM
                      DEPENDS hal_py graph_algorithm ${SPHINX_DOC_FILES})

endif()