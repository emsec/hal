#include "netlist/netlist_internal_manager.h"
#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/netlist.h"
#include "netlist/netlist_factory.h"
#include "test_def.h"
#include "gtest/gtest.h"
#include <core/log.h>
#include <iostream>
#include <netlist/gate.h>
#include <netlist/net.h>

class netlist_internal_manager_test : public ::testing::Test
{
protected:
    const std::string g_lib_name = "XILINX_SIMPRIM";

    virtual void SetUp()
    {
        NO_COUT_BLOCK;
    }

    virtual void TearDown()
    {
    }

    // Creates an empty netlist with a certain id if passed
    std::shared_ptr<netlist> create_empty_netlist(int id = -1)
    {
        NO_COUT_BLOCK;
        std::shared_ptr<gate_library> gl = gate_library_manager::get_gate_library(g_lib_name);
        std::shared_ptr<netlist> nl(new netlist(gl));

        if (id >= 0)
        {
            nl->set_id(id);
        }
        return nl;
    }
};

/**
 * Testing things
 *
 * Functions: <functions>
 */
TEST_F(netlist_internal_manager_test, check_)
{
}

// NOTE: This file can be deleted