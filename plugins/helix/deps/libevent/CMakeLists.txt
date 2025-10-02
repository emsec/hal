include(ExternalProject)

set(LIBEVENT_INSTALL "${CMAKE_BINARY_DIR}")
set(LIBEVENT_BASE "${CMAKE_BINARY_DIR}/libevent")
set(LIBEVENT_DOWNLOAD "${LIBEVENT_BASE}/download")
set(LIBEVENT_BUILD "${LIBEVENT_BASE}/build")
set(LIBEVENT_INCLUDES "${LIBEVENT_BASE}/include")
set(LIBEVENT_LIB "${LIBEVENT_BASE}/lib/libevent${CMAKE_SHARED_LIBRARY_SUFFIX}")
set(HAVE_LIBEVENT TRUE)

get_github_latest_release_tag(libevent libevent LIBEVENT_TAG)

message(
  "-- Using libevent-${LIBEVENT_TAG}"
)

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

ExternalProject_Add(
  libevent_src
  PREFIX libevent
  GIT_REPOSITORY https://github.com/libevent/libevent
  GIT_TAG ${LIBEVENT_TAG}
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${LIBEVENT_BASE}
             -DBUILD_SHARED_LIBS=ON)

file(MAKE_DIRECTORY ${LIBEVENT_INCLUDES})

add_library(event SHARED IMPORTED GLOBAL)

set_target_properties(event PROPERTIES
  IMPORTED_LOCATION_DEBUG ${LIBEVENT_LIB}
  IMPORTED_LOCATION_RELEASE ${LIBEVENT_LIB}
  INTERFACE_INCLUDE_DIRECTORIES ${LIBEVENT_INCLUDES}
)

add_dependencies(event libevent_src)

mark_as_advanced(HAVE_LIBEVENT LIBEVENT_LIB LIBEVENT_INCLUDES)
