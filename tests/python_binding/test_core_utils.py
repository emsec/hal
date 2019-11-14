import unittest
import hal_py
import logging

'''
NOTE: Every function is only tested once with all possible amounts of inputs,
      but NOT extensively (Because its already done in the c++ tests)
'''

class TestCoreUtils(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig()

    @classmethod
    def tearDownClass(cls):
        pass

    def setUp(self):
        self.log = logging.getLogger("LOG")
        self.cu = hal_py.core_utils

    def tearDown(self):
        pass

    ######## TEST CASES ########

    # Testing the python binding for function: <name>
    def test_template(self):
        self.assertIsNotNone(self.cu)

    # Testing the python binding for function: get_binary_directory()
    def test_get_binary_directory(self):
        self.assertNotEqual(self.cu.get_binary_directory(), "")

    # Testing the python binding for function: get_base_directory()
    def test_get_base_directory(self):
        self.assertNotEqual(self.cu.get_base_directory(), "")

    # Testing the python binding for function: get_library_directory()
    def test_get_library_directory(self):
        self.assertNotEqual(self.cu.get_library_directory(), "")

    # Testing the python binding for function: get_share_directory()
    def test_get_share_directory(self):
        self.assertNotEqual(self.cu.get_share_directory(), "")

    # Testing the python binding for function: get_user_share_directory()
    def test_get_user_share_directory(self):
        self.assertNotEqual(self.cu.get_user_share_directory(), "")

    # Testing the python binding for function: get_user_config_directory()
    def test_get_user_config_directory(self):
        self.assertNotEqual(self.cu.get_user_config_directory(), "")

    # Testing the python binding for function: get_default_log_directory()
    def test_get_default_log_directory(self):
        self.assertNotEqual(self.cu.get_default_log_directory(), "")

    # Testing the python binding for function: get_gate_library_directories()
    def test_get_gate_library_directories(self):
        dir_list = self.cu.get_gate_library_directories()
        self.assertNotEqual(len(dir_list), 0)
        for d in dir_list:
            self.assertNotEqual(d, "")

    # IN PROGRESS: others

if __name__ == '__main__':
    unittest.main()
