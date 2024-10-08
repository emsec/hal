# ###############################
# ####   PkgConfig
# ###############################

find_package(PkgConfig REQUIRED)

if(PkgConfig_FOUND)
    message(VERBOSE "PKG_CONFIG_EXECUTABLE: ${PKG_CONFIG_EXECUTABLE}")
elseif(NOT PkgConfig_FOUND)
    set(missing_package "TRUE")

    if(LINUX)
        message(STATUS "Please install PkgConfig (https://linux.die.net/man/1/pkg-config)")
    endif(LINUX)

    if(APPLE AND CMAKE_HOST_APPLE)
        message(STATUS "To install pkgconfig on MacOS using homebrew run following command:")
        message(STATUS "    brew install pkgconfig")
    endif(APPLE AND CMAKE_HOST_APPLE)
endif(PkgConfig_FOUND)

# ###############################
# ####   Sanitizers
# ###############################
find_package(Sanitizers REQUIRED)

# ###############################
# ####   Bitwuzla
# ###############################
pkg_check_modules(BITWUZLA bitwuzla)

if(BITWUZLA_FOUND)
    message(STATUS "Found BITWUZLA")
    message(STATUS "    BITWUZLA_LIBRARIES: ${BITWUZLA_LIBRARIES}")
    message(STATUS "    BITWUZLA_LINK_LIBRARIES: ${BITWUZLA_LINK_LIBRARIES}")
    message(STATUS "    BITWUZLA_INCLUDE_DIRS: ${BITWUZLA_INCLUDE_DIRS}")
else()
    set(BITWUZLA_LIBRARY "")
    set(BITWUZLA_INCLUDE_DIRS "")
    message(STATUS "Bitwuzla not found, but this is optional...")
endif(BITWUZLA_FOUND)


# ###############################
# ####   OpenMP
# ###############################

# See cmake/detect_dependencies.cmake
if(APPLE AND CMAKE_HOST_APPLE)
    find_package(OpenMP)

    if(OPENMP_FOUND OR OPENMP_CXX_FOUND)
        message(VERBOSE "Found libomp without any special flags")
    endif()

    # If OpenMP wasn't found, try if we can find it in the default Macports location
    if((NOT OPENMP_FOUND) AND(NOT OPENMP_CXX_FOUND) AND EXISTS "/opt/local/lib/libomp/libomp.dylib") # older cmake uses OPENMP_FOUND, newer cmake also sets OPENMP_CXX_FOUND, homebrew installations seem only to get the latter set.
        set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I/opt/local/include/libomp/")
        set(OpenMP_CXX_LIB_NAMES omp)
        set(OpenMP_omp_LIBRARY /opt/local/lib/libomp/libomp.dylib)

        find_package(OpenMP)

        if(OPENMP_FOUND OR OPENMP_CXX_FOUND)
            message(VERBOSE "Found libomp in macports default location.")
        else()
            message(FATAL_ERROR "Didn't find libomp. Tried macports default location but also didn't find it.")
        endif()
    endif()

    # If OpenMP wasn't found, try if we can find it in the default Homebrew location
    if((NOT OPENMP_FOUND) AND(NOT OPENMP_CXX_FOUND) AND EXISTS "/usr/local/opt/libomp/lib/libomp.dylib")
        set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I/usr/local/opt/libomp/include")
        set(OpenMP_CXX_LIB_NAMES omp)
        set(OpenMP_omp_LIBRARY /usr/local/opt/libomp/lib/libomp.dylib)

        find_package(OpenMP)

        if(OPENMP_FOUND OR OPENMP_CXX_FOUND)
            message(VERBOSE "Found libomp in homebrew default location.")
        else()
            message(FATAL_ERROR "Didn't find libomp. Tried homebrew default location but also didn't find it.")
        endif()
    endif()

    set(Additional_OpenMP_Libraries_Workaround "")

    # Workaround because older cmake on apple doesn't support FindOpenMP
    if((NOT OPENMP_FOUND) AND(NOT OPENMP_CXX_FOUND))
        if((APPLE AND((CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang") OR(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")))
            AND((CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "7.0") AND(CMAKE_VERSION VERSION_LESS "3.12.0")))
            message(VERBOSE "Applying workaround for OSX OpenMP with old cmake that doesn't have FindOpenMP")
            set(OpenMP_CXX_FLAGS "-Xclang -fopenmp")
            set(Additional_OpenMP_Libraries_Workaround "-lomp")
        else()
            message(FATAL_ERROR "Did not find OpenMP. Build with -DDISABLE_OPENMP=ON if you want to allow this and are willing to take the performance hit.")
        endif()
    endif()
