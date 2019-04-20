##############################################################################
# @file  FindSphinx.cmake
# @brief Find Sphinx documentation build tools.
#
# @par Input variables:
# <table border="0">
#   <tr>
#     @tp @b Sphinx_DIR @endtp
#     <td>Installation directory of Sphinx tools. Can also be set as environment variable.</td>
#   </tr>
#   <tr>
#     @tp @b SPHINX_DIR @endtp
#     <td>Alternative environment variable for @c Sphinx_DIR.</td>
#   </tr>
#   <tr>
#     @tp @b Sphinx_FIND_COMPONENTS @endtp
#     <td>Sphinx build tools to look for, i.e., 'apidoc' and/or 'build'.</td>
#   </tr>
# </table>
#
# @par Output variables:
# <table border="0">
#   <tr>
#     @tp @b Sphinx_FOUND @endtp
#     <td>Whether all or only the requested Sphinx build tools were found.</td>
#   </tr>
#   <tr>
#     @tp @b SPHINX_FOUND @endtp
#     <td>Alias for @c Sphinx_FOUND.<td>
#   </tr>
#   <tr>
#     @tp @b SPHINX_EXECUTABLE @endtp
#     <td>Non-cached alias for @c Sphinx-build_EXECUTABLE.</td>
#   </tr>
#   <tr>
#     @tp @b Sphinx_PYTHON_EXECUTABLE @endtp
#     <td>Python executable used to run sphinx-build. This is either the
#         by default found Python interpreter or a specific version as
#         specified by the shebang (#!) of the sphinx-build script.</td>
#   </tr>
#   <tr>
#     @tp @b Sphinx_PYTHON_OPTIONS @endtp
#     <td>A list of Python options extracted from the shebang (#!) of the
#         sphinx-build script. The -E option is added by this module
#         if the Python executable is not the system default to avoid
#         problems with a differing setting of the @c PYTHONHOME.</td>
#   </tr>
#   <tr>
#     @tp @b Sphinx-build_EXECUTABLE @endtp
#     <td>Absolute path of the found sphinx-build tool.</td>
#   </tr>
#   <tr>
#     @tp @b Sphinx-apidoc_EXECUTABLE @endtp
#     <td>Absolute path of the found sphinx-apidoc tool.</td>
#   </tr>
#   <tr>
#     @tp @b Sphinx_VERSION_STRING @endtp
#     <td>Sphinx version found e.g. 1.1.2.</td>
#   </tr>
#   <tr>
#     @tp @b Sphinx_VERSION_MAJOR @endtp
#     <td>Sphinx major version found e.g. 1.</td>
#   </tr>
#   <tr>
#     @tp @b Sphinx_VERSION_MINOR @endtp
#     <td>Sphinx minor version found e.g. 1.</td>
#   </tr>
#   <tr>
#     @tp @b Sphinx_VERSION_PATCH @endtp
#     <td>Sphinx patch version found e.g. 2.</td>
#   </tr>
# </table>
#
# @ingroup CMakeFindModules
##############################################################################

#=============================================================================
# Copyright 2011-2012 University of Pennsylvania
# Copyright 2013-2016 Andreas Schuh <andreas.schuh.84@gmail.com>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

set (_Sphinx_REQUIRED_VARS)

# ----------------------------------------------------------------------------
# initialize search
if (NOT Sphinx_DIR)
    if (NOT $ENV{Sphinx_DIR} STREQUAL "")
        set (Sphinx_DIR "$ENV{Sphinx_DIR}")
    else ()
        set (Sphinx_DIR "$ENV{SPHINX_DIR}")
    endif ()
endif ()

# ----------------------------------------------------------------------------
# default components to look for
if (NOT Sphinx_FIND_COMPONENTS)
    set (Sphinx_FIND_COMPONENTS "build" "apidoc")
elseif (NOT Sphinx_FIND_COMPONENTS MATCHES "^(build|apidoc)$")
    message (FATAL_ERROR "Invalid Sphinx component in: ${Sphinx_FIND_COMPONENTS}")
endif ()

# ----------------------------------------------------------------------------
# find components, i.e., build tools
foreach (_Sphinx_TOOL IN LISTS Sphinx_FIND_COMPONENTS)
    if (Sphinx_DIR)
        find_program (
                Sphinx-${_Sphinx_TOOL}_EXECUTABLE
                NAMES         sphinx-${_Sphinx_TOOL} sphinx-${_Sphinx_TOOL}.py
                HINTS         "${Sphinx_DIR}"
                PATH_SUFFIXES bin
                DOC           "The sphinx-${_Sphinx_TOOL} Python script."
                NO_DEFAULT_PATH
        )
    else ()
        find_program (
                Sphinx-${_Sphinx_TOOL}_EXECUTABLE
                NAMES sphinx-${_Sphinx_TOOL} sphinx-${_Sphinx_TOOL}.py
                DOC   "The sphinx-${_Sphinx_TOOL} Python script."
        )
    endif ()
    mark_as_advanced (Sphinx-${_Sphinx_TOOL}_EXECUTABLE)
    list (APPEND _Sphinx_REQUIRED_VARS Sphinx-${_Sphinx_TOOL}_EXECUTABLE)
