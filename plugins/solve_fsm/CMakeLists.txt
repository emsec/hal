option(PL_SOLVE_FSM "PL_SOLVE_FSM" ON)
if(PL_SOLVE_FSM OR BUILD_ALL_PLUGINS)
    file(GLOB_RECURSE SOLVE_FSM_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE SOLVE_FSM_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE SOLVE_FSM_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)

    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

    find_package(Z3 REQUIRED)
    if(Z3_FOUND)
        message(STATUS "Found z3")
    else()
        message(STATUS "Could not find z3")
    endif(Z3_FOUND)


    hal_add_plugin(solve_fsm
                   SHARED
                   HEADER ${SOLVE_FSM_INC}
                   SOURCES ${SOLVE_FSM_SRC} ${SOLVE_FSM_PYTHON_SRC}
                   LINK_LIBRARIES PUBLIC ${Z3_LIBRARIES}
                   )
endif()
