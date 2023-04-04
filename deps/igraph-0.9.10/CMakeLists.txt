include(ExternalProject)

set (IGRAPH_INSTALL   "${CMAKE_BINARY_DIR}")
set (IGRAPH_BASE      "${CMAKE_BINARY_DIR}/igraph")
set (IGRAPH_DOWNLOAD  "${IGRAPH_BASE}/download")
set (IGRAPH_BUILD     "${IGRAPH_BASE}/build")
set (IGRAPH_INCLUDES  "${IGRAPH_INSTALL}/include")
set (IGRAPH_LIB       "${IGRAPH_INSTALL}/lib/libigraph${CMAKE_SHARED_LIBRARY_SUFFIX}")
set (HAVE_IGRAPH TRUE)
  
message("-- IGRAPH version 0.9.10 : download and build libigraph${CMAKE_SHARED_LIBRARY_SUFFIX} in ${IGRAPH_BASE}")

ExternalProject_Add(igraph_0_9
  PREFIX "${IGRAPH_INSTALL}"
  URL "https://github.com/igraph/igraph/releases/download/0.9.10/igraph-0.9.10.tar.gz"
  SOURCE_DIR "${IGRAPH_DOWNLOAD}"
  CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${IGRAPH_INSTALL}" "-DBUILD_SHARED_LIBS=ON" "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}" "-Wno-deprecated" "-DIGRAPH_VERSION=0.9.10"
  BINARY_DIR "${IGRAPH_BUILD}"
  PATCH_COMMAND patch -p2 -u -i ${CMAKE_CURRENT_SOURCE_DIR}/patch.txt
  INSTALL_DIR "${IGRAPH_INSTALL}"
  BUILD_BYPRODUCTS "${IGRAPH_LIB}"
  )

add_library ( igraph::igraph STATIC IMPORTED GLOBAL )

set_target_properties ( igraph::igraph PROPERTIES
  IMPORTED_LOCATION_DEBUG ${IGRAPH_LIB}
  IMPORTED_LOCATION_RELEASE ${IGRAPH_LIB}
  INTERFACE_INCLUDE_DIRECTORIES ${IGRAPH_INCLUDES}
)

add_dependencies ( igraph::igraph igraph_0_9 )

mark_as_advanced (
  HAVE_IGRAPH
  IGRAPH_LIB
  IGRAPH_INCLUDES
  )
  
