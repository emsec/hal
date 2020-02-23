find_package(PkgConfig)
pkg_check_modules(PC_LIBCGRAPH libcgraph)
set(GRAPHVIZ_DEFINITIONS ${PC_LIBCGRAPH_CFLAGS_OTHER})
pkg_check_modules(PC_LIBCDT libcdt)
set(GRAPHVIZ_DEFINITIONS ${PC_LIBCDT_CFLAGS_OTHER})
pkg_check_modules(PC_LIBGVC libgvc)
set(GRAPHVIZ_DEFINITIONS ${PC_LIBGVC_CFLAGS_OTHER})
pkg_check_modules(PC_LIBPATHPLAN libpathplan)
set(GRAPHVIZ_DEFINITIONS ${PC_LIBPATHPLAN_CFLAGS_OTHER})
find_path(GRAPHVIZ_INCLUDE_DIR     NAMES cgraph.h
          PATHS
          ${PC_LIBCGRAPH_INCLUDEDIR}
          ${PC_LIBCGRAPH_INCLUDE_DIRS}
          )
find_library(GRAPHVIZ_CDT_LIBRARY      NAMES cdt
             PATHS
             ${PC_LIBCDT_LIBDIR}
             ${PC_LIBCDT_LIBRARY_DIRS}
             )
find_library(GRAPHVIZ_GVC_LIBRARY      NAMES gvc
             PATHS
             ${PC_LIBGVC_LIBDIR}
             ${PC_LIBGVC_LIBRARY_DIRS}
             )
find_library(GRAPHVIZ_CGRAPH_LIBRARY   NAMES cgraph
             PATHS
             ${PC_LIBCGRAPH_LIBDIR}
             ${PC_LIBCGRAPH_LIBRARY_DIRS}
             )
find_library(GRAPHVIZ_PATHPLAN_LIBRARY NAMES pathplan
             PATHS
             ${PC_LIBPATHPLAN_LIBDIR}
             ${PC_LIBPATHPLAN_LIBRARY_DIRS}
             )

set(GRAPHVIZ_LIBRARIES ${GRAPHVIZ_CDT_LIBRARY} ${GRAPHVIZ_GVC_LIBRARY} ${GRAPHVIZ_CGRAPH_LIBRARY} ${GRAPHVIZ_PATHPLAN_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Graphviz DEFAULT_MSG GRAPHVIZ_LIBRARIES GRAPHVIZ_INCLUDE_DIR)

mark_as_advanced(
        GRAPHVIZ_INCLUDE_DIR
        GRAPHVIZ_CDT_LIBRARY
        GRAPHVIZ_GVC_LIBRARY
        GRAPHVIZ_CGRAPH_LIBRARY
        GRAPHVIZ_PATHPLAN_LIBRARY
)