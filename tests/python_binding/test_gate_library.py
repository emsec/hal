import sys
import os

# Automatically searches for the hal_py.so file and adds its path to the environment variables

dirname = os.path.dirname(__file__)
hal_path = os.path.join(dirname, '../../')
hal_py_path = None
hal_base_path = None

for p in os.listdir(hal_path):
    build_path = os.path.join(hal_path, p)
    if not os.path.isfile(build_path):
        if os.path.exists(os.path.join(hal_path, p + '/lib/hal_py.so')):
            hal_base_path = build_path
            hal_py_path = os.path.join(hal_path, p + '/lib/')
            break

if not hal_py_path is None:
    sys.path.append(hal_py_path)
    os.environ["HAL_BASE_PATH"] = str(build_path)
else:
    print("Can't find hal_py.so instance. Did you build it? If it can't be found anyway, make sure its directory is "
          "added to your PYTHONPATH and your HAL_BASE_PATH to your environment variables.")


import unittest
import hal_py
import logging
import copy

# Testing the binding of the the gate library, as well as the
# gate_type classes with its children (gate_type_lut, gate_type_sequential)

'''
NOTE: Every function is only tested once with all possible amounts of inputs,
      but NOT extensively (its already done in the c++ tests)
'''


class TestGateLibrary(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig()

    @classmethod
    def tearDownClass(cls):
        pass

    def setUp(self):
        self.log = logging.getLogger("LOG")

    def tearDown(self):
        pass

    # ====== HELPER FUNCTIONS =======

    # ========= TEST CASES ==========


# ---------- gate_type tests ----------

    # Testing the python binding for the gate_type class
    def test_gate_type(self):
        gt = hal_py.gate_type("TEST_GATE_TYPE")

        # Identifier
        self.assertEqual(gt.get_base_type(), hal_py.base_type.combinatorial)
        self.assertEqual(gt.get_name(), "TEST_GATE_TYPE")

        # Input Pins
        gt.add_input_pin("I0")
        gt.add_input_pins(["I1", "I2"])
        self.assertEqual(set(gt.get_input_pins()), {"I0", "I1", "I2"})

        # Output Pins
        gt.add_output_pin("O0")
        gt.add_output_pins(["O1", "O2"])
        self.assertEqual(set(gt.get_output_pins()), {"O0", "O1", "O2"})

        # Boolean Function
        bf_0 = hal_py.boolean_function.from_string("!I0 ^ I1 ^ I2", ["I0", "I1", "I2"])
        bf_1 = hal_py.boolean_function.from_string("I0 ^ !I1 ^ I2", ["I0", "I1", "I2"])
        bf_2 = hal_py.boolean_function.from_string("I0 ^ I1 ^ !I2", ["I0", "I1", "I2"])
        gt.add_boolean_function("O0", bf_0)
        gt.add_boolean_function("O1", bf_1)
        gt.add_boolean_function("O2", bf_2)
        self.assertEqual(gt.get_boolean_functions(), {"O0": bf_0, "O1": bf_1, "O2": bf_2})


# -------- gate_type_lut tests --------

    # Testing the python binding for the gate_type_lut class
    def test_gate_type_lut(self):
        gt = hal_py.gate_type_lut("TEST_GATE_TYPE")

        # Identifier
        self.assertEqual(gt.get_base_type(), hal_py.base_type.lut)

        # Output from Init String Pins
        gt.add_output_pins(["O1", "O2"])
        gt.add_output_from_init_string_pin("OFIS0")
        gt.add_output_from_init_string_pin("OFIS1")
        self.assertEqual(gt.get_output_from_init_string_pins(), {"OFIS0", "OFIS1"})

        # Config Data: Category
        gt.set_config_data_category("test_category")
        self.assertEqual(gt.get_config_data_category(), "test_category")

        # Config Data: Identifier
        gt.set_config_data_identifier("test_identifier")
        self.assertEqual(gt.get_config_data_identifier(), "test_identifier")

        # Config Data: Ascending Order
        gt.set_config_data_ascending_order(True)
        self.assertTrue(gt.is_config_data_ascending_order())
        gt.set_config_data_ascending_order(False)
        self.assertFalse(gt.is_config_data_ascending_order())


# ---- gate_type_sequential tests -----

    # Testing the python binding for the gate_type_sequential class
    def test_gate_type_sequential(self):
        gt = hal_py.gate_type_sequential("TEST_GATE_TYPE", hal_py.base_type.ff)

        # Identifier
        self.assertEqual(gt.get_base_type(), hal_py.base_type.ff)

        # Inverted State Output Pins
        gt.add_inverted_state_output_pin("invSOP_0")
        gt.add_inverted_state_output_pin("invSOP_1")
        self.assertEqual(gt.get_inverted_state_output_pins(), {"invSOP_0", "invSOP_1"})

        # State Output Pins
        gt.add_state_output_pin("SOP_0")
        gt.add_state_output_pin("SOP_1")
        self.assertEqual(gt.get_state_output_pins(), {"SOP_0", "SOP_1"})

        # Init Data Category
        gt.set_init_data_category("test_category")
        self.assertEqual(gt.get_init_data_category(), "test_category")

        # Init Data Identifier
        gt.set_init_data_identifier("test_identifier")
        self.assertEqual(gt.get_init_data_identifier(), "test_identifier")

        # Set-Reset Behaviour
        gt.set_set_reset_behavior(hal_py.set_reset_behavior.H, hal_py.set_reset_behavior.L)
        self.assertEqual(gt.get_set_reset_behavior(), (hal_py.set_reset_behavior.H, hal_py.set_reset_behavior.L))


# -------- gate_library tests ---------

    # Testing the python binding for the gate_library class
    def test_gate_library(self):
        gl = hal_py.gate_library("test_glib")

        # Identifier
        self.assertEqual(gl.get_name(), "test_glib")

        # Includes
        gl.add_include("test_include_0")
        gl.add_include("test_include_1")
        self.assertEqual(set(gl.get_includes()), {"test_include_0", "test_include_1"})

        # Gate Types
        # -- create some gate types
        gt_gnd = hal_py.gate_type("TEST_GND")
        gt_gnd.add_output_pin("O")
        _0 = hal_py.boolean_function(hal_py.value.ZERO)
        gt_gnd.add_boolean_function("O", _0)

        gt_vcc = hal_py.gate_type("TEST_VCC")
        gt_vcc.add_output_pin("O")
        _1 = hal_py.boolean_function(hal_py.value.ONE)
        gt_vcc.add_boolean_function("O", _1)

        # -- the actual tests
        gl.add_gate_types(gt_gnd)
        gl.add_gate_types(gt_vcc)
        self.assertEqual(gl.get_gate_types(), {"TEST_GND": gt_gnd, "TEST_VCC": gt_vcc})
        self.assertEqual(gl.get_gnd_gate_types(), {"TEST_GND": gt_gnd})
        self.assertEqual(gl.get_vcc_gate_types(), {"TEST_VCC": gt_vcc})


# -------------------------------------

if __name__ == '__main__':
    unittest.main()
