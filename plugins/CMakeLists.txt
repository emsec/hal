foreach(i IN ITEMS "" "_DEBUG" "_RELEASE" "_MINSIZEREL" "_RELWITHDEBINFO")
    foreach(j IN ITEMS "RUNTIME" "ARCHIVE" "LIBRARY")
        set(TMP_${j}_OUTPUT_DIR${i} ${CMAKE_${j}_OUTPUT_DIRECTORY${i}})
        set(CMAKE_${j}_OUTPUT_DIRECTORY${i} "${TMP_${j}_OUTPUT_DIR${i}}/hal_plugins")
    endforeach()
endforeach()

include_directories(${include_directories}
                    ${CMAKE_SOURCE_DIR}/include
                    ${CMAKE_SOURCE_DIR}/hal_plugins
                   )

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/lib/hal_plugins/)
file(WRITE ${CMAKE_BINARY_DIR}/lib/hal_plugins/__init__.py "")
install(FILES ${CMAKE_BINARY_DIR}/lib/hal_plugins/__init__.py DESTINATION ${PLUGIN_LIBRARY_INSTALL_DIRECTORY})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})
FOREACH(subdir ${SUBDIRS})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/CMakeLists.txt")
        ADD_SUBDIRECTORY(${subdir})
    endif()
ENDFOREACH()

foreach(i IN ITEMS "" "_DEBUG" "_RELEASE" "_MINSIZEREL" "_RELWITHDEBINFO")
    foreach(j IN ITEMS "RUNTIME" "ARCHIVE" "LIBRARY")
        set(CMAKE_${j}_OUTPUT_DIRECTORY${i} "${TMP_${j}_OUTPUT_DIR${i}}")
    endforeach()
endforeach()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${TMP_RUNTIME_OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${TMP_RUNTIME_OUTPUT_DIR_DEBUG})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${TMP_RUNTIME_OUTPUT_DIR})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${TMP_ARCHIVE_OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${TMP_ARCHIVE_OUTPUT_DIR_DEBUG})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${TMP_ARCHIVE_OUTPUT_DIR})
