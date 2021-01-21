# - Try to find Z3
# Once done this will define
#  Z3_FOUND - System has Z3
#  Z3_INCLUDE_DIRS - The Z3 include directories
#  Z3_LIBRARIES - The libraries needed to use Z3


find_path (Z3_INCLUDE_DIRS z3++.h
  PATHS ${CMAKE_EXTRA_INCLUDES} PATH_SUFFIXES z3/ z3/include NO_DEFAULT_PATH
  )
if(NOT Z3_INCLUDE_DIRS)
    find_path (Z3_INCLUDE_DIRS z3++.h
      PATHS /usr/local/include /usr/include /include /sw/include /usr/lib /usr/lib64 /usr/lib/x86_64-linux-gnu/ ${CMAKE_EXTRA_INCLUDES} PATH_SUFFIXES z3/ z3/include
      )
endif(NOT Z3_INCLUDE_DIRS)

## -----------------------------------------------------------------------------
## Check for the library

find_library (Z3_LIBRARIES NAMES z3
  PATHS ${CMAKE_EXTRA_LIBRARIES} PATH_SUFFIXES z3/ NO_DEFAULT_PATH
  )
if(NOT Z3_LIBRARIES)
    find_library (Z3_LIBRARIES NAMES z3
      PATHS /usr/local/lib /usr/lib /lib /sw/lib ${CMAKE_EXTRA_LIBRARIES} PATH_SUFFIXES z3/
      )
endif(NOT Z3_LIBRARIES)


if (Z3_INCLUDE_DIRS AND Z3_LIBRARIES)
  set (Z3_FOUND TRUE)
endif()
