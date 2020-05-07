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
    print("Can't find hal_py.so instance. Did you build it? If it can't be found anyway, make sure its directory is added to your PYTHONPATH and your HAL_BASE_PATH to your environment variables.")


import unittest
import hal_py
import logging

'''
NOTE: Every function is only tested once with all possible amounts of inputs,
      but NOT extensively (its already done in the c++ tests)
'''


class TestDataContainer(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig()

    @classmethod
    def tearDownClass(cls):
        pass

    def setUp(self):
        # Minimal ID for all: netlists, gates, nets, modules, ...
        self.min_id = 3 # <- just to be sure ;)
        self.log = logging.getLogger("LOG")
        self.netlist = hal_py.netlist
        self.g_lib_name = "EXAMPLE_GATE_LIBRARY"

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

    # Testing the python binding for the data_container class
        nl = self.create_empty_netlist()
        test_gate = nl.create_net(self.min_id, "test_net")
        test_gate.set_data("category", "key", "data_type", "value")
        self.assertEqual(test_gate.get_data_by_key("category", "key"), ("data_type", "value"))

        test_gate.delete_data("category", "key")
        self.assertEqual(test_gate.get_data_by_key("category", "key"), ("", ""))


if __name__ == '__main__':
    unittest.main()
