import unittest
import hal_py
import logging


class MyTestCase(unittest.TestCase):
    def test_something(self):
        #this line is important
        logging.basicConfig()
        log = logging.getLogger("LOG")

        cu = hal_py.core_utils
        log.warning(cu.get_base_directory())
        self.assertIsNone(cu)


if __name__ == '__main__':
    unittest.main()
