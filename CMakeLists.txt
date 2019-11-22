cmake_minimum_required(VERSION 3.15)

message(STATUS "cmake version: ${CMAKE_VERSION}")

include(CMakeDependentOption)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "selected build type: ${CMAKE_BUILD_TYPE}")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "selected build type: ${CMAKE_BUILD_TYPE}")
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    message(STATUS "selected build type: ${CMAKE_BUILD_TYPE}")
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    message(STATUS "selected build type: ${CMAKE_BUILD_TYPE}")
else()
    message(FATAL_ERROR "unsupported build type: ${CMAKE_BUILD_TYPE}, supported are: Debug, MinSizeRel, RelWithDebInfo, Release")
endif()

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

#Change Compiler on macOS to one that supports OpenMP
if(APPLE AND CMAKE_HOST_APPLE)
    #Needs to be set before project(hal)!
    message(STATUS "You need to install a compiler with OpenMP support:")
    message(STATUS "You need to install a compiler with OpenMP support:")
    message(STATUS "    brew install llvm")
    message(STATUS "    Set Environment Variable:")
    message(STATUS "    LDFLAGS=\"-L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib\"")
    message(STATUS "    CPPFLAGS=\"-I/usr/local/opt/llvm/include\"")
endif(APPLE AND CMAKE_HOST_APPLE)

# Configure CCache if available
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_FOUND})
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE_FOUND})
    message(STATUS "ccache found!")
endif(CCACHE_FOUND)

#Add path to CMake script files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/" "${CMAKE_SOURCE_DIR}/deps/sanitizers-cmake/cmake/")
include(CMakeToolsHelpers OPTIONAL)
set(DEB_CHANGELOG_REQUIRED ON)
set(DEB_CHANGELOG "${CMAKE_CURRENT_SOURCE_DIR}/Changelog")
unset(CHANGELOG_LAST_VERSION)
unset(CHANGELOG_LAST_MESSAGE)
include(DebChangelog)

#Set the project name
project(hal)

set(PROJECT_VENDOR "hal")
set(PROJECT_WEBSITE "https://github.com/emsec/hal")
set(PROJECT_MAINTAINER "Sebastian Wallat <sebastian.wallat@rub.de>")
set(PROJECT_DESCRIPTION_SUMMARY "Hardware Reverse engineering framework")
set(PROJECT_DESCRIPTION "hal - Hardware Analyzer")
set(CHANGELOG_MESSAGE ${CHANGELOG_LAST_MESSAGE})
set(PROJECT_PPA "ppa:sebastian-wallat/hal")
set(PROJECT_PPA_USER "sebastian-wallat")

#Use C11 and C++17 as minimum standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED on)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

option(USE_VERSION_FROM_OPTION "USE_VERSION_FROM_OPTION" FALSE)
include(HalUtils)
include(DetectDistro)
if(NOT ${USE_VERSION_FROM_OPTION})
    hal_get_version()
else()
    set(HAL_VERSION_RETURN ${HAL_VERSION_MAJOR}.${HAL_VERSION_MINOR}.${HAL_VERSION_PATCH})
endif()
unset(HAL_VERSION_MAJOR_SELF CACHE)

string(TIMESTAMP BUILD_TIMESTAMP UTC)
message(STATUS "HAL_VERSION: ${HAL_VERSION_RETURN}")

# declare project output binary path
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
foreach(i IN ITEMS "RUNTIME" "ARCHIVE" "LIBRARY")
    #    if(LINUX)
    set(appendix "/lib")
    if(i STREQUAL "RUNTIME")
        set(appendix "/bin")
    endif(i STREQUAL "RUNTIME")
    #    elseif(APPLE)
    #        set(appendix "/bin")
    #    endif(LINUX)
    if(NOT CMAKE_${i}_OUTPUT_DIRECTORY)
        set(CMAKE_${i}_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}${appendix}")
    elseif()
        set(CMAKE_${i}_OUTPUT_DIRECTORY "${CMAKE_${i}_OUTPUT_DIRECTORY}${appendix}")
    endif()
