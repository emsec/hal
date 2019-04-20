#
# Find yaml-cpp
#
# This module defines the following variables:
# - YAMLCPP_INCLUDE_DIRS
# - YAMLCPP_LIBRARIES
# - YAMLCPP_FOUND
#
# The following variables can be set as arguments for the module.
# - YAMLCPP_ROOT_DIR : Root library directory of yaml-cpp
#

find_package(PkgConfig REQUIRED)

if (WIN32)
    # Find include files
    find_path(
            YAMLCPP_INCLUDE_DIR
            NAMES yaml-cpp/yaml.h
            PATHS
            $ENV{PROGRAMFILES}/include
            ${YAMLCPP_ROOT_DIR}/include
            DOC "The directory where yaml-cpp/yaml.h resides")

    # Find library files
    find_library(
            YAMLCPP_LIBRARY_RELEASE
            NAMES libyaml-cppmd
            PATHS
            $ENV{PROGRAMFILES}/lib
            ${YAMLCPP_ROOT_DIR}/lib)

    find_library(
            YAMLCPP_LIBRARY_DEBUG
            NAMES libyaml-cppmdd
            PATHS
            $ENV{PROGRAMFILES}/lib
            ${YAMLCPP_ROOT_DIR}/lib)
else()
pkg_check_modules(YAMLCPP REQUIRED yaml-cpp)
find_path(YAMLCPP_INCLUDE_DIRECTORY
          NAMES yaml-cpp/yaml.h
          PATHS ${YAMLCPP_INCLUDE_DIRS} /usr/include/
          )
find_library(YAMLCPP_LIBRARY
             NAMES yaml-cpp
             PATHS ${YAMLCPP_LIBRARY_DIRS})

endif()

include(FindPackageHandleStandardArgs)
if (WIN32)
    # Handle REQUIRD argument, define *_FOUND variable
    find_package_handle_standard_args(YamlCpp DEFAULT_MSG YAMLCPP_INCLUDE_DIRS YAMLCPP_LIBRARY_RELEASE YAMLCPP_LIBRARY_DEBUG)

    # Define YAMLCPP_LIBRARIES and YAMLCPP_INCLUDE_DIRS
    if (YAMLCPP_FOUND)
        set(YAMLCPP_LIBRARIES_RELEASE ${YAMLCPP_LIBRARY_RELEASE})
        set(YAMLCPP_LIBRARIES_DEBUG ${YAMLCPP_LIBRARY_DEBUG})
        set(YAMLCPP_LIBRARIES debug ${YAMLCPP_LIBRARIES_DEBUG} optimized ${YAMLCPP_LIBRARY_RELEASE})
        set(YAMLCPP_INCLUDE_DIRS ${YAMLCPP_INCLUDE_DIR})
    endif()

    # Hide some variables
    mark_as_advanced(YAMLCPP_INCLUDE_DIR YAMLCPP_LIBRARY_RELEASE YAMLCPP_LIBRARY_DEBUG)
else()
    find_package_handle_standard_args(YamlCpp
                                      FOUND_VAR YAMLCPP_FOUND
                                      REQUIRED_VARS YAMLCPP_LIBRARY YAMLCPP_INCLUDE_DIRECTORY
                                      )

    if (YamlCpp_FOUND)
        set(YAMLCPP_INCLUDE_DIRS ${YAMLCPP_INCLUDE_DIRECTORY})
        set(YAMLCPP_LIBRARIES ${YAMLCPP_LIBRARY})
    endif ()
    mark_as_advanced(YAMLCPP_INCLUDE_DIRECTORY YAMLCPP_LIBRARY)
endif()
