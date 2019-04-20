include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include)

add_executable(runBasicTests
               basic_check.cpp)

target_link_libraries(runBasicTests gtest gtest_main ${LINK_LIBS})

add_test(runBasicTests_test ${CMAKE_BINARY_DIR}/bin/runBasicTests --gtest_output=xml:${CMAKE_BINARY_DIR}/gtestresults-runBasicTests.xml)