endforeach()

foreach(i IN ITEMS "_DEBUG" "_RELEASE" "_MINSIZEREL" "_RELWITHDEBINFO")
    foreach(j IN ITEMS "RUNTIME" "ARCHIVE" "LIBRARY")
        if(NOT CMAKE_${j}_OUTPUT_DIRECTORY${i})
            set(CMAKE_${j}_OUTPUT_DIRECTORY${i} ${CMAKE_${j}_OUTPUT_DIRECTORY})
        endif()
    endforeach()
endforeach()

#Install Directive target directiories
message(STATUS "")
include(GNUInstallDirs)
set(INCLUDE_INSTALL_DIRECTORY ${CMAKE_INSTALL_INCLUDEDIR}/hal)
message(STATUS "INCLUDE_INSTALL_DIRECTORY: ${INCLUDE_INSTALL_DIRECTORY}")

set(INCLUDE_INSTALL_DIRECTORY_FULL ${CMAKE_INSTALL_FULL_INCLUDEDIR}/hal)
message(STATUS "INCLUDE_INSTALL_DIRECTORY_FULL: ${INCLUDE_INSTALL_DIRECTORY_FULL}")

set(SHARE_INSTALL_DIRECTORY ${CMAKE_INSTALL_DATADIR}/hal)
message(STATUS "SHARE_INSTALL_DIRECTORY: ${SHARE_INSTALL_DIRECTORY}")

set(SHARE_INSTALL_DIRECTORY_FULL ${CMAKE_INSTALL_FULL_DATADIR}/hal)
message(STATUS "SHARE_INSTALL_DIRECTORY_FULL: ${SHARE_INSTALL_DIRECTORY_FULL}")

set(PKGCONFIG_INSTALL_DIRECTORY ${CMAKE_INSTALL_DATADIR}/pkgconfig)
message(STATUS "PKGCONFIG_INSTALL_DIRECTORY: ${PKGCONFIG_INSTALL_DIRECTORY}")

set(BINARY_INSTALL_DIRECTORY ${CMAKE_INSTALL_BINDIR})
message(STATUS "BINARY_INSTALL_DIRECTORY: ${BINARY_INSTALL_DIRECTORY}")

set(BINARY_INSTALL_DIRECTORY_FULL ${CMAKE_INSTALL_FULL_BINDIR})
message(STATUS "BINARY_INSTALL_DIRECTORY_FULL: ${BINARY_INSTALL_DIRECTORY_FULL}")

set(LIBRARY_INSTALL_DIRECTORY ${CMAKE_INSTALL_LIBDIR})
message(STATUS "LIBRARY_INSTALL_DIRECTORY: ${LIBRARY_INSTALL_DIRECTORY}")

set(LIBRARY_INSTALL_DIRECTORY_FULL ${CMAKE_INSTALL_FULL_LIBDIR})
message(STATUS "LIBRARY_INSTALL_DIRECTORY_FULL: ${LIBRARY_INSTALL_DIRECTORY_FULL}")

set(PLUGIN_INCLUDE_INSTALL_DIRECTORY ${INCLUDE_INSTALL_DIRECTORY}/hal_plugins)
message(STATUS "PLUGIN_INCLUDE_INSTALL_DIRECTORY: ${PLUGIN_INCLUDE_INSTALL_DIRECTORY}")
set(PLUGIN_BINARY_INSTALL_DIRECTORY ${BINARY_INSTALL_DIRECTORY})
message(STATUS "PLUGIN_BINARY_INSTALL_DIRECTORY: ${PLUGIN_BINARY_INSTALL_DIRECTORY}")
set(PLUGIN_LIBRARY_INSTALL_DIRECTORY ${LIBRARY_INSTALL_DIRECTORY}/hal_plugins)
message(STATUS "PLUGIN_LIBRARY_INSTALL_DIRECTORY: ${PLUGIN_LIBRARY_INSTALL_DIRECTORY}")
message(STATUS "")

#For all ExternalProject_ADD handle configure build and test as different stages
set_property(DIRECTORY PROPERTY EP_STEP_TARGETS configure build test)

