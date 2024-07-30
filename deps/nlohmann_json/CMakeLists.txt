add_library(nlohmann_json::nlohmann_json INTERFACE)
target_include_directories(nlohmann_json::nlohmann_json INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>")
