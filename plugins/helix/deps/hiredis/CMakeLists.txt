include(ExternalProject)

set(HIREDIS_INSTALL "${CMAKE_BINARY_DIR}")
set(HIREDIS_BASE "${CMAKE_BINARY_DIR}/hiredis")
set(HIREDIS_DOWNLOAD "${HIREDIS_BASE}/download")
set(HIREDIS_BUILD "${HIREDIS_BASE}/build")
set(HIREDIS_INCLUDES "${HIREDIS_BASE}/include")
set(HIREDIS_LIB "${HIREDIS_BASE}/lib/libhiredis${CMAKE_SHARED_LIBRARY_SUFFIX}")
set(HAVE_HIREDIS TRUE)

get_github_latest_release_tag(redis hiredis HIREDIS_TAG)

message(
  "-- Using hiredis-${HIREDIS_TAG}"
)

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

ExternalProject_Add(
  hiredis_src
  PREFIX hiredis
  GIT_REPOSITORY https://github.com/redis/hiredis
  GIT_TAG ${HIREDIS_TAG}
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/hiredis
             -DBUILD_SHARED_LIBS=ON)

file(MAKE_DIRECTORY ${HIREDIS_INCLUDES})

add_library(hiredis SHARED IMPORTED GLOBAL)

set_target_properties(
  hiredis
  PROPERTIES IMPORTED_LOCATION_DEBUG ${HIREDIS_LIB}
             IMPORTED_LOCATION_RELEASE ${HIREDIS_LIB}
             INTERFACE_INCLUDE_DIRECTORIES ${HIREDIS_INCLUDES})

add_dependencies(hiredis hiredis_src)

mark_as_advanced(HAVE_HIREDIS HIREDIS_LIB HIREDIS_INCLUDES)