else()
    find_package(OpenMP REQUIRED)

    if(OpenMP_FOUND)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
    endif()
endif()

if(OPENMP_FOUND)
    message(STATUS "openMP found")
else()
    message(STATUS "openMP not found")
endif()

# ###############################
# ####   Filesystem
# ###############################
find_package(Filesystem REQUIRED Final Experimental)


# ###############################
# ####   RapidJSON
# ###############################
find_package(RapidJSON REQUIRED)
if(RapidJSON_FOUND AND NOT TARGET RapidJSON::RapidJSON)
    if(NOT RAPIDJSON_INCLUDEDIR)
        set(RAPIDJSON_INCLUDEDIR ${RAPIDJSON_INCLUDE_DIRS})
    endif()

    # fix for macOS if most recent version
    if(NOT RAPIDJSON_INCLUDEDIR)
        set(RAPIDJSON_INCLUDEDIR ${RapidJSON_INCLUDE_DIRS})
    endif()


    add_library(RapidJSON::RapidJSON INTERFACE IMPORTED)
    set_target_properties(RapidJSON::RapidJSON PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${RAPIDJSON_INCLUDEDIR}"
    )
    message(STATUS "Found rapidjson ${RAPIDJSON_INCLUDEDIR}")
    message(STATUS "Set rapidjson path successully: ${RAPIDJSON_INCLUDEDIR}")
endif()


# ###############################
# ####   Python support
# ###############################

# set(Python_ADDITIONAL_VERSIONS 3.5 3.6 3.8)
find_package(Python3 COMPONENTS Interpreter Development)

if(Python3_Interpreter_FOUND)
    message(STATUS "Python3_INCLUDE_DIRS: ${Python3_INCLUDE_DIRS}")
    message(STATUS "Python3_LIBRARIES: ${Python3_LIBRARIES}")
    message(STATUS "PYTHON_MODULE_PREFIX: ${PYTHON_MODULE_PREFIX}")
    message(STATUS "PYTHON_MODULE_EXTENSION: ${PYTHON_MODULE_EXTENSION}")
elseif(NOT Python3_Interpreter_FOUND)
    set(Missing_package "TRUE")

    if(APPLE AND CMAKE_HOST_APPLE)
        message(STATUS "To install python3 on MacOS using homebrew run following command:")
        message(STATUS "    brew install python3")
    endif(APPLE AND CMAKE_HOST_APPLE)
endif(Python3_Interpreter_FOUND)


# ###############################
# ####   pybind11
# ###############################

# Need Version 2.2.4 Not available in ubuntu bionic
find_package(pybind11 2.7 CONFIG)

if(${pybind11_FOUND})
    message(VERBOSE "Found pybind11 v${pybind11_VERSION}: ${pybind11_INCLUDE_DIRS}")
    message(VERBOSE "Found pybind11 >= 2.7")
elseif(USE_VENDORED_PYBIND11)
    message(STATUS "pybind11 >= 2.7 not found, will build our provided version")
    add_subdirectory(deps/pybind11)
else()
    message(FATAL_ERROR "pybind11 >= 2.7 not found and USE_VENDORED_PYBIND11 is OFF")
endif()

# ###############################
# ####   spdlog
# ###############################

if(USE_VENDORED_SPDLOG)
    message(STATUS "using spdlog from deps")
    set(spdlog_VERSION 1.9.2)
    add_library(spdlog::spdlog INTERFACE IMPORTED)
    set_target_properties(spdlog::spdlog PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/deps/spdlog-${spdlog_VERSION}/include"
    )
