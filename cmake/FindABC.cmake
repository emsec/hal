# Find ABC
# ABC_FOUND - system has ABC lib
# ABC_LIBRARY - Libraries needed to use ABC
# ABC_INCLUDE_DIR - the ABC include directory

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
    find_library (ABC_LIBRARY NAMES libabc.so PATHS /usr/local/lib /usr/lib /lib /sw/lib /opt/ ${CMAKE_EXTRA_LIBRARIES} PATH_SUFFIXES abc/)
  endif(NOT ABC_LIBRARY)
endif()


mark_as_advanced(ABC_LIBRARY)
if(ABC_LIBRARY)
  set(ABC_FOUND true)
endif()
