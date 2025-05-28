cmake_minimum_required(VERSION 3.5.0)
project(
  saleae_cli
  VERSION 0.0.1
  LANGUAGES CXX
  DESCRIPTION "CLI tool to browse SALEAE wavedata"
  )

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/plugins/simulator/netlist_simulator_controller/include)

find_package(Qt5 COMPONENTS Core REQUIRED )

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/plugins/simulator/netlist-simulator-controller/include)

add_executable(saleae saleae.cpp)

target_link_libraries(saleae netlist_simulator_controller hal::core hal::netlist)
