# The binding lifetime check reads the binding sources rather than running them, so it needs no
# build artefacts and no netlist, only a Python interpreter.
add_test(NAME runTest-python_binding_lifetimes
         COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/check_binding_lifetimes.py
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# The smoke test, unlike the check above, runs the bindings: it builds a small netlist and calls
# every binding reachable from it that takes no arguments. It therefore needs the built modules and
# the gate libraries, which it finds through HAL_BASE_PATH.
add_test(NAME runTest-python_binding_smoke
         COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/smoke_test_bindings.py
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
set_tests_properties(runTest-python_binding_smoke PROPERTIES
                     ENVIRONMENT "PYTHONPATH=${CMAKE_BINARY_DIR}/lib;HAL_BASE_PATH=${CMAKE_BINARY_DIR}")
