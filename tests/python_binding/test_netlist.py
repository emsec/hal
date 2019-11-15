import unittest
import hal_py
import logging

'''
NOTE: Every function is only tested once with all possible amounts of inputs,
      but NOT extensively (its already done in the c++ tests)
'''

class TestCoreUtils(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig()
        #help(hal_py.netlist_factory)

    @classmethod
    def tearDownClass(cls):
        pass

    def setUp(self):
        # Minimal ID for all: netlists, gates, nets, modules, ...
        self.min_id = 3 # <- just to be sure ;)
        self.log = logging.getLogger("LOG")
        self.netlist = hal_py.netlist
        self.g_lib_name = "EXAMPLE_LIB"

    def tearDown(self):
        pass

    # ====== HELPER FUNCTIONS =======
    def create_empty_netlist(self):
        nl = hal_py.netlist_factory.create_netlist(self.g_lib_name)
        return nl

    # Gets the first module with the name <name> from a module set <module_set>
    def get_module_by_name (self, module_set, name):
        for m in module_set:
            if m.get_name() == name:
                return m
        return None

    # ========= TEST CASES ==========

    # Testing the python binding for function: <name>
    def test_template(self):
        nl = self.create_empty_netlist()
        nl.set_design_name("test_name")
        self.assertEqual(nl.get_design_name(), "test_name")

    # Testing the python binding for functions: create_gate, get_gates
    def test_create_gate(self):
        # NOTE: float x, float y ?
        # Create a gate (with id)
        nl = self.create_empty_netlist()
        test_gate = nl.create_gate(self.min_id, "INV", "test_gate")
        self.assertIsNotNone(test_gate)
        self.assertEqual(len(nl.get_gates("INV", "test_gate")), 1)
        self.assertEqual(test_gate.get_id(), self.min_id)
        # Create a gate (without (passed) id)
        test_gate_no_id = nl.create_gate("INV", "test_gate_no_id")
        self.assertIsNotNone(test_gate_no_id)
        self.assertEqual(len(nl.get_gates("INV", "test_gate_no_id")), 1)

    # Testing the python binding for functions: create_module, get_modules
    def test_create_module(self):
        # NOTE: float x, float y ?
        # Create a module (with id)
        nl = self.create_empty_netlist()
        test_module = nl.create_module(self.min_id, "test_module", nl.get_top_module())
        self.assertIsNotNone(test_module)
        self.assertIsNotNone(self.get_module_by_name(nl.get_modules(), "test_module"))
        self.assertEqual(test_module.get_id(), self.min_id)

        # Create a module (without (passed) id)
        test_module_no_id = nl.create_module("test_module_no_id", nl.get_top_module())
        self.assertIsNotNone(test_module_no_id)
        self.assertIsNotNone(self.get_module_by_name(nl.get_modules(), "test_module_no_id"))

    # TO BE CONTINUED...


if __name__ == '__main__':
    unittest.main()
