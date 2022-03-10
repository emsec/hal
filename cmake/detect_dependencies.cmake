################################
#####   PkgConfig
################################

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

################################
#####   Sanitizers
################################

find_package(Sanitizers REQUIRED)

################################
#####   OpenMP
################################

# See cmake/detect_dependencies.cmake
if(APPLE AND CMAKE_HOST_APPLE)
    find_package(OpenMP)

    if(OPENMP_FOUND OR OPENMP_CXX_FOUND)
        message(VERBOSE "Found libomp without any special flags")
    endif()

    # If OpenMP wasn't found, try if we can find it in the default Macports location
    if((NOT OPENMP_FOUND) AND (NOT OPENMP_CXX_FOUND) AND EXISTS "/opt/local/lib/libomp/libomp.dylib") # older cmake uses OPENMP_FOUND, newer cmake also sets OPENMP_CXX_FOUND, homebrew installations seem only to get the latter set.
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
    if((NOT OPENMP_FOUND) AND (NOT OPENMP_CXX_FOUND) AND EXISTS "/usr/local/opt/libomp/lib/libomp.dylib")
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
    if((NOT OPENMP_FOUND) AND (NOT OPENMP_CXX_FOUND))
        if((APPLE AND ((CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang") OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")))
            AND ((CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "7.0") AND (CMAKE_VERSION VERSION_LESS "3.12.0")))
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

################################
#####   Filesystem
################################

find_package(Filesystem REQUIRED Final Experimental)


################################
#####   RapidJSON
################################

find_package(RapidJSON REQUIRED)
message(VERBOSE "Found rapidjson ${RAPIDJSON_INCLUDEDIR}")
if(RapidJSON_FOUND AND NOT TARGET RapidJSON::RapidJSON)
    add_library(RapidJSON::RapidJSON INTERFACE IMPORTED)
    set_target_properties(RapidJSON::RapidJSON PROPERTIES
                          INTERFACE_INCLUDE_DIRECTORIES "${RAPIDJSON_INCLUDE_DIRS}"
                          )
endif()

################################
#####   pybind11
################################

# Need Version 2.2.4 Not available in ubuntu bionic
find_package(pybind11 2.4.3 CONFIG)
if(${pybind11_FOUND})
    message(VERBOSE "Found pybind11 v${pybind11_VERSION}: ${pybind11_INCLUDE_DIRS}")
    message(VERBOSE "Found pybind11 >= 2.4.3")
else()
    message(STATUS "pybind11 >= 2.4.3 not found, will build our provided version")
    add_subdirectory(deps/pybind11)
endif()

################################
#####   spdlog
################################


message(STATUS "using spdlog from deps")
set(spdlog_VERSION 1.5.0)
add_library(spdlog::spdlog INTERFACE IMPORTED)
set_target_properties(spdlog::spdlog PROPERTIES
                        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/deps/spdlog-1.5.0/include"
                        )

################################
#####   Python support
################################

#set(Python_ADDITIONAL_VERSIONS 3.5 3.6 3.8)
find_package (Python3 COMPONENTS Interpreter Development)
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

################################
#####   Graphviz
################################
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
    #   target_link_libraries(xxx PUBLIC ... graphviz::graphviz ... )
    # or in plugins:
    #   add_custom_target( ...
    #                      LINK_LIBRARIES ... graphviz::graphviz)
endif()

################################
#####   Berkeley ABC
################################

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
    add_library(abc::libabc-pic INTERFACE IMPORTED)
    set_target_properties(abc::libabc-pic PROPERTIES INTERFACE_LINK_LIBRARIES libabc-pic)
    set(ABC_LIBRARY abc::libabc-pic)
endif()


################################
#####   z3
################################

find_package(Z3 REQUIRED)
if(Z3_FOUND)
    message(STATUS "Found z3")
    message(STATUS "    Z3_LIBRARIES: ${Z3_LIBRARIES}")
    message(STATUS "    Z3_INCLUDE_DIRS: ${Z3_INCLUDE_DIRS}")
else()
    set(Missing_package "TRUE")
    message(STATUS "Could not find z3")
endif(Z3_FOUND)