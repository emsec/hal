option(PL_VERILATOR "PL_VERILATOR" OFF)

if(PL_VERILATOR OR PL_SIMULATOR OR BUILD_ALL_PLUGINS)

    file(GLOB_RECURSE VERILATOR_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE VERILATOR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE VERILATOR_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)


    enable_cxx_compile_option_if_supported("-O1" "Perf" "PUBLIC")
    enable_cxx_compile_option_if_supported("-g" "Perf" "PUBLIC")
    enable_cxx_compile_option_if_supported("-fno-inline-functions" "Perf" "PUBLIC")
    enable_cxx_compile_option_if_supported("-fno-inline-functions-called-once" "Perf" "PUBLIC")
    enable_cxx_compile_option_if_supported("-fno-optimize-sibling-calls" "Perf" "PUBLIC")

    hal_add_plugin(verilator
                   SHARED
                   HEADER ${VERILATOR_INC}
                   SOURCES ${VERILATOR_SRC} ${VERILATOR_PYTHON_SRC}
                   PYDOC SPHINX_DOC_INDEX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/documentation/verilator.rst
                   LINK_LIBRARIES PUBLIC netlist_simulator_controller verilog_writer)


    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_sanitizers(verilator)
    endif()
    
endif()
