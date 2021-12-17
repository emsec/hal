option(PL_TEST_BOOLEAN_FUNCTION "PL_TEST_BOOLEAN_FUNCTION" OFF)
if(PL_TEST_BOOLEAN_FUNCTION OR BUILD_ALL_PLUGINS)

    enable_cxx_compile_option_if_supported("-O1" "Perf" "PUBLIC")
    enable_cxx_compile_option_if_supported("-g" "Perf" "PUBLIC")
    enable_cxx_compile_option_if_supported("-fno-inline-functions" "Perf" "PUBLIC")
    enable_cxx_compile_option_if_supported("-fno-inline-functions-called-once" "Perf" "PUBLIC")
    enable_cxx_compile_option_if_supported("-fno-optimize-sibling-calls" "Perf" "PUBLIC")
    enable_cxx_compile_option_if_supported("-fno-omit-frame-pointer" "Perf" "PUBLIC")

    file(GLOB_RECURSE TEST_BOOLEAN_FUNCTION_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE TEST_BOOLEAN_FUNCTION_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE TEST_BOOLEAN_FUNCTION_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)

    hal_add_plugin(test_boolean_function
                   SHARED
                   HEADER ${TEST_BOOLEAN_FUNCTION_INC}
                   SOURCES ${TEST_BOOLEAN_FUNCTION_SRC} ${TEST_BOOLEAN_FUNCTION_PYTHON_SRC}
                   )
endif()
