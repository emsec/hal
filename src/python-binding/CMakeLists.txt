set(PYTHON_BINDING_LIB hal_py)
set(PYTHON_PLUGIN_LIB hal_python)

include_directories(${include_directories}
                    ${CMAKE_SOURCE_DIR}/include
                    SYSTEM ${Boost_INCLUDE_DIR}
                    SYSTEM ${PYBIND11_INCLUDE_DIR}
                    SYSTEM ${PYTHON_INCLUDE_DIRS})

file(GLOB_RECURSE PYTHON_BINDING_LIB_SRC ${CMAKE_SOURCE_DIR}/src/python-binding/python_definitions.cpp)

add_library(${PYTHON_BINDING_LIB} SHARED ${PYTHON_BINDING_LIB_SRC})
#Set shared library suffix for MacOS; Used for boost_python support
set_target_properties(${PYTHON_BINDING_LIB} PROPERTIES PREFIX "")
if(APPLE AND CMAKE_HOST_APPLE)
    set_target_properties(${PYTHON_BINDING_LIB} PROPERTIES SUFFIX ".so")
endif(APPLE AND CMAKE_HOST_APPLE)
target_link_libraries(${PYTHON_BINDING_LIB} PUBLIC hal::core hal::netlist ${PYTHON_LIBRARIES} pybind11::module)
install(TARGETS ${PYTHON_BINDING_LIB}
        EXPORT hal
        LIBRARY DESTINATION ${LIBRARY_INSTALL_DIRECTORY}
        PERMISSIONS
        OWNER_READ
        OWNER_WRITE
        OWNER_EXECUTE
        GROUP_READ
        GROUP_EXECUTE
        WORLD_READ
        WORLD_EXECUTE
        INCLUDES DESTINATION ${INCLUDE_INSTALL_DIRECTORY})
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_sanitizers(${PYTHON_BINDING_LIB})
endif()

file(GLOB_RECURSE PYTHON_PLUGIN_LIB_HDR ${CMAKE_SOURCE_DIR}/include/python-binding/*.h)
file(GLOB_RECURSE PYTHON_PLUGIN_LIB_SRC ${CMAKE_SOURCE_DIR}/src/python-binding/factory_python_shell.cpp ${CMAKE_SOURCE_DIR}/src/python-binding/plugin_python_shell.cpp)

add_library(${PYTHON_PLUGIN_LIB} SHARED ${PYTHON_PLUGIN_LIB_HDR} ${PYTHON_PLUGIN_LIB_SRC})
set_target_properties(${PYTHON_PLUGIN_LIB} PROPERTIES DEFINE_SYMBOL API_PYTHON_PLUGIN)
target_link_libraries(${PYTHON_PLUGIN_LIB} PUBLIC hal::core hal::netlist PRIVATE ${PYTHON_LIBRARIES})
install(TARGETS ${PYTHON_PLUGIN_LIB}
        EXPORT hal
        LIBRARY DESTINATION ${LIBRARY_INSTALL_DIRECTORY}
        PERMISSIONS
        OWNER_READ
        OWNER_WRITE
        OWNER_EXECUTE
        GROUP_READ
        GROUP_EXECUTE
        WORLD_READ
        WORLD_EXECUTE
        INCLUDES DESTINATION ${INCLUDE_INSTALL_DIRECTORY})
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_sanitizers(${PYTHON_PLUGIN_LIB})
endif()