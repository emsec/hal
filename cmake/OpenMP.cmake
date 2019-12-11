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
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}" PARENT_SCOPE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" PARENT_SCOPE)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}" PARENT_SCOPE)
    endif()
endif()