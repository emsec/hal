#-------------------------------------------------------------------------------
# Get version
#-------------------------------------------------------------------------------
function(hal_get_version)
    message(STATUS "Extracting Version Information")
    execute_process(
            COMMAND ${CMAKE_SOURCE_DIR}/tools/genversion.py
            OUTPUT_VARIABLE VERSION_LIST
            OUTPUT_STRIP_TRAILING_WHITESPACE
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

    if(VERSION_LIST STREQUAL "")
        file(READ ${CMAKE_SOURCE_DIR}/CURRENT_VERSION VERSION_FILE)
        message(STATUS "VERSION_FILE: ${VERSION_FILE}")
        string(REGEX MATCHALL "v([0-9]+)\.([0-9]+)\.([0-9]+)" match ${VERSION_FILE})
        if(NOT ${match} EQUAL "")
            set(HAL_VERSION_RETURN ${VERSION_FILE} PARENT_SCOPE)
            message(STATUS "HAL_VERSION_RETURN: ${HAL_VERSION_RETURN}")
            set(HAL_VERSION_MAJOR ${CMAKE_MATCH_1} PARENT_SCOPE)
            set(HAL_VERSION_MINOR ${CMAKE_MATCH_2} PARENT_SCOPE)
            set(HAL_VERSION_PATCH ${CMAKE_MATCH_3} PARENT_SCOPE)
            set(HAL_VERSION_TWEAK 0 PARENT_SCOPE)
            set(HAL_VERSION_ADDITIONAL_COMMITS 0 PARENT_SCOPE)
            set(HAL_VERSION_HASH "unknown" PARENT_SCOPE)
            set(HAL_VERSION_DIRTY false PARENT_SCOPE)
            set(HAL_VERSION_BROKEN false PARENT_SCOPE)
        endif()
    else()

        list(GET VERSION_LIST 0 RETURN)
        list(GET VERSION_LIST 1 MAJOR)
        list(GET VERSION_LIST 2 MINOR)
        list(GET VERSION_LIST 3 PATCH)
        list(GET VERSION_LIST 4 TWEAK)
        list(GET VERSION_LIST 5 ADDITIONAL_COMMITS)
        list(GET VERSION_LIST 6 HASH)
        list(GET VERSION_LIST 7 DIRTY)
        list(GET VERSION_LIST 8 BROKEN)

        set(HAL_VERSION_RETURN ${RETURN} PARENT_SCOPE)
        set(HAL_VERSION_MAJOR ${MAJOR} PARENT_SCOPE)
        set(HAL_VERSION_MINOR ${MINOR} PARENT_SCOPE)
        set(HAL_VERSION_PATCH ${PATCH} PARENT_SCOPE)
        set(HAL_VERSION_TWEAK ${TWEAK} PARENT_SCOPE)
        set(HAL_VERSION_ADDITIONAL_COMMITS ${ADDITIONAL_COMMITS} PARENT_SCOPE)
        set(HAL_VERSION_HASH ${HASH} PARENT_SCOPE)
        set(HAL_VERSION_DIRTY ${DIRTY} PARENT_SCOPE)
        set(HAL_VERSION_BROKEN ${BROKEN} PARENT_SCOPE)
    endif()
endfunction()

function(setup_output_directories)
    message(STATUS "")
    # Declare and setup project output directories
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    foreach(i IN ITEMS "RUNTIME" "ARCHIVE" "LIBRARY")
        set(appendix "/lib")
        if(i STREQUAL "RUNTIME")
            set(appendix "/bin")
        endif(i STREQUAL "RUNTIME")
        if(NOT CMAKE_${i}_OUTPUT_DIRECTORY)
            set(CMAKE_${i}_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}${appendix}" CACHE INTERNAL "")
            message(VERBOSE "CMAKE_${i}_OUTPUT_DIRECTORY: ${CMAKE_${i}_OUTPUT_DIRECTORY}")
        elseif()
            set(CMAKE_${i}_OUTPUT_DIRECTORY "${CMAKE_${i}_OUTPUT_DIRECTORY}${appendix}" CACHE INTERNAL "")
            message(VERBOSE "CMAKE_${i}_OUTPUT_DIRECTORY: ${CMAKE_${i}_OUTPUT_DIRECTORY}")
        endif()
    endforeach()

    foreach(i IN ITEMS "_DEBUG" "_RELEASE" "_MINSIZEREL" "_RELWITHDEBINFO")
        foreach(j IN ITEMS "RUNTIME" "ARCHIVE" "LIBRARY")
            if(NOT CMAKE_${j}_OUTPUT_DIRECTORY${i})
                set(CMAKE_${j}_OUTPUT_DIRECTORY${i} ${CMAKE_${j}_OUTPUT_DIRECTORY} CACHE INTERNAL "")
                message(VERBOSE "CMAKE_${j}_OUTPUT_DIRECTORY${i}: ${CMAKE_${j}_OUTPUT_DIRECTORY${i}}")
            endif()
        endforeach()
    endforeach()

    # Install Directive target directiories
    include(GNUInstallDirs)
    set(INCLUDE_INSTALL_DIRECTORY ${CMAKE_INSTALL_INCLUDEDIR}/hal CACHE INTERNAL "")
    message(VERBOSE "INCLUDE_INSTALL_DIRECTORY: ${INCLUDE_INSTALL_DIRECTORY}")

    set(INCLUDE_INSTALL_DIRECTORY_FULL ${CMAKE_INSTALL_FULL_INCLUDEDIR}/hal CACHE INTERNAL "")
    message(VERBOSE "INCLUDE_INSTALL_DIRECTORY_FULL: ${INCLUDE_INSTALL_DIRECTORY_FULL}")

    set(SHARE_INSTALL_DIRECTORY ${CMAKE_INSTALL_DATADIR}/hal CACHE INTERNAL "")
    message(VERBOSE "SHARE_INSTALL_DIRECTORY: ${SHARE_INSTALL_DIRECTORY}")

    set(SHARE_INSTALL_DIRECTORY_FULL ${CMAKE_INSTALL_FULL_DATADIR}/hal CACHE INTERNAL "")
    message(VERBOSE "SHARE_INSTALL_DIRECTORY_FULL: ${SHARE_INSTALL_DIRECTORY_FULL}")

    set(PKGCONFIG_INSTALL_DIRECTORY ${CMAKE_INSTALL_DATADIR}/pkgconfig CACHE INTERNAL "")
    message(VERBOSE "PKGCONFIG_INSTALL_DIRECTORY: ${PKGCONFIG_INSTALL_DIRECTORY}")

    set(BINARY_INSTALL_DIRECTORY ${CMAKE_INSTALL_BINDIR} CACHE INTERNAL "")
    message(VERBOSE "BINARY_INSTALL_DIRECTORY: ${BINARY_INSTALL_DIRECTORY}")

    set(BINARY_INSTALL_DIRECTORY_FULL ${CMAKE_INSTALL_FULL_BINDIR} CACHE INTERNAL "")
    message(VERBOSE "BINARY_INSTALL_DIRECTORY_FULL: ${BINARY_INSTALL_DIRECTORY_FULL}")

    set(LIBRARY_INSTALL_DIRECTORY ${CMAKE_INSTALL_LIBDIR} CACHE INTERNAL "")
    message(VERBOSE "LIBRARY_INSTALL_DIRECTORY: ${LIBRARY_INSTALL_DIRECTORY}")

    set(LIBRARY_INSTALL_DIRECTORY_FULL ${CMAKE_INSTALL_FULL_LIBDIR} CACHE INTERNAL "")
    message(VERBOSE "LIBRARY_INSTALL_DIRECTORY_FULL: ${LIBRARY_INSTALL_DIRECTORY_FULL}")

    set(PLUGIN_INCLUDE_INSTALL_DIRECTORY ${INCLUDE_INSTALL_DIRECTORY}/hal_plugins CACHE INTERNAL "")
    message(VERBOSE "PLUGIN_INCLUDE_INSTALL_DIRECTORY: ${PLUGIN_INCLUDE_INSTALL_DIRECTORY}")

    set(PLUGIN_BINARY_INSTALL_DIRECTORY ${BINARY_INSTALL_DIRECTORY} CACHE INTERNAL "")
    message(VERBOSE "PLUGIN_BINARY_INSTALL_DIRECTORY: ${PLUGIN_BINARY_INSTALL_DIRECTORY}")

    set(PLUGIN_LIBRARY_INSTALL_DIRECTORY ${LIBRARY_INSTALL_DIRECTORY}/hal_plugins CACHE INTERNAL "")
    message(VERBOSE "PLUGIN_LIBRARY_INSTALL_DIRECTORY: ${PLUGIN_LIBRARY_INSTALL_DIRECTORY}")
    message(STATUS "")
endfunction()

include(CheckCXXCompilerFlag)
function(enable_cxx_compile_option_if_supported flag build_type visibility)
    list(FIND "COMPILE_OPTIONS_${visibility}" "${flag}" flag_already_set)
    list(FIND "COMPILE_OPTIONS_${visibility}" "$<$<CONFIG:${build_type}>:${flag}>" flag_already_set2)
    if((flag_already_set EQUAL -1) AND (flag_already_set2 EQUAL -1))
        check_cxx_compiler_flag("${flag}" supports_${flag})
        if(supports_${flag} AND (NOT (build_type STREQUAL "")))
            set(COMPILE_OPTIONS_${visibility} "${COMPILE_OPTIONS_${visibility}};$<$<CONFIG:${build_type}>:${flag}>" PARENT_SCOPE)
        elseif(supports_${flag} AND (build_type STREQUAL ""))
            set(COMPILE_OPTIONS_${visibility} "${COMPILE_OPTIONS_${visibility}};${flag}" PARENT_SCOPE)
        endif()
        unset(supports_${flag} CACHE)
    endif()
    unset(flag_already_set CACHE)
endfunction()

include(CheckCCompilerFlag)
function(enable_c_compile_option_if_supported flag build_type visibility)
    list(FIND "COMPILE_OPTIONS_${visibility}" "${flag}" flag_already_set)
    list(FIND "COMPILE_OPTIONS_${visibility}" "$<$<CONFIG:${build_type}>:${flag}>" flag_already_set2)
    if((flag_already_set EQUAL -1) AND (flag_already_set2 EQUAL -1))
        check_c_compiler_flag("${flag}" supports_${flag})
        if(supports_${flag} AND (NOT (build_type STREQUAL "")))
            set(COMPILE_OPTIONS_${visibility} "${COMPILE_OPTIONS_${visibility}};$<$<CONFIG:${build_type}>:${flag}>" PARENT_SCOPE)
        elseif(supports_${flag} AND (build_type STREQUAL ""))
            set(COMPILE_OPTIONS_${visibility} "${COMPILE_OPTIONS_${visibility}};${flag}" PARENT_SCOPE)
        endif()
        unset(supports_${flag} CACHE)
    endif()
    unset(flag_already_set CACHE)
endfunction()

# DetectDistro.cmake -- Detect Linux distribution
function(detect_distro)
    message(STATUS "System is: ${CMAKE_SYSTEM_NAME}")
    set(LINUX_DISTRO "" CACHE INTERNAL "")
    set(LINUX_DISTRO_VERSION_NAME "" CACHE INTERNAL "")
    if(LINUX)
        # Detect Linux distribution (if possible)
        execute_process(COMMAND "/usr/bin/lsb_release" "-is"
                        TIMEOUT 4
                        OUTPUT_VARIABLE LINUX_DISTRO
                        ERROR_QUIET
                        OUTPUT_STRIP_TRAILING_WHITESPACE)
        message(STATUS "Linux distro is: ${LINUX_DISTRO}")

        # Detect Linux distribution (if possible)
        execute_process(COMMAND "/usr/bin/lsb_release" "-cs"
                        TIMEOUT 4
                        OUTPUT_VARIABLE LINUX_DISTRO_VERSION_NAME
                        ERROR_QUIET
                        OUTPUT_STRIP_TRAILING_WHITESPACE)
        message(STATUS "Linux Distro version name is: ${LINUX_DISTRO_VERSION_NAME}")
    endif()
    execute_process(COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE)
    message(STATUS "System Arch is: ${ARCHITECTURE}")
endfunction()

MACRO(SUBDIRLIST result curdir)
    FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
    SET(dirlist "")
    FOREACH(child ${children})
        IF(IS_DIRECTORY ${curdir}/${child})
            LIST(APPEND dirlist ${child})
        ENDIF()
    ENDFOREACH()
    SET(${result} ${dirlist})
ENDMACRO()

macro(configure_files src_dir dst_dir extension)
    message(STATUS "Configuring directory ${dst_dir}")
    make_directory(${dst_dir})

    file(GLOB files_to_copy RELATIVE ${src_dir} ${src_dir}/*.${extension})
    foreach(file_to_copy ${files_to_copy})
        set(file_path_to_copy ${src_dir}/${file_to_copy})
        if(NOT IS_DIRECTORY ${file_path_to_copy})
            message(STATUS "Configuring file ${file_to_copy}")
            configure_file(
                    ${file_path_to_copy}
                    ${dst_dir}/${file_to_copy}
                    @ONLY)
        endif(NOT IS_DIRECTORY ${file_path_to_copy})
    endforeach(file_to_copy)
endmacro(configure_files)

# Build a hal plugin:
# hal_add_plugin(<name> [MODULE | SHARED] [EXCLUDE_FROM_ALL]
#                     [NO_EXTRAS] [SYSTEM] [THIN_LTO] source1 [source2 ...])
#
function(hal_add_plugin target_name)
    set(options  MODULE SHARED EXCLUDE_FROM_ALL NO_EXTRAS SYSTEM THIN_LTO)
    set(oneValueArgs DESTINATION RENAME)
    set(multiValueArgs TARGETS CONFIGURATIONS)
    set(options MODULE SHARED EXCLUDE_FROM_ALL NO_EXTRAS SYSTEM THIN_LTO)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN} )

    file(GLOB_RECURSE GRAPH_ALGORITHM_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE GRAPH_ALGORITHM_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE PYTHON_BINDING_LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/python_definitions.cpp)

    add_library(${target_name} SHARED ${GRAPH_ALGORITHM_SRC} ${PYTHON_BINDING_LIB_SRC} ${GRAPH_ALGORITHM_INC})
    target_include_directories(${TARGET_NAME}
                               PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
                               $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
                               $<BUILD_INTERFACE:${IGRAPH_INCLUDES}>
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                               $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                               )
    target_compile_definitions(${TARGET_NAME} PUBLIC
                               -DIGRAPH_VERSION_MAJOR_GUESS=${IGRAPH_VERSION_MAJOR_GUESS}
                               -DIGRAPH_VERSION_MINOR_GUESS=${IGRAPH_VERSION_MINOR_GUESS}
                               -DIGRAPH_VERSION_PATCH_GUESS=${IGRAPH_VERSION_PATCH_GUESS}
                               )
    #Set shared library suffix for MacOS
    if(APPLE AND CMAKE_HOST_APPLE)
        set_target_properties(${TARGET_NAME} PROPERTIES SUFFIX ".so")
        if(APPLE AND CMAKE_HOST_APPLE)
            set_target_properties(${TARGET_NAME} PROPERTIES
                                  INSTALL_NAME_DIR ${PLUGIN_LIBRARY_INSTALL_DIRECTORY}
                                  )
        endif()
    endif(APPLE AND CMAKE_HOST_APPLE)
    target_link_libraries(${TARGET_NAME} hal::core hal::netlist ${PYTHON_LIBRARIES} pybind11::pybind11 ${IGRAPH_LIBRARIES})
    install(TARGETS ${TARGET_NAME} LIBRARY DESTINATION ${PLUGIN_LIBRARY_INSTALL_DIRECTORY} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE INCLUDES DESTINATION ${PLUGIN_INCLUDE_INSTALL_DIRECTORY})
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${PLUGIN_INCLUDE_INSTALL_DIRECTORY}/graph_algorithm/include/)
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(${TARGET_NAME})
    endif()
#    if(ARG_MODULE AND ARG_SHARED)
#        message(FATAL_ERROR "Can't be both MODULE and SHARED")
#    elseif(ARG_SHARED)
#        set(lib_type SHARED)
#    else()
#        set(lib_type MODULE)
#    endif()
#
#    if(ARG_EXCLUDE_FROM_ALL)
#        set(exclude_from_all EXCLUDE_FROM_ALL)
#    endif()
#
#    add_library(${target_name} ${lib_type} ${exclude_from_all} ${ARG_UNPARSED_ARGUMENTS})
#
#    if(ARG_SYSTEM)
#        set(inc_isystem SYSTEM)
#    endif()
#
#    target_include_directories(${target_name} ${inc_isystem}
#                               PRIVATE ${PYBIND11_INCLUDE_DIR}  # from project CMakeLists.txt
#                               PRIVATE ${pybind11_INCLUDE_DIR}  # from pybind11Config
#                               PRIVATE ${PYTHON_INCLUDE_DIRS})
#
#    # Python debug libraries expose slightly different objects
#    # https://docs.python.org/3.6/c-api/intro.html#debugging-builds
#    # https://stackoverflow.com/questions/39161202/how-to-work-around-missing-pymodule-create2-in-amd64-win-python35-d-lib
#    if(PYTHON_IS_DEBUG)
#        target_compile_definitions(${target_name} PRIVATE Py_DEBUG)
#    endif()
#
#    # The prefix and extension are provided by FindPythonLibsNew.cmake
#    set_target_properties(${target_name} PROPERTIES PREFIX "${PYTHON_MODULE_PREFIX}")
#    set_target_properties(${target_name} PROPERTIES SUFFIX "${PYTHON_MODULE_EXTENSION}")
#
#    # -fvisibility=hidden is required to allow multiple modules compiled against
#    # different pybind versions to work properly, and for some features (e.g.
#    # py::module_local).  We force it on everything inside the `pybind11`
#    # namespace; also turning it on for a pybind module compilation here avoids
#    # potential warnings or issues from having mixed hidden/non-hidden types.
#    set_target_properties(${target_name} PROPERTIES CXX_VISIBILITY_PRESET "hidden")
#    set_target_properties(${target_name} PROPERTIES CUDA_VISIBILITY_PRESET "hidden")
#
#    if(WIN32 OR CYGWIN)
#        # Link against the Python shared library on Windows
#        target_link_libraries(${target_name} PRIVATE ${PYTHON_LIBRARIES})
#    elseif(APPLE)
#        # It's quite common to have multiple copies of the same Python version
#        # installed on one's system. E.g.: one copy from the OS and another copy
#        # that's statically linked into an application like Blender or Maya.
#        # If we link our plugin library against the OS Python here and import it
#        # into Blender or Maya later on, this will cause segfaults when multiple
#        # conflicting Python instances are active at the same time (even when they
#        # are of the same version).
#
#        # Windows is not affected by this issue since it handles DLL imports
#        # differently. The solution for Linux and Mac OS is simple: we just don't
#        # link against the Python library. The resulting shared library will have
#        # missing symbols, but that's perfectly fine -- they will be resolved at
#        # import time.
#
#        target_link_libraries(${target_name} PRIVATE "-undefined dynamic_lookup")
#
#        if(ARG_SHARED)
#            # Suppress CMake >= 3.0 warning for shared libraries
#            set_target_properties(${target_name} PROPERTIES MACOSX_RPATH ON)
#        endif()
#    endif()
#
#    # Make sure C++11/14 are enabled
#    if(CMAKE_VERSION VERSION_LESS 3.3)
#        target_compile_options(${target_name} PUBLIC ${PYBIND11_CPP_STANDARD})
#    else()
#        target_compile_options(${target_name} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:${PYBIND11_CPP_STANDARD}>)
#    endif()
#
#    if(ARG_NO_EXTRAS)
#        return()
#    endif()
#
#    _pybind11_add_lto_flags(${target_name} ${ARG_THIN_LTO})
#
#    if (NOT MSVC AND NOT ${CMAKE_BUILD_TYPE} MATCHES Debug|RelWithDebInfo)
#        # Strip unnecessary sections of the binary on Linux/Mac OS
#        if(CMAKE_STRIP)
#            if(APPLE)
#                add_custom_command(TARGET ${target_name} POST_BUILD
#                                   COMMAND ${CMAKE_STRIP} -x $<TARGET_FILE:${target_name}>)
#            else()
#                add_custom_command(TARGET ${target_name} POST_BUILD
#                                   COMMAND ${CMAKE_STRIP} $<TARGET_FILE:${target_name}>)
#            endif()
#        endif()
#    endif()
#
#    if(MSVC)
#        # /MP enables multithreaded builds (relevant when there are many files), /bigobj is
#        # needed for bigger binding projects due to the limit to 64k addressable sections
#        target_compile_options(${target_name} PRIVATE /bigobj)
#        if(CMAKE_VERSION VERSION_LESS 3.11)
#            target_compile_options(${target_name} PRIVATE $<$<NOT:$<CONFIG:Debug>>:/MP>)
#        else()
#            # Only set these options for C++ files.  This is important so that, for
#            # instance, projects that include other types of source files like CUDA
#            # .cu files don't get these options propagated to nvcc since that would
#            # cause the build to fail.
#            target_compile_options(${target_name} PRIVATE $<$<NOT:$<CONFIG:Debug>>:$<$<COMPILE_LANGUAGE:CXX>:/MP>>)
#        endif()
#    endif()
endfunction()