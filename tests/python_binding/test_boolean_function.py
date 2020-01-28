import sys
import os

### Automatically searches for the hal_py.so file and adds its path to the environment variables

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

###
import unittest
import hal_py
import logging
import copy

# Testing the binding of the boolean_function class

'''
NOTE: Every function is only tested once with all possible amounts of inputs,
      but NOT extensively (its already done in the c++ tests)
'''

class TestBooleanFunction(unittest.TestCase):
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

    def are_equal(self, bf_0, bf_1):
        if bf_0.get_variables() != bf_1.get_variables():
            print ("NOT EQUAL: Not the same variable set!")
            return False
        var_list = list(bf_0.get_variables())
        return bf_0.get_truth_table(var_list) == bf_1.get_truth_table(var_list)

    # ========= TEST CASES ==========

    # Test the boolean function class with all its functions
    def test_boolean_function(self):
        a = hal_py.boolean_function("A")
        b = hal_py.boolean_function("B")
        c = hal_py.boolean_function("C")
        _1 = hal_py.boolean_function(hal_py.value.ONE)

        # Get Variables
        self.assertEqual((a | b).get_variables(), {"A", "B"})

        # Get Truth Table
        self.assertEqual((a & b).get_truth_table(["A", "B"]), [hal_py.value.ZERO, hal_py.value.ZERO,
                                                             hal_py.value.ZERO, hal_py.value.ONE])

        # From String
        self.assertTrue(self.are_equal(hal_py.boolean_function.from_string("(A&(!B)) | ((!A)&B)", ["A", "B"]),
                                       hal_py.boolean_function.from_string("A ^ B", ["A", "B"])))


        # Constants
        self.assertTrue((a ^ a).is_constant_zero())
        self.assertFalse((a & a).is_constant_zero())
        self.assertTrue((a | _1).is_constant_one())
        self.assertFalse((a & a).is_constant_one())

        # Evaluate
        bf_input = { "A" : hal_py.value.ONE, "B" : hal_py.value.ZERO }
        self.assertEqual((a | b)(bf_input), hal_py.value.ONE)
        self.assertEqual((a & b)(bf_input), hal_py.value.ZERO)

        # DNF/Optimize
        self.assertFalse(((a | b) & c).is_dnf())
        self.assertTrue(((a & b) | (a & c)).is_dnf())
        self.assertTrue(((a | b) & c).to_dnf().is_dnf())
        self.assertTrue(self.are_equal(((a | b) & c).to_dnf(), ((a | b) & c)))
        self.assertTrue(self.are_equal(((a | b | b | b) & c).optimize(), ((a | b) & c)))

        # Substitute
        self.assertTrue(self.are_equal((a | b).substitute("B","C"), (a | c)))
        self.assertTrue(self.are_equal((a | b).substitute("B",(b | c)), (a | b | c)))

        # Is Empty
        self.assertTrue(hal_py.boolean_function().is_empty())
        self.assertFalse((a | b).is_empty())

        # Assign Operators
        # Note: Copy constructor?
        tmp_bf = hal_py.boolean_function("A")
        tmp_bf &= b
        tmp_bf |= c
        tmp_bf ^= _1
        self.assertTrue(self.are_equal(tmp_bf, ((a & b) | c) ^ _1))

        # Comparison Operators
        self.assertTrue((a & b) == (a & b))
        self.assertTrue((a & b) != (a | b))




if __name__ == '__main__':
    unittest.main()
