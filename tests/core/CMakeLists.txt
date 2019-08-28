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

target_link_libraries(runTest-callback_hook   gtest gtest_main ${LINK_LIBS} test_utils)
target_link_libraries(runTest-log   gtest gtest_main ${LINK_LIBS} test_utils)
target_link_libraries(runTest-program_arguments   gtest gtest_main ${LINK_LIBS} test_utils)
target_link_libraries(runTest-program_options   gtest gtest_main ${LINK_LIBS} test_utils)
target_link_libraries(runTest-utils   gtest gtest_main ${LINK_LIBS} test_utils)
target_link_libraries(runTest-plugin_manager   gtest gtest_main ${LINK_LIBS} test_utils)


add_test(runTest-callback_hook_test ${CMAKE_BINARY_DIR}/bin/runTest-callback_hook --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
add_test(runTest-log_test ${CMAKE_BINARY_DIR}/bin/runTest-log --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
add_test(runTest-program_arguments_test ${CMAKE_BINARY_DIR}/bin/runTest-program_arguments --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
add_test(runTest-program_options_test ${CMAKE_BINARY_DIR}/bin/runTest-program_options --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
add_test(runTest-utils_test ${CMAKE_BINARY_DIR}/bin/runTest-utils --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)
add_test(runTest-plugin_manager_test ${CMAKE_BINARY_DIR}/bin/runTest-plugin_manager --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)