#Enable test collection
enable_testing()
include(CTest)

set(Missing_package "FALSE")

#Search or build project dependencies and if necessary setup parameters
find_package(PkgConfig REQUIRED)
if(PkgConfig_FOUND)
    message(STATUS "PKG_CONFIG_EXECUTABLE: ${PKG_CONFIG_EXECUTABLE}")
elseif(NOT PkgConfig_FOUND)
    set(Missing_package "TRUE")
    if(LINUX)
        message(STATUS "Please install PkgConfig (https://linux.die.net/man/1/pkg-config)")
    endif(LINUX)
    if(APPLE AND CMAKE_HOST_APPLE)
        message(STATUS "To install pkgconfig on MacOS using homebrew run following command:")
        message(STATUS "    brew install pkgconfig")
    endif(APPLE AND CMAKE_HOST_APPLE)
endif(PkgConfig_FOUND)

include(Subdirs)

find_package(Sanitizers)

## Find OpenMP
if(APPLE AND CMAKE_HOST_APPLE)
    find_package(OpenMP)

    if(OPENMP_FOUND OR OPENMP_CXX_FOUND)
        message(STATUS "Found li
        mp without any special flags")
    endif()

    # If OpenMP wasn't found, try if we can find it in the default Macports location
    if((NOT OPENMP_FOUND) AND (NOT OPENMP_CXX_FOUND) AND EXISTS "/opt/local/lib/libomp/libomp.dylib") # older cmake uses OPENMP_FOUND, newer cmake also sets OPENMP_CXX_FOUND, homebrew installations seem only to get the latter set.
        set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I/opt/local/include/libomp/")
        set(OpenMP_CXX_LIB_NAMES omp)
        set(OpenMP_omp_LIBRARY /opt/local/lib/libomp/libomp.dylib)

        find_package(OpenMP)
        if(OPENMP_FOUND OR OPENMP_CXX_FOUND)
            message(STATUS "Found libomp in macports default location.")
        else()
            message(FATAL_ERROR "Didn't find libomp. Tried macports default location but also didn't find it.")
        endif()
    endif()

    # If OpenMP wasn't found, try if we can find it in the default Homebrew location
    if((NOT OPENMP_FOUND) AND (NOT OPENMP_CXX_FOUND) AND EXISTS "/usr/local/opt/libomp/lib/libomp.dylib")
        set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I/usr/local/opt/libomp/include")
        set(OpenMP_CXX_LIB_NAMES omp)
        set(OpenMP_omp_LIBRARY /usr/local/opt/libomp/lib/libomp.dylib)

        find_package(OpenMP)
        if(OPENMP_FOUND OR OPENMP_CXX_FOUND)
            message(STATUS "Found libomp in homebrew default location.")
        else()
            message(FATAL_ERROR "Didn't find libomp. Tried homebrew default location but also didn't find it.")
        endif()
    endif()

    set(Additional_OpenMP_Libraries_Workaround "")

    # Workaround because older cmake on apple doesn't support FindOpenMP
    if((NOT OPENMP_FOUND) AND (NOT OPENMP_CXX_FOUND))
        if((APPLE AND ((CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang") OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")))
            AND ((CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "7.0") AND (CMAKE_VERSION VERSION_LESS "3.12.0")))
            message(STATUS "Applying workaround for OSX OpenMP with old cmake that doesn't have FindOpenMP")
            set(OpenMP_CXX_FLAGS "-Xclang -fopenmp")
            set(Additional_OpenMP_Libraries_Workaround "-lomp")
        else()
            message(FATAL_ERROR "Did not find OpenMP. Build with -DDISABLE_OPENMP=ON if you want to allow this and are willing to take the performance hit.")
        endif()
    endif()
else()
    find_package(OpenMP REQUIRED)
    if(OpenMP_FOUND)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
    endif()
endif()

find_package(IGraph REQUIRED)
if(HAVE_IGRAPH)
    message(STATUS "Found following IGRAPH Libraries")
    message(STATUS "    IGRAPH_INCLUDE_DIRS: ${IGRAPH_INCLUDES}")
    message(STATUS "    IGRAPH_LIBRARIES: ${IGRAPH_LIBRARIES}")
    include_directories(${IGRAPH_INCLUDES})

    add_definitions(-DIGRAPH_VERSION_MAJOR_GUESS=${IGRAPH_VERSION_MAJOR_GUESS})
    add_definitions(-DIGRAPH_VERSION_MINOR_GUESS=${IGRAPH_VERSION_MINOR_GUESS})
    add_definitions(-DIGRAPH_VERSION_PATCH_GUESS=${IGRAPH_VERSION_PATCH_GUESS})
endif(HAVE_IGRAPH)

find_package(Filesystem REQUIRED Final Experimental)

set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.58.0 COMPONENTS system REQUIRED)
if(Boost_FOUND)
    message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
    message(STATUS "Boost_VERSION: ${Boost_VERSION}")

    include_directories(${Boost_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS})
