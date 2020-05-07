include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests)

add_executable(runTest-callback_hook
               callback_hook.cpp)

add_executable(runTest-log
            log.cpp)

add_executable(runTest-program_arguments
        program_arguments.cpp)

add_executable(runTest-program_options
        program_options.cpp)

add_executable(runTest-utils
        utils.cpp)

add_executable(runTest-plugin_manager
        plugin_manager.cpp)

target_link_libraries(runTest-callback_hook   pthread  gtest gtest_main hal::core hal::netlist test_utils)
target_link_libraries(runTest-log   pthread  gtest gtest_main hal::core hal::netlist test_utils)
target_link_libraries(runTest-program_arguments   pthread  gtest gtest_main hal::core hal::netlist test_utils)
target_link_libraries(runTest-program_options   pthread  gtest gtest_main hal::core hal::netlist test_utils)
target_link_libraries(runTest-utils pthread   gtest gtest_main hal::core hal::netlist test_utils)
target_link_libraries(runTest-plugin_manager   pthread  gtest gtest_main hal::core hal::netlist test_utils)


add_test(runTest-callback_hook_test ${CMAKE_BINARY_DIR}/bin/runTest-callback_hook --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
add_test(runTest-log_test ${CMAKE_BINARY_DIR}/bin/runTest-log --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
add_test(runTest-program_arguments_test ${CMAKE_BINARY_DIR}/bin/runTest-program_arguments --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
add_test(runTest-program_options_test ${CMAKE_BINARY_DIR}/bin/runTest-program_options --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
add_test(runTest-utils_test ${CMAKE_BINARY_DIR}/bin/runTest-utils --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
add_test(runTest-plugin_manager_test ${CMAKE_BINARY_DIR}/bin/runTest-plugin_manager --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)

# Test plugin:
foreach(i IN ITEMS "" "_DEBUG" "_RELEASE" "_MINSIZEREL" "_RELWITHDEBINFO")
    foreach(j IN ITEMS "RUNTIME" "ARCHIVE" "LIBRARY")
        set(TMP_${j}_OUTPUT_DIR${i} ${CMAKE_${j}_OUTPUT_DIRECTORY${i}})
        set(CMAKE_${j}_OUTPUT_DIRECTORY${i} "${TMP_${j}_OUTPUT_DIR${i}}/hal_plugins")
    endforeach()
endforeach()

add_subdirectory("test_plugin")


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