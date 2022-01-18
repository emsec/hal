# Find ABC
# ABC_FOUND - system has ABC lib
# ABC_LIBRARY - Libraries needed to use ABC
# ABC_INCLUDE_DIR - the ABC include directory

project(abc)
# check the path provided to cmake
if (ABC_PATH)
    find_library(ABC_LIBRARY NAMES libabc.so PATHS ${ABC_PATH})
    find_path(ABC_INCLUDE_DIR NAMES src/base/abc/abc.h PATHS ${ABC_PATH})
endif(ABC_PATH)

# if abc not found, lets check ourselves
if (ABC_LIBRARY AND ABC_INCLUDE_DIR)
  message(STATUS "Used provided ABC path")
else()
  # lets try to find libabc.so
  find_library(ABC_LIBRARY NAMES libabc.so PATHS ${CMAKE_EXTRA_LIBRARIES} PATH_SUFFIXES abc/ NO_DEFAULT_PATH)
  # try some additional paths
  if(NOT ABC_LIBRARY)
    find_library (ABC_LIBRARY NAMES libabc.so PATHS /usr/local/lib /usr/lib /lib /sw/lib /opt/ ${CMAKE_EXTRA_LIBRARIES} PATH_SUFFIXES abc/)
  endif(NOT ABC_LIBRARY)

  message(STATUS "${ABC_LIBRARY}")

  find_path(ABC_INCLUDE_DIR NAMES src/base/abc/abc.h PATHS ${CMAKE_EXTRA_INCLUDES} PATH_SUFFIXES abc)
  if(NOT ABC_INCLUDE_DIR)
    find_path (ABC_INCLUDE_DIR src/base/abc/abc.h
      PATHS /usr/local/include /usr/include /include /sw/include /usr/lib /usr/lib64 /usr/lib/x86_64-linux-gnu/ /opt/ ${CMAKE_EXTRA_INCLUDES} PATH_SUFFIXES abc/)
  endif(NOT ABC_INCLUDE_DIR)
  message(STATUS "${ABC_INCLUDE_DIR}")

endif()


mark_as_advanced(ABC_LIBRARY ABC_INCLUDE_DIR)
if(ABC_LIBRARY AND ABC_INCLUDE_DIR)
  set(ABC_FOUND true)
endif()