elseif(NOT Boost_FOUND)
    set(Missing_package "TRUE")
    if(APPLE AND CMAKE_HOST_APPLE)
        message(STATUS "To install boost on MacOS using homebrew run following command:")
        message(STATUS "    brew install boost --c++11")
    endif(APPLE AND CMAKE_HOST_APPLE)
endif(Boost_FOUND)

#find_package(Threads REQUIRED)

find_package(RapidJSON REQUIRED)
message(STATUS "Found rapidjson ${RAPIDJSON_INCLUDEDIR}")
include_directories(${RAPIDJSON_INCLUDEDIR})

# Need Version 2.2.4 Not available in ubuntu bionic
find_package(pybind11 2.2.4 CONFIG)
if(${pybind11_FOUND})
    message(STATUS "Found pybind11 v${pybind11_VERSION}: ${pybind11_INCLUDE_DIRS}")
    message(STATUS "Found pybind11 >= 2.2.4")
    include_directories(pybind11_INCLIDE_DIRS)
else()
    message(STATUS "pybind11 >= 2.2.4 not found")
    add_subdirectory(deps/pybind11)
endif()

find_package(spdlog 1.3.1 EXACT CONFIG)
message(STATUS "Found spdlog v${spdlog_VERSION}")
if(${spdlog_FOUND})
    message(STATUS "Found spdlog >= 1.3.1")
else()
    message(STATUS "spdlog >= 1.3.1 not found")
    include_directories(${include_directories} "${CMAKE_CURRENT_SOURCE_DIR}/deps/spdlog-1.3.1/include")
    set(spdlog_VERSION 1.3.1)
endif()

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

    #set(CMAKE_C_FLAGS${build_type} "${CMAKE_C_FLAGS${build_type}} -Werror")
    #set(CMAKE_CXX_FLAGS${build_type} "${CMAKE_CXX_FLAGS${build_type}} -Werror")
    message(STATUS "C flags: ${CMAKE_C_FLAGS${build_type}}")
    message(STATUS "CXX flags: ${CMAKE_CXX_FLAGS${build_type}}")
    message(STATUS " ")
endif(UNIX)

if(${CMAKE_BUILD_TYPE} EQUAL "Debug" AND APPLE AND CMAKE_HOST_APPLE)
    add_definitions(-D_LIBCPP_DEBUG)
endif(${CMAKE_BUILD_TYPE} EQUAL "Debug" AND APPLE AND CMAKE_HOST_APPLE)

#set names for core libraries
set(NETLIST_LIB hal_netlist)
set(CORE_LIB hal_core)

option(WITH_GUI "WITH_GUI" ON)

#define default linker target
set(LINK_LIBS ${CMAKE_DL_LIBS}
    ${NETLIST_LIB}
    ${CORE_LIB}
    )

set(GRAPH_ALGORITHM_LIB graph_algorithm)
set(QUINE_MCCLUSKY_LIB quine_mccluskey)

option(BUILD_ALL_PLUGINS "BUILD_ALL_PLUGINS" OFF)
option(BUILD_TESTS "BUILD_TESTS" OFF)
option(BUILD_COVERAGE "BUILD_COVERAGE" OFF)

if(${BUILD_TESTS} AND ${BUILD_COVERAGE})
    include(CodeCoverage)
    append_coverage_compiler_flags()
    set(COVERAGE_EXCLUDES '/usr/*' '${CMAKE_SOURCE_DIR}/tests/lib/googletest/*' '${CMAKE_SOURCE_DIR}/plugins/*' '${CMAKE_SOURCE_DIR}/deps/*' '${CMAKE_BINARY_DIR}/*' '${CMAKE_CURRENT_BINARY_DIR}/*' '${CMAKE_SOURCE_DIR}/include/gui/*' '${CMAKE_SOURCE_DIR}/src/gui/*')
    message(STATUS "COVERAGE_EXCLUDES: ${COVERAGE_EXCLUDES}")
    setup_target_for_coverage(NAME ${PROJECT_NAME}_coverage EXECUTABLE ctest DEPENDENCY runTest-log runTest-program_arguments runTest-program_options)
endif(${BUILD_TESTS} AND ${BUILD_COVERAGE})

add_subdirectory("src")
add_subdirectory("plugins")
if(${BUILD_TESTS})
    add_subdirectory("tests")
endif(${BUILD_TESTS})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/hal.pc.in ${CMAKE_BINARY_DIR}/hal.pc @ONLY)

if(Missing_package)
    message(WARNING "In case of missing packages make sure that following dependencies are installed: pkgconfig, python3, qt5")
endif(Missing_package)

#install header
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION ${INCLUDE_INSTALL_DIRECTORY})
install(DIRECTORY ${CMAKE_BINARY_DIR}/share/hal/gate_libraries DESTINATION ${SHARE_INSTALL_DIRECTORY} PATTERN "*.json")
configure_file("defaults/guidefaults.ini" ${CMAKE_BINARY_DIR}/share/hal/defaults/guidefaults.ini @ONLY)
install(DIRECTORY ${CMAKE_BINARY_DIR}/share/hal/defaults DESTINATION ${SHARE_INSTALL_DIRECTORY} PATTERN "*.ini")
install(FILES ${CMAKE_BINARY_DIR}/hal.pc DESTINATION "${PKGCONFIG_INSTALL_DIRECTORY}")
message(STATUS "PKGCONFIG_INSTALL_DIRECTORY ${PKGCONFIG_INSTALL_DIRECTORY}")

if(LINUX)
    configure_file(
            "installer/deb/hal.conf.in"
            "${CMAKE_BINARY_DIR}/hal.conf"
            @ONLY)
    install(FILES "${CMAKE_BINARY_DIR}/hal.conf" DESTINATION "${SHARE_INSTALL_DIRECTORY}/ld_conf/")
endif()

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

#Documentation settings Add the End to solve conflict between FindSphinx and Python bindings

option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation")
if(BUILD_DOCUMENTATION)
    find_package(Doxygen REQUIRED)

    set(doxyfile "documentation/Doxyfile")

    configure_file(${doxyfile} ${doxyfile} @ONLY)

    add_custom_target(doc
                      COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                      COMMENT "Generating API documentation with Doxygen"
                      VERBATIM)

    #install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc)

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

# on make/ninja install run configure and run ldconfig
option(ENABLE_INSTALL_LDCONFIG "ENABLE_INSTALL_LDCONFIG" ON)

if(ENABLE_INSTALL_LDCONFIG)
    if(LINUX)
        configure_file(
                "cmake/post_install.cmake.in"
                "${CMAKE_BINARY_DIR}/post_install.cmake"
                @ONLY)
        install(SCRIPT "${CMAKE_BINARY_DIR}/post_install.cmake")
    endif(LINUX)
endif(ENABLE_INSTALL_LDCONFIG)

# add uninstall target
configure_file(
        "cmake/uninstall.cmake.in"
        "${CMAKE_BINARY_DIR}/uninstall.cmake"
        @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_BINARY_DIR}/uninstall.cmake")
