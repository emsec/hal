#include "netlist/module.h"
#include "netlist/event_system/gate_event_handler.h"
#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/netlist.h"
#include "netlist/netlist_factory.h"
#include "test_def.h"
#include "gtest/gtest.h"
#include <core/log.h>
#include <iostream>
#include <netlist/gate.h>
#include <netlist/net.h>

class module_test : public ::testing::Test
{
protected:
    const std::string g_lib_name = "EXAMPLE_GATE_LIBRARY";

    virtual void SetUp()
    {
        NO_COUT_BLOCK;
        gate_library_manager::load_all();
    }

    virtual void TearDown()
    {
    }

    // Creates an empty netlist with a certain id if passed
    std::shared_ptr<netlist> create_empty_netlist(const int id = -1)
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
 * Testing the access on the id, the type and the stored netlist after calling the constructor
 *
 * Functions: constructor, get_id, get_name
 */
TEST_F(module_test, check_constructor){TEST_START{// Creating a module of id 123 and type "test module"
                                                     std::shared_ptr<netlist> nl = create_empty_netlist(0);
std::shared_ptr<module> sm = nl->create_module(123, "test module", nl->get_top_module());

EXPECT_EQ(sm->get_id(), (u32)123);
EXPECT_EQ(sm->get_name(), "test module");
EXPECT_EQ(sm->get_netlist(), nl);
}
TEST_END
}

/**
 * Testing the addition of gates to the module. Verify the addition by call the
 * get_gates function and the contains_gate function
 *
 * Functions: create_gate
 */
TEST_F(module_test, check_create_gate){TEST_START{// Add some gates to the module
                                                     std::shared_ptr<netlist> nl = create_empty_netlist(0);
    std::shared_ptr<gate> gate_0 = nl->create_gate(1, "INV", "gate_0");
    std::shared_ptr<gate> gate_1 = nl->create_gate(2, "INV", "gate_1");
    // this gate is not part of the module
    std::shared_ptr<gate> gate_not_in_sm = nl->create_gate(3, "INV", "gate_not_in_sm");

    // Add gate_0 and gate_1 to a module
    std::shared_ptr<module> sm = nl->create_module(2, "test module", nl->get_top_module());
            sm->assign_gate(gate_0);
            sm->assign_gate(gate_1);

    std::set<std::shared_ptr<gate>> expRes = {gate_0, gate_1};

    EXPECT_EQ(sm->get_gates(), expRes);
    EXPECT_TRUE(sm->contains_gate(gate_0));
    EXPECT_TRUE(sm->contains_gate(gate_1));
    EXPECT_FALSE(sm->contains_gate(gate_not_in_sm));
}
{
    // Add the same gate twice to the module
    NO_COUT_TEST_BLOCK;
    std::shared_ptr<netlist> nl  = create_empty_netlist(0);
    std::shared_ptr<gate> gate_0 = nl->create_gate(1, "INV", "gate_0");

    // Add gate_0 twice
    std::shared_ptr<module> sm = nl->create_module(2, "test module", nl->get_top_module());
    sm->assign_gate(gate_0);
    sm->assign_gate(gate_0);

    std::set<std::shared_ptr<gate>> expRes = {
        gate_0,
    };

    EXPECT_EQ(sm->get_gates(), expRes);
    EXPECT_TRUE(sm->contains_gate(gate_0));
}

// NEGATIVE
{
    // Gate is a nullptr
    NO_COUT_TEST_BLOCK;
    std::shared_ptr<netlist> nl = create_empty_netlist(0);
    std::shared_ptr<module> sm = nl->create_module(2, "test module", nl->get_top_module());
    sm->assign_gate(nullptr);
    EXPECT_TRUE(sm->get_gates().empty());
}
TEST_END
}

/**
 * Testing the addition of nets to the module. Verify the addition by call the
 * get_nets function and the contains_net function
 *
 * Functions: create_net
 */
TEST_F(module_test, check_create_net){TEST_START{// Add some nets to the module
                                                    std::shared_ptr<netlist> nl = create_empty_netlist(0);
    std::shared_ptr<net> net_0 = nl->create_net(1, "net_0");
    std::shared_ptr<net> net_1 = nl->create_net(2, "net_1");
    // this net is not part of the module
    std::shared_ptr<net> net_not_in_sm = nl->create_net(3, "net_not_in_sm");

    // Add net_0 and net_1 to a module
    std::shared_ptr<module> sm = nl->create_module(2, "test module", nl->get_top_module());
            sm->assign_net(net_0);
            sm->assign_net(net_1);

    std::set<std::shared_ptr<net>> expRes = {net_0, net_1};

    EXPECT_EQ(sm->get_nets(), expRes);
    EXPECT_TRUE(sm->contains_net(net_0));
    EXPECT_TRUE(sm->contains_net(net_1));
    EXPECT_FALSE(sm->contains_net(net_not_in_sm));
}
{
    NO_COUT_TEST_BLOCK;
    // Add the same net twice to the module
    std::shared_ptr<netlist> nl = create_empty_netlist(0);
    std::shared_ptr<net> net_0  = nl->create_net(1, "net_0");

    // Add net_0 twice
    std::shared_ptr<module> sm = nl->create_module(2, "test module", nl->get_top_module());
    sm->assign_net(net_0);
    sm->assign_net(net_0);

    std::set<std::shared_ptr<net>> expRes = {
        net_0,
    };

    EXPECT_EQ(sm->get_nets(), expRes);
    EXPECT_TRUE(sm->contains_net(net_0));
}

// NEGATIVE
{
    // Net is a nullptr
    NO_COUT_TEST_BLOCK;
    std::shared_ptr<netlist> nl   = create_empty_netlist(0);
    std::shared_ptr<module> sm = nl->create_module(2, "test module", nl->get_top_module());
    sm->assign_net(nullptr);
    EXPECT_TRUE(sm->get_nets().empty());
}
/*{
            // net is not part of the same netlist TODO: Requirements?
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist(0);
            std::shared_ptr<netlist> other_nl = create_empty_netlist(1);
            std::shared_ptr<net> net_0(new net(other_nl, 0, "net_0"));
            other_nl->create_net(net_0);

            std::shared_ptr<module> sm = nl->create_module(2, "test module", nl->get_top_module());
            EXPECT_FALSE(sm->assign_net(net_0));
            EXPECT_TRUE(sm->get_nets().empty());
        }*/
TEST_END
}

/**
 * Testing the deletion of gates from module. Verify the deletion by call the
 * get_gates function
 *
 * Functions: delete_gate
 */
TEST_F(module_test, check_delete_gate){TEST_START{// Remove a gate from a module
    std::shared_ptr<netlist> nl = create_empty_netlist(0);
    std::shared_ptr<gate> gate_0 = nl->create_gate(1, "INV", "gate_0");

    std::shared_ptr<module> sm = nl->create_module(2, "test module", nl->get_top_module());
            sm->assign_gate(gate_0);

    EXPECT_TRUE(sm->remove_gate(gate_0));
    EXPECT_TRUE(sm->get_gates().empty());
}
{
    // Remove a gate which isn't part of the module
    NO_COUT_TEST_BLOCK;
    std::shared_ptr<netlist> nl  = create_empty_netlist(0);
    std::shared_ptr<gate> gate_0 = nl->create_gate(1, "INV", "gate_0");

    std::shared_ptr<module> sm = nl->create_module(2, "test module", nl->get_top_module());

    EXPECT_FALSE(sm->remove_gate(gate_0));
    EXPECT_TRUE(sm->get_gates().empty());
}
// NEGATIVE
{
    // Delete a nullptr
    std::shared_ptr<netlist> nl   = create_empty_netlist(0);
    std::shared_ptr<module> sm = nl->create_module(2, "test module", nl->get_top_module());
    EXPECT_FALSE(sm->remove_gate(nullptr));
    EXPECT_TRUE(sm->get_gates().empty());
}
TEST_END
}

/**
 * Testing the deletion of nets from module. Verify the deletion by call the
 * get_nets function
 *
 * Functions: delete_net
 */
TEST_F(module_test, check_delete_net){TEST_START{// Remove a net from a module
    std::shared_ptr<netlist> nl = create_empty_netlist(0);
    std::shared_ptr<net> net_0 = nl->create_net(1, "net_0");

    std::shared_ptr<module> sm = nl->create_module(2, "test module", nl->get_top_module());
            sm->assign_net(net_0);

    EXPECT_TRUE(sm->remove_net(net_0));
    EXPECT_TRUE(sm->get_nets().empty());
}
{
    // Remove a net which isn't part of the module
    NO_COUT_TEST_BLOCK;
    std::shared_ptr<netlist> nl = create_empty_netlist(0);
    std::shared_ptr<net> net_0  = nl->create_net(1, "net_0");

    std::shared_ptr<module> sm = nl->create_module(2, "test module", nl->get_top_module());

    EXPECT_FALSE(sm->remove_net(net_0));
    EXPECT_TRUE(sm->get_nets().empty());
}
// NEGATIVE
{
    // Delete a nullptr
    std::shared_ptr<netlist> nl   = create_empty_netlist(0);
    std::shared_ptr<module> sm = nl->create_module(2, "test module", nl->get_top_module());
    EXPECT_FALSE(sm->remove_net(nullptr));
    EXPECT_TRUE(sm->get_nets().empty());
}
TEST_END
}
