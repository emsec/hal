add_library(subprocess::subprocess INTERFACE)
target_include_directories(subprocess::subprocess INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>")
