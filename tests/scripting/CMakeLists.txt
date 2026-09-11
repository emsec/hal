# --python-script has to behave the same without a GUI and inside it. The check runs the same script both ways
# against the FSM example project and compares what the script saw and how each run ended.
if(BUILD_TESTS AND (PL_GUI OR BUILD_ALL_PLUGINS) AND (PL_PYTHON_SHELL OR BUILD_ALL_PLUGINS))
    set(SCRIPTING_TEST_DIR ${CMAKE_BINARY_DIR}/tests/scripting)
    file(MAKE_DIRECTORY ${SCRIPTING_TEST_DIR})
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf ${CMAKE_SOURCE_DIR}/examples/fsm.zip
                    WORKING_DIRECTORY ${SCRIPTING_TEST_DIR}
                    RESULT_VARIABLE FSM_EXTRACT_RESULT)
    if(NOT FSM_EXTRACT_RESULT EQUAL 0)
        message(FATAL_ERROR "could not extract examples/fsm.zip for the scripting test")
    endif()

    add_test(NAME runTest-python_script_modes
             COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_script_modes.py
                     ${CMAKE_BINARY_DIR}/bin/hal ${SCRIPTING_TEST_DIR}/fsm ${SCRIPTING_TEST_DIR}/work)
    set_tests_properties(runTest-python_script_modes PROPERTIES
                         ENVIRONMENT "HAL_BASE_PATH=${CMAKE_BINARY_DIR};QT_QPA_PLATFORM=offscreen"
                         TIMEOUT 900)

    add_test(NAME runTest-remote_control
             COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_remote_control.py
                     ${CMAKE_BINARY_DIR}/bin/hal ${SCRIPTING_TEST_DIR}/fsm ${SCRIPTING_TEST_DIR}/work_remote)
    set_tests_properties(runTest-remote_control PROPERTIES
                         ENVIRONMENT "HAL_BASE_PATH=${CMAKE_BINARY_DIR};QT_QPA_PLATFORM=offscreen"
                         TIMEOUT 600)
endif()
