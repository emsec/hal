#include "netlist_test_utils.h"
#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/netlist.h"
#include "gtest/gtest.h"
#include <iostream>
#include <sstream>
#include <experimental/filesystem>


using namespace test_utils;

class gate_library_parser_liberty_test : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        NO_COUT_BLOCK;
        gate_library_manager::load_all();
    }

    virtual void TearDown()
    {
    }
};

/**
 * Testing the correct usage of the vhdl parser by parse a small vhdl-format string, which describes the netlist
 * shown above.
 *
 * Functions: parse
 */
TEST_F(gate_library_parser_liberty_test, check_main_example)
{
    TEST_START
        ASSERT_EQ(1,1);
    // IN PROGRESS ...
    TEST_END
}

