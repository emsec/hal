# Find ABC
# ABC_FOUND - system has ABC lib
# ABC_LIBRARY - Libraries needed to use ABC

# check the path provided to cmake
if (ABC_PATH)
    find_library(ABC_LIBRARY NAMES libabc.so PATHS ${ABC_PATH})
endif(ABC_PATH)

# if abc not found, lets check ourselves
if (ABC_LIBRARY)
  message(STATUS "Used provided ABC path")
else()
  # lets try to find libabc.so
  find_library(ABC_LIBRARY NAMES libabc.so PATHS ${CMAKE_EXTRA_LIBRARIES} PATH_SUFFIXES abc/ NO_DEFAULT_PATH)
  # try some additional paths
  if(NOT ABC_LIBRARY)
    find_library (ABC_LIBRARY NAMES libabc.so PATHS /usr/local/lib /usr/lib /lib /sw/lib  ${CMAKE_EXTRA_LIBRARIES} PATH_SUFFIXES abc/)
  endif(NOT ABC_LIBRARY)
  # lets download abc ourselves
  if(NOT ABC_LIBRARY)
    message(STATUS "Downloading and building abc. This will take a while, check README.md to see how to speed up the process...")
    configure_file(cmake/abc-CMakeLists.txt.in "${CMAKE_BINARY_DIR}/abc-download/CMakeLists.txt")
    execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
                    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/abc-download" )
    execute_process(COMMAND "${CMAKE_COMMAND}" --build .
                    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/abc-download" )

    add_subdirectory("${CMAKE_BINARY_DIR}/abc-src")

    set(ABC_LIBRARY ${CMAKE_BINARY_DIR}/abc-src/libabc.so)
  endif()
endif()


mark_as_advanced(ABC_LIBRARY)
if(ABC_LIBRARY)
  set(ABC_FOUND true)
endif()