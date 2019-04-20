include(ExternalProject)
if(WIN32)
    set(BUDDY_LIB_NAME ${CMAKE_FIND_LIBRARY_PREFIXES}bdd.dll)
elseif(APPLE)
    set(BUDDY_LIB_NAME ${CMAKE_FIND_LIBRARY_PREFIXES}bdd.dylib)
else()
    set(BUDDY_LIB_NAME ${CMAKE_FIND_LIBRARY_PREFIXES}bdd.so)
endif()
ExternalProject_Add(buddy
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}/deps/buddy-2.4/
                    #URL http://downloads.sourceforge.net/project/buddy/buddy/BuDDy%202.4/buddy-2.4.tar.gz
                    DOWNLOAD_COMMAND ""
                    UPDATE_COMMAND ""
                    CONFIGURE_COMMAND cp -R <SOURCE_DIR>/. <BINARY_DIR> && <BINARY_DIR>/configure --prefix=${CMAKE_CURRENT_BINARY_DIR}/deps
                    BUILD_COMMAND ${MAKE}
                    BUILD_IN_SOURCE 0
                    INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/deps
                    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/deps
                    BUILD_BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/deps/lib/${BUDDY_LIB_NAME}
                    )
ExternalProject_Get_Property(buddy install_dir)
set(BUDDY_INCLUDE_DIR ${install_dir}/include)
MESSAGE(STATUS "BUDDY_INCLUDE_DIR:           " ${BUDDY_INCLUDE_DIR})
set(BUDDY_LIBRARY_PATH ${install_dir}/lib/${BUDDY_LIB_NAME})
set(BUDDY_LIBRARY buddy_lib)
add_library(${BUDDY_LIBRARY} SHARED IMPORTED)
set_property(TARGET ${BUDDY_LIBRARY} PROPERTY IMPORTED_LOCATION
             ${BUDDY_LIBRARY_PATH})
add_dependencies(${BUDDY_LIBRARY} buddy)
MESSAGE(STATUS "BUDDY_LIBRARY_PATH:      " ${BUDDY_LIBRARY_PATH})
MESSAGE(STATUS "BUDDY_LIBRARY:           " ${BUDDY_LIBRARY})
if(APPLE)
    file(GLOB BDD_LIB ${install_dir}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}bdd*.dylib*)
    message(STATUS "BDD_LIB: ${BDD_LIB}")
    install(FILES ${BDD_LIB} DESTINATION ${LIBRARY_INSTALL_DIRECTORY})
elseif(LINUX)
    file(GLOB BDD_LIB ${install_dir}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}bdd.so*)
    message(STATUS "BDD_LIB: ${BDD_LIB}")
    install(FILES ${BDD_LIB} DESTINATION ${LIBRARY_INSTALL_DIRECTORY})
endif(APPLE)

# Pattern for CMake based project
#include(ExternalProject)
#if(WIN32)
#    set(BUDDY_LIB_NAME ${CMAKE_FIND_LIBRARY_PREFIXES}bdd.dll)
#elseif(APPLE)
#    set(BUDDY_LIB_NAME ${CMAKE_FIND_LIBRARY_PREFIXES}bdd.dylib)
#else()
#    set(BUDDY_LIB_NAME ${CMAKE_FIND_LIBRARY_PREFIXES}bdd.so)
#endif()
#message(STATUS "BUDDY_LIB_NAME: ${BUDDY_LIB_NAME}")
#
#ExternalProject_Add(buddy
#                    SOURCE_DIR ${CMAKE_SOURCE_DIR}/deps/buddy-2.4/
#                    DOWNLOAD_COMMAND ""
#                    UPDATE_COMMAND ""
#                    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/deps
#                    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DBUDDY_BUILD_EXAMPLES=OFF
#                    BUILD_COMMAND ${MAKE}
#                    INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/deps
#                    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/deps
#                    BUILD_BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/deps/lib/${BUDDY_LIB_NAME}
#                    )
#ExternalProject_Get_Property(buddy install_dir)
#set(BUDDY_INCLUDE_DIR ${install_dir}/include)
#MESSAGE(STATUS "BUDDY_INCLUDE_DIR:           " ${BUDDY_INCLUDE_DIR})
#set(BUDDY_LIBRARY_PATH ${CMAKE_CURRENT_BINARY_DIR}/deps/lib/${BUDDY_LIB_NAME})
#set(BUDDY_LIBRARY bdd)
#add_library(${BUDDY_LIBRARY} SHARED IMPORTED)
#set_property(TARGET ${BUDDY_LIBRARY} PROPERTY IMPORTED_LOCATION
#             ${BUDDY_LIBRARY_PATH})
#add_dependencies(${BUDDY_LIBRARY} buddy)
#MESSAGE(STATUS "BUDDY_LIBRARY_PATH:      " ${BUDDY_LIBRARY_PATH})
#MESSAGE(STATUS "BUDDY_LIBRARY:           " ${BUDDY_LIBRARY})
#if(APPLE)
##    file(GLOB BDD_LIB ${install_dir}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}bdd.dylib)
#        set(BDD_LIB ${install_dir}/deps/lib/${BUDDY_LIB_NAME})
#    message(STATUS "BDD_LIB: ${BDD_LIB}")
#    install(FILES ${BDD_LIB} DESTINATION ${LIBRARY_INSTALL_DIRECTORY})
#elseif(LINUX)
##    file(GLOB BDD_LIB ${install_dir}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}bdd.so)
#    set(BDD_LIB ${install_dir}/deps/lib/${BUDDY_LIB_NAME})
#    message(STATUS "BDD_LIB: ${BDD_LIB}")
#    install(FILES ${BDD_LIB} DESTINATION ${LIBRARY_INSTALL_DIRECTORY})
#endif(APPLE)