else()
    find_package(spdlog REQUIRED)
    if(spdlog_FOUND)
        get_target_property(SPDLOG_HEADERS_DIR spdlog::spdlog INTERFACE_INCLUDE_DIRECTORIES)
        message(STATUS "Using system's spdlog headers at ${SPDLOG_HEADERS_DIR}")
    else()
        message(FATAL_ERROR "spdlog was not found and USE_VENDORED_SPDLOG is OFF")
    endif()
endif()

# ###############################
# ####   subprocess
# ###############################
message(STATUS "using subprocess from deps")
add_library(subprocess::subprocess INTERFACE IMPORTED)
set_target_properties(subprocess::subprocess PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/deps/subprocess"
)

# ###############################
# ####   nlohmann_json
# ###############################
message(STATUS "using nlohmann_json from deps")
add_library(nlohmann_json::nlohmann_json INTERFACE IMPORTED)
set_target_properties(nlohmann_json::nlohmann_json PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/deps/nlohmann_json"
)

# ###############################
# ####   Graphviz
# ###############################
find_package(Graphviz)

if(${graphviz_FOUND})
    add_library(graphviz::graphviz INTERFACE IMPORTED)
    set_target_properties(graphviz::graphviz PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${GRAPHVIZ_INCLUDE_DIR}
    )
    set_target_properties(graphviz::graphviz PROPERTIES
        INTERFACE_LINK_LIBRARIES ${GRAPHVIZ_LIBRARIES}
    )

    # Use graphviz via:
    # target_link_libraries(xxx PUBLIC ... graphviz::graphviz ... )
    # or in plugins:
    # add_custom_target( ...
    # LINK_LIBRARIES ... graphviz::graphviz)
endif()

# ###############################
# ####   Berkeley ABC
# ###############################

# abc stuff
# Download and unpack abc at configure time
add_library(ABC INTERFACE IMPORTED)
find_package(ABC)

if(${ABC_FOUND})
    message(STATUS "Found ABC:")
    message(STATUS "    ABC_LIBRARY: ${ABC_LIBRARY}")
else()
    message(STATUS "ABC not found")
    message(STATUS "Will build abc ourselves, check README.md to see how to speed up the process...")

    add_subdirectory(deps/abc)
    set(ABC_LIBRARY abc::libabc-pic)
    add_library(${ABC_LIBRARY} INTERFACE IMPORTED)
    set_target_properties(${ABC_LIBRARY} PROPERTIES INTERFACE_LINK_LIBRARIES libabc-pic)
    install(TARGETS libabc-pic LIBRARY DESTINATION ${LIBRARY_INSTALL_DIRECTORY})
endif()

# ###############################
# ####   z3
# ###############################
find_package(Z3 REQUIRED)

if(Z3_FOUND)
    message(STATUS "Found z3")
    message(STATUS "    Z3_LIBRARIES: ${Z3_LIBRARIES}")
    message(STATUS "    Z3_INCLUDE_DIRS: ${Z3_INCLUDE_DIRS}")
else()
    set(Missing_package "TRUE")
    message(STATUS "Could not find z3")
endif(Z3_FOUND)

# ###############################
# ####   igraph
# ###############################
if(USE_VENDORED_IGRAPH)
    set(IGRAPH_SUBDIR "${CMAKE_SOURCE_DIR}/deps/igraph-0.10.x")
    add_subdirectory(${IGRAPH_SUBDIR})
    get_directory_property(igraph_INCLUDES DIRECTORY ${IGRAPH_SUBDIR} DEFINITION IGRAPH_INCLUDES)
    get_directory_property(igraph_LIBRARIES      DIRECTORY ${IGRAPH_SUBDIR} DEFINITION IGRAPH_LIB)
    message(STATUS "Using igraph from ${IGRAPH_SUBDIR}")
else()
    find_package(igraph REQUIRED)
    get_target_property(igraph_LIBRARIES igraph::igraph IMPORTED_LOCATION_RELEASE)
    get_target_property(igraph_INCLUDES igraph::igraph INTERFACE_INCLUDE_DIRECTORIES)
    message(STATUS "Using system igraph ${igraph_VERSION} from ${igraph_LIBRARIES}")
endif()
