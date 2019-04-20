# Download and unpack pybind11 at configure time
configure_file(pybind11-CMakeLists.txt.in "${CMAKE_BINARY_DIR}/pybind11-download/CMakeLists.txt")
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/pybind11-download" )
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/pybind11-download" )

# Add pybind11 directly to our build. This adds
# the following targets: gtest, gtest_main, gmock
# and gmock_main
add_subdirectory("${CMAKE_BINARY_DIR}/pybind11-src"
                    "${CMAKE_BINARY_DIR}/pybind11-build")

# The pybind11 targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if(CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${PYBIND11_INCLUDE_DIR}")
endif()

# Now simply link your own targets against gtest, gmock,
# etc. as appropriate

# Download and unpack rapidjson at configure time
configure_file(rapidjson-CMakeLists.txt.in "${CMAKE_BINARY_DIR}/rapidjson-download/CMakeLists.txt")
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/rapidjson-download" )
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/rapidjson-download" )

# Download and unpack spdlog at configure time
configure_file(spdlog-CMakeLists.txt.in "${CMAKE_BINARY_DIR}/spdlog-download/CMakeLists.txt")
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/spdlog-download" )
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/spdlog-download" )