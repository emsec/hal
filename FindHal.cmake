find_package(PkgConfig)
pkg_check_modules(PC_LIBCGRAPH hal)

# Include dir
find_path(Hal_INCLUDE_DIR
          NAMES hal/pragma_once.h
          PATHS ${hal_PKGCONF_INCLUDE_DIRS}
          )


# Finally the library itself
find_library(hal_core_LIBRARY
             NAMES hal_core
             PATHS ${hal_PKGCONF_LIBRARY_DIRS}
             )

# Finally the library itself
find_library(hal_graph_LIBRARY
             NAMES hal_graph
             PATHS ${hal_PKGCONF_LIBRARY_DIRS}
             )

find_library(hal_hdl_parser_LIBRARY
             NAMES hal_hdl_parser
             PATHS ${hal_PKGCONF_LIBRARY_DIRS}
             )

find_library(hal_hdl_writer_LIBRARY
             NAMES hal_hdl_writer
             PATHS ${hal_PKGCONF_LIBRARY_DIRS}
             )

set(HAL_LIBRARIES ${hal_core_LIBRARY} ${hal_graph_LIBRARY} ${hal_hdl_parser_LIBRARY} ${hal_hdl_writer_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Hal DEFAULT_MSG HAL_LIBRARIES Hal_INCLUDE_DIR)

mark_as_advanced(
        Hal_INCLUDE_DIR
        hal_core_LIBRARY
        hal_graph_LIBRARY
        hal_hdl_parser_LIBRARY
        hal_hdl_writer_LIBRARY
)