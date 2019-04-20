# DetectDistro.cmake -- Detect Linux distribution

message(STATUS "System is: ${CMAKE_SYSTEM_NAME}")
set(LINUX_DISTRO "")
set(LINUX_DISTRO_VERSION_NAME "")
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
EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )
message(STATUS "System Arch is: ${ARCHITECTURE}")