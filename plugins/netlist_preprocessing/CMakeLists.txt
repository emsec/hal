#Option moved to plugins/CMakeLists.txt since dependencies need to be build as well
#option(PL_NETLIST_PREPROCESSING "PL_NETLIST_PREPROCESSING" ON)

if(PL_NETLIST_PREPROCESSING OR BUILD_ALL_PLUGINS)
    file(GLOB_RECURSE NETLIST_PREPROCESSING_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE NETLIST_PREPROCESSING_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE NETLIST_PREPROCESSING_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)

    hal_add_plugin(netlist_preprocessing
        SHARED
        HEADER ${NETLIST_PREPROCESSING_INC}
        SOURCES ${NETLIST_PREPROCESSING_SRC} ${NETLIST_PREPROCESSING_PYTHON_SRC}
        PYDOC SPHINX_DOC_INDEX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/documentation/netlist_preprocessing.rst
        LINK_LIBRARIES PUBLIC ${Z3_LIBRARIES} z3_utils resynthesis OpenMP::OpenMP_CXX nlohmann_json::nlohmann_json
    )

    add_subdirectory(test)
endif()
