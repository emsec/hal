# minimum required cmake version
cmake_minimum_required(VERSION 3.0)

set(PATCHELF_SRC patchelf.cc)
set(PATCHELF_HDR elf.h patchelf.h)

add_library (patchelf::patchelf SHARED ${PATCHELF_SRC})

target_include_directories(patchelf::patchelf PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
                                                     "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

