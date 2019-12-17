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