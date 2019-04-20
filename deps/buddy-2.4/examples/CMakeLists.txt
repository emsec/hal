cmake_minimum_required(VERSION 2.8)

add_subdirectory(adder)

if(NOT WIN32)
add_subdirectory(bddcalc)
endif()

add_subdirectory(bddtest)
add_subdirectory(cmilner)
add_subdirectory(fdd)
add_subdirectory(milner)
add_subdirectory(money)
add_subdirectory(queen)

if(NOT WIN32)
add_subdirectory(solitare)
endif()
