include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests)

add_library(
    test_utils
        include/test_def.h
        include/netlist_test_utils.h
        src/test_def.cpp
        src/netlist_test_utils.cpp
)

target_include_directories(test_utils PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(test_utils  pthread  gtest gtest_main ${LINK_LIBS})