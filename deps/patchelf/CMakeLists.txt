# minimum required cmake version
cmake_minimum_required(VERSION 3.0)

set(PATCHELF_DIR patchelf)
set(PATCHELF_SRC ${PATCHELF_DIR}/patchelf.cc)
set(PATCHELF_HDR ${PATCHELF_DIR}/elf.h ${PATCHELF_DIR}/patchelf.h)

add_library (patchelf SHARED ${PATCHELF_SRC})

target_include_directories(patchelf PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
                                           "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

install(TARGETS patchelf
        EXPORT hal
        LIBRARY DESTINATION ${LIBRARY_INSTALL_DIRECTORY}
        PERMISSIONS
            OWNER_READ
            OWNER_WRITE
            OWNER_EXECUTE
            GROUP_READ
            GROUP_EXECUTE
            WORLD_READ
            WORLD_EXECUTE
        )

# Install the headers to the include directory
install(FILES ${PATCHELF_HDR} DESTINATION include/patchelf)