endforeach ()

# set main Sphinx_EXECUTABLE so basis_find_package can derive DEPENDS_Sphinx_DIR
if (Sphinx-build_EXECUTABLE)
    set (Sphinx_EXECUTABLE ${Sphinx-build_EXECUTABLE})
else ()
    set (Sphinx_EXECUTABLE ${Sphinx-apidoc_EXECUTABLE})
endif ()

# ----------------------------------------------------------------------------
# determine Python executable used by Sphinx
if (Sphinx-build_EXECUTABLE)
    # extract python executable from shebang of sphinx-build
    find_package (PythonInterp QUIET)
    set (Sphinx_PYTHON_EXECUTABLE "${PYTHON_EXECUTABLE}")
    set (Sphinx_PYTHON_OPTIONS)
    file (STRINGS "${Sphinx-build_EXECUTABLE}" FIRST_LINE LIMIT_COUNT 1)
    if (FIRST_LINE MATCHES "^#!(.*/python.*)") # does not match "#!/usr/bin/env python" !
        string (REGEX REPLACE "^ +| +$" "" Sphinx_PYTHON_EXECUTABLE "${CMAKE_MATCH_1}")
        if (Sphinx_PYTHON_EXECUTABLE MATCHES "([^ ]+) (.*)")
            set (Sphinx_PYTHON_EXECUTABLE "${CMAKE_MATCH_1}")
            string (REGEX REPLACE " +" ";" Sphinx_PYTHON_OPTIONS "${CMAKE_MATCH_2}")
        endif ()
    endif ()
    # this is done to avoid problems with multiple Python versions being installed
    # remember: CMake command if(STR EQUAL STR) is bad and may cause many troubles !
    string (REGEX REPLACE "([.+*?^$])" "\\\\\\1" _Sphinx_PYTHON_EXECUTABLE_RE "${PYTHON_EXECUTABLE}")
    list (FIND Sphinx_PYTHON_OPTIONS -E IDX)
    if (IDX EQUAL -1 AND NOT Sphinx_PYTHON_EXECUTABLE MATCHES "^${_Sphinx_PYTHON_EXECUTABLE_RE}$")
        list (INSERT Sphinx_PYTHON_OPTIONS 0 -E)
    endif ()
    unset (_Sphinx_PYTHON_EXECUTABLE_RE)
endif ()

# ----------------------------------------------------------------------------
# determine Sphinx version
if (Sphinx-build_EXECUTABLE)
    # intentionally use invalid -h option here as the help that is shown then
    # will include the Sphinx version information
    if (Sphinx_PYTHON_EXECUTABLE)
        execute_process (
                COMMAND "${Sphinx_PYTHON_EXECUTABLE}" ${Sphinx_PYTHON_OPTIONS} "${Sphinx-build_EXECUTABLE}" -h
                OUTPUT_VARIABLE _Sphinx_VERSION
                ERROR_VARIABLE  _Sphinx_VERSION
        )
    elseif (UNIX)
        execute_process (
                COMMAND "${Sphinx-build_EXECUTABLE}" -h
                OUTPUT_VARIABLE _Sphinx_VERSION
                ERROR_VARIABLE  _Sphinx_VERSION
        )
    endif ()
    if (_Sphinx_VERSION MATCHES "Sphinx v([0-9]+\\.[0-9]+\\.[0-9]+)")
        set (Sphinx_VERSION_STRING "${CMAKE_MATCH_1}")
        string (REPLACE "." ";" _Sphinx_VERSION "${Sphinx_VERSION_STRING}")
        list(GET _Sphinx_VERSION 0 Sphinx_VERSION_MAJOR)
        list(GET _Sphinx_VERSION 1 Sphinx_VERSION_MINOR)
        list(GET _Sphinx_VERSION 2 Sphinx_VERSION_PATCH)
        if (Sphinx_VERSION_PATCH EQUAL 0)
            string (REGEX REPLACE "\\.0$" "" Sphinx_VERSION_STRING "${Sphinx_VERSION_STRING}")
        endif ()
    endif()
endif ()

# ----------------------------------------------------------------------------
# compatibility with FindPythonInterp.cmake and FindPerl.cmake
set (SPHINX_EXECUTABLE "${Sphinx-build_EXECUTABLE}")

# ----------------------------------------------------------------------------
# handle the QUIETLY and REQUIRED arguments and set SPHINX_FOUND to TRUE if
# all listed variables are TRUE
include (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS (
        Sphinx
        REQUIRED_VARS
        ${_Sphinx_REQUIRED_VARS}
        VERSION_VAR
        Sphinx_VERSION_STRING
)

unset (_Sphinx_VERSION)
unset (_Sphinx_REQUIRED_VARS)