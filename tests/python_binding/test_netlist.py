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
        #help(hal_py.netlist)

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

    # Testing the python binding for functions: create_gate, get_gates, delete_gate, mark_global_gnd_gate,
    #  mark_global_vcc_gate, unmark_global_vcc_gate, unmark_global_vcc_gate, is_global_gnd_gate, is_global_vcc_gate
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

        # Mark and unmark a global gnd_gate 
        nl.mark_global_gnd_gate(test_gate)
        self.assertTrue(nl.is_global_gnd_gate(test_gate))
        nl.unmark_global_gnd_gate(test_gate)
        self.assertFalse(nl.is_global_gnd_gate(test_gate))

        # Mark and unmark a global vcc_gate 
        nl.mark_global_vcc_gate(test_gate)
        self.assertTrue(nl.is_global_vcc_gate(test_gate))
        nl.unmark_global_vcc_gate(test_gate)
        self.assertFalse(nl.is_global_vcc_gate(test_gate))

        # Delete both gates
        nl.delete_gate(test_gate)
        nl.delete_gate(test_gate_no_id)
        self.assertEqual(len(nl.get_gates("INV", "test_gate")), 0)
        self.assertEqual(len(nl.get_gates("INV", "test_gate_no_id")), 0)
        


    # Testing the python binding for functions: create_module, get_modules, delete modules 
    def test_create_module(self):
        # NOTE: float x, float y ?
        # Create a module (with id)
        nl = self.create_empty_netlist()
        test_module = nl.create_module(self.min_id, "test_module", nl.get_top_module())
        self.assertIsNotNone(test_module)

        # Create a module (without (passed) id)
        test_module_no_id = nl.create_module("test_module_no_id", nl.get_top_module())
        self.assertIsNotNone(test_module_no_id)
        self.assertIsNotNone(self.get_module_by_name(nl.get_modules(), "test_module_no_id"))

        # Delete both modules
        nl.delete_module(test_module)
        nl.delete_module(test_module_no_id)
        self.assertIsNone(self.get_module_by_name(nl.get_modules(), "test_module_no_id"))
        self.assertIsNone(self.get_module_by_name(nl.get_modules(), "test_module"))

    # Testing the python binding for functions: create_net, get_nets, delete_net, mark_global_input_net, mark_global_output_net,
    #                                           unmark_global_input_net, unmark_global_output_net, is_global_input_net, is_global_output_net
    def test_create_net(self):
        nl = self.create_empty_netlist()
        default_net_amt = len(nl.get_nets())

        # Create a net
        test_net = nl.create_net(self.min_id, "test_net")
        self.assertIsNotNone(test_net)
        self.assertIsNotNone(nl.get_net_by_id(self.min_id))
        self.assertEqual(len(nl.get_nets()), default_net_amt+1)

        # Mark and unmark a global_input_net
        nl.mark_global_input_net(test_net)
        self.assertTrue(nl.is_global_input_net(test_net))
        nl.unmark_global_input_net(test_net)
        self.assertFalse(nl.is_global_input_net(test_net))

        # Mark and unmark a global_output_net
        nl.mark_global_output_net(test_net)
        self.assertTrue(nl.is_global_output_net(test_net))
        nl.unmark_global_output_net(test_net)
        self.assertFalse(nl.is_global_output_net(test_net))

        # Delete the net
        nl.delete_net(test_net)
        self.assertEqual(len(nl.get_nets()), default_net_amt)

    


if __name__ == '__main__':
    unittest.main()
