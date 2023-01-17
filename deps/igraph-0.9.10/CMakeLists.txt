include(ExternalProject)

set (IGRAPH_BASE      "${CMAKE_BINARY_DIR}/igraph")
set (IGRAPH_DOWNLOAD  "${IGRAPH_BASE}/download")
set (IGRAPH_BUILD     "${IGRAPH_BASE}/build")
set (IGRAPH_INCLUDES  "${IGRAPH_BASE}/include")
set (IGRAPH_LIB       "${IGRAPH_BASE}/lib/libigraph.so")
set (HAVE_IGRAPH TRUE)
  
message("IGRAPH version 0.9.10 : download and build in ${IGRAPH_BASE}")

ExternalProject_Add(igraph_0_9
  PREFIX "${IGRAPH_BASE}"
  URL "https://github.com/igraph/igraph/archive/refs/tags/0.9.10.tar.gz"
  SOURCE_DIR "${IGRAPH_DOWNLOAD}"
  CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${IGRAPH_BASE}" "-DBUILD_SHARED_LIBS=ON" "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}" "-DIGRAPH_VERSION=0.9.10"
  BINARY_DIR "${IGRAPH_BUILD}"
  PATCH_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/patch.sh ${CMAKE_CURRENT_SOURCE_DIR} ${IGRAPH_DOWNLOAD}
  INSTALL_DIR "${IGRAPH_BASE}"
  )

add_library ( igraph::igraph STATIC IMPORTED GLOBAL )

set_target_properties ( igraph::igraph PROPERTIES
  IMPORTED_LOCATION_DEBUG ${IGRAPH_LIB}
  IMPORTED_LOCATION_RELEASE ${IGRAPH_LIB}
#  INTERFACE_INCLUDE_DIRECTORIES ${IGRAPH_INCLUDES}
)

mark_as_advanced (
  HAVE_IGRAPH
  IGRAPH_LIB
  IGRAPH_INCLUDES
  )
  
