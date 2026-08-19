# The binding lifetime check reads the binding sources rather than running them, so it needs no
# build artefacts and no netlist, only a Python interpreter.
add_test(NAME runTest-python_binding_lifetimes
         COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/check_binding_lifetimes.py
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
