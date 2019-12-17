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
        message(VERBOSE "VERSION_FILE: ${VERSION_FILE}")
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
    set(options  MODULE SHARED EXCLUDE_FROM_ALL INSTALL)
    set(oneValueArgs RENAME)
    set(multiValueArgs HEADER SOURCES LINK_LIBRARIES INCLUDES DEFINITIONS COMPILE_OPTIONS LINK_OPTIONS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN} )


    if(ARG_MODULE AND ARG_SHARED)
        message(FATAL_ERROR "Can't be both MODULE and SHARED")
    elseif(ARG_SHARED)
        set(lib_type SHARED)
    else()
        set(lib_type MODULE)
    endif()

    if(ARG_EXCLUDE_FROM_ALL)
        set(exclude_from_all EXCLUDE_FROM_ALL)
    endif()

    add_library(${target_name} ${lib_type} ${exclude_from_all} ${ARG_HEADER} ${ARG_SOURCES} ${ARG_PY_SOURCES})

    target_include_directories(${target_name}
                               PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
                               $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
                               $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                               ${ARG_INCLUDES}
                               )
    if (ARG_DEFINITIONS)
    target_compile_definitions(${target_name}
                               ${ARG_DEFINITIONS}
                               )
    endif()

    target_compile_options(${target_name}
                           PUBLIC ${COMPILE_OPTIONS_PUBLIC}
                           PRIVATE ${COMPILE_OPTIONS_PRIVATE}
                           INTERFACE ${COMPILE_OPTIONS_INTERFACE}
                           ${ARG_COMPILE_OPTIONS})
    if (ARG_LINK_OPTIONS)
        target_link_options(${target_name} ${ARG_LINK_OPTIONS})
    endif()
    #Set shared library suffix for MacOS
    if(APPLE AND CMAKE_HOST_APPLE)
        set_target_properties(${target_name} PROPERTIES SUFFIX ".so")
        set_target_properties(${target_name} PROPERTIES INSTALL_NAME_DIR ${PLUGIN_LIBRARY_INSTALL_DIRECTORY})
    endif(APPLE AND CMAKE_HOST_APPLE)

    target_link_libraries(${target_name}
                          PUBLIC hal::core hal::netlist ${PYTHON_LIBRARIES} pybind11::pybind11
                          ${ARG_LINK_LIBRARIES})
    install(TARGETS ${target_name} LIBRARY DESTINATION ${PLUGIN_LIBRARY_INSTALL_DIRECTORY} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE INCLUDES DESTINATION ${PLUGIN_INCLUDE_INSTALL_DIRECTORY})
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(${target_name})
    endif()
endfunction()