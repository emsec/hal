cmake_minimum_required(VERSION 3.8)

message(STATUS "cmake version: ${CMAKE_VERSION}")

include(CMakeDependentOption)

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

#Change Compiler on macOS to one that supports OpenMP
if(APPLE AND CMAKE_HOST_APPLE)
    #Needs to be set before project(hal)!
    message(STATUS "You need to install a compiler with OpenMP support:")
    message(STATUS "    brew install llvm")
    message(STATUS "    Set Environment Variable:")
    message(STATUS "    LDFLAGS=\"-L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib\"")
    message(STATUS "    CPPFLAGS=\"-I/usr/local/opt/llvm/include\"")
    set(CMAKE_C_COMPILER /usr/local/opt/llvm/bin/clang)
    set(CMAKE_CXX_COMPILER /usr/local/opt/llvm/bin/clang++)
    set(ENV{LDFLAGS} "-L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib")
    set(ENV{CPPFLAGS} "-I/usr/local/opt/llvm/include")
endif(APPLE AND CMAKE_HOST_APPLE)

# Configure CCache if available
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_FOUND})
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE_FOUND})
    message(STATUS "ccache found!")
endif(CCACHE_FOUND)

#Set the project name
project(hal)

#Use C11 and C++17 as minimum standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED on)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

#Add path to CMake script files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/" "${CMAKE_SOURCE_DIR}/deps/sanitizers-cmake/cmake/")

message(STATUS "")

include(HalUtils)
hal_get_version()
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

set(PLUGIN_INCLUDE_INSTALL_DIRECTORY ${INCLUDE_INSTALL_DIRECTORY}/plugins)
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

#Included in Project
include_directories(${include_directories} "${CMAKE_BINARY_DIR}/spdlog-src/include")
message(STATUS "using local spdlog: ${CMAKE_BINARY_DIR}/spdlog-src/include")

include_directories(${include_directories} "${CMAKE_BINARY_DIR}/rapidjson-src/include")
message(STATUS "using local rapidjson: ${CMAKE_BINARY_DIR}/rapidjson-src/include")

include(BuildBuddy)
include(Subdirs)

find_package(Sanitizers)

find_package(OpenMP REQUIRED)
if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.58.0 COMPONENTS system filesystem graph REQUIRED)
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

find_package(Threads REQUIRED)

message(STATUS "")

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
    ${Boost_LIBRARIES}
    )

option(BUILD_ALL_PLUGINS "BUILD_ALL_PLUGINS" OFF)
option(BUILD_TESTS "BUILD_TESTS" ON)
option(BUILD_COVERAGE "BUILD_COVERAGE" OFF)

if(${BUILD_TESTS} AND ${BUILD_COVERAGE})
    include(CodeCoverage)
    append_coverage_compiler_flags()
    set(COVERAGE_EXCLUDES '/usr/*' '${CMAKE_SOURCE_DIR}/tests/lib/googletest/*' '${CMAKE_SOURCE_DIR}/plugins/*' '${CMAKE_SOURCE_DIR}/deps/*' '${CMAKE_BINARY_DIR}/*' '${CMAKE_CURRENT_BINARY_DIR}/*' '${CMAKE_SOURCE_DIR}/include/gui/*' '${CMAKE_SOURCE_DIR}/src/gui/*')
    message(STATUS "COVERAGE_EXCLUDES: ${COVERAGE_EXCLUDES}")
    setup_target_for_coverage(NAME ${PROJECT_NAME}_coverage EXECUTABLE ctest DEPENDENCY runTest-log runTest-program_arguments runTest-program_options)
endif(${BUILD_TESTS} AND ${BUILD_COVERAGE})

#Add source directories
add_subdirectory("deps")
message(STATUS "PYBIND11_INCLUDE_DIR: ${PYBIND11_INCLUDE_DIR}")
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
install(FILES ${CMAKE_BINARY_DIR}/hal.pc DESTINATION "${PKGCONFIG_INSTALL_DIRECTORY}")
message(STATUS "PKGCONFIG_INSTALL_DIRECTORY ${PKGCONFIG_INSTALL_DIRECTORY}")

if(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
    message(STATUS "CPack used for Packaging")
    include(InstallRequiredSystemLibraries)

    set(CPACK_SET_DESTDIR false)
    set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local/")
    set(GENERATE_DEB FALSE)
    include(DetectDistro)
    set(PACKAGE_APPENDIX "")
    if(LINUX)
        set(PACKAGE_APPENDIX "${LINUX_DISTRO}_${LINUX_DISTRO_VERSION_NAME}_${ARCHITECTURE}")
        if(LINUX_DISTRO STREQUAL "Ubuntu")
            set(GENERATE_DEB TRUE)
        endif(LINUX_DISTRO STREQUAL "Ubuntu")
        if(LINUX_DISTRO STREQUAL "Debian")
            set(GENERATE_DEB TRUE)
        endif(LINUX_DISTRO STREQUAL "Debian")
    elseif(APPLE)
        set(PACKAGE_APPENDIX "macOS_${ARCHITECTURE}")
    endif(LINUX)
    set(CPACK_GENERATOR "TGZ;STGZ;ZIP")
    if(${GENERATE_DEB})
        set(CPACK_GENERATOR "${CPACK_GENERATOR};DEB")
        install(FILES "resources/packaging/deb/hal.conf" DESTINATION "${SHARE_INSTALL_DIRECTORY}/ld_conf/")
        set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/")
    endif(${GENERATE_DEB})
    if(APPLE)
        set(CPACK_GENERATOR "${CPACK_GENERATOR};productbuild")
    endif(APPLE)
    message(STATUS "CPACK_GENERATOR: ${CPACK_GENERATOR}")

    set(CPACK_PACKAGE_NAME "hal-reverse")
    set(CPACK_PACKAGE_DESCRIPTION "hal - Hardware Analyzer")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Hardware Reverse engineering framework")
    set(CPACK_PACKAGE_VENDOR "hal")
    set(CPACK_PACKAGE_CONTACT "sebastian.wallat@rub.de")
    set(CPACK_PACKAGE_VERSION_MAJOR "${HAL_VERSION_MAJOR}")
    set(CPACK_PACKAGE_VERSION_MINOR "${HAL_VERSION_MINOR}")
    set(CPACK_PACKAGE_VERSION_PATCH "${HAL_VERSION_PATCH}")
    set(CPACK_PACKAGE_VERSION_TWEAK "${HAL_VERSION_TWEAK}")
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${HAL_VERSION_RETURN}_${PACKAGE_APPENDIX}")
    set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${HAL_VERSION_RETURN}_${PACKAGE_APPENDIX}")

    set(CPACK_DEBIAN_PACKAGE_DEPENDS " libboost-all-dev (>= 1.62), libstdc++6 (>= 6.3), libgomp1 (>= 6.3), libc6 (>= 2.24), libqt5widgets5 (>= 5.7), libqt5concurrent5 (>= 5.7), libpython3.6 (>= 3.6), python3-pybind11 (>= 2), libbdd-dev (>= 2.4), libqt5svg5-dev (>= 5.9.5), python3-jedi (>= 0.11)")
    set(DEBIAN_PACKAGE_BUILDS_DEPENDS " pkgconf (>= 0.9), libboost-all-dev (>= 1.62), cmake (>= 3.7), g++ (>= 6.3), qt5-default (>= 5.7), libpython3.7-dev (>= 3.7),  libqt5svg-dev (>= 5.9.5), python3-sphinx (>= 1.6), python3-sphinx-rtd-theme (>= 0.2)")
    set(CPACK_DEBIAN_PACKAGE_CONFLICTS "")
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_SOURCE_DIR}/resources/packaging/deb/postinst;${CMAKE_SOURCE_DIR}/resources/packaging/deb/postrm")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_SECTION "devel")

    # Following after all definitions!
    set(CPACK_COMPONENTS_ALL Libraries ApplicationData)
    include(CPack)

    # To use this:
    # make package
    # sudo dpkg -i hal-x.x.x.x.deb

    # This will result in the file:
    #/usr`

endif(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")

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

