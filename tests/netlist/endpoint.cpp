#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/netlist.h"
#include "netlist/netlist_factory.h"
#include "netlist_test_utils.h"
#include "gtest/gtest.h"
#include <core/log.h>
#include <iostream>
#include <netlist/gate.h>
#include <netlist/net.h>

using namespace test_utils;

class endpoint_test : public ::testing::Test
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
 * Testing the get_gate and set_gate function
 *
 * Functions: get_gate, set_gate
 */
TEST_F(endpoint_test, check_set_get_gate)
{
    TEST_START
    std::shared_ptr<netlist> nl = create_empty_netlist(0);
    endpoint ep;
    std::shared_ptr<gate> test_gate = nl->create_gate(123, get_gate_type_by_name("INV"), "test_gate");
    ep.set_gate(test_gate);
    EXPECT_EQ(ep.get_gate(), test_gate);
    EXPECT_EQ(ep.gate, test_gate);
    TEST_END
}

/**
 * Testing the get_pin_type and set_pin_type function
 *
 * Functions: get_pin_type and set_pin_type
 */
TEST_F(endpoint_test, check_set_get_pin_type)
{
    TEST_START
    endpoint ep;
    ep.set_pin_type("PIN_TYPE");
    EXPECT_EQ(ep.get_pin_type(), "PIN_TYPE");
    EXPECT_EQ(ep.pin_type, "PIN_TYPE");
    TEST_END
}

/**
 * Testing the copy operator
 *
 * Functions: operator=
 */
TEST_F(endpoint_test, check_copy_operator)
{
    TEST_START
    std::shared_ptr<netlist> nl = create_empty_netlist(0);
    endpoint ep;
    std::shared_ptr<gate> test_gate = nl->create_gate(123, get_gate_type_by_name("INV"), "test_gate");
    ep.set_gate(test_gate);
    ep.set_pin_type("O");

    endpoint other_ep;
    other_ep = ep;
    EXPECT_EQ(other_ep.gate, test_gate);
    EXPECT_EQ(other_ep.pin_type, "O");
    TEST_END
}

/**
 * Testing the operator< and operator== by create a set an search in it (the set uses this
 * operators internally)
 *
 * Functions: operator<, operator==
 */
TEST_F(endpoint_test, check_comparison_operators)
{
    TEST_START
    std::shared_ptr<netlist> nl       = create_empty_netlist(0);
    std::shared_ptr<gate> test_gate_0 = nl->create_gate(1, get_gate_type_by_name("INV"), "test_gate_0");
    std::shared_ptr<gate> test_gate_1 = nl->create_gate(2, get_gate_type_by_name("INV"), "test_gate_1");

    // Create some endpoints
    endpoint ep_0 = get_endpoint(test_gate_0, "I");
    endpoint ep_1 = get_endpoint(test_gate_0, "0");
    endpoint ep_2 = get_endpoint(nullptr, "I");
    endpoint ep_3 = get_endpoint(nullptr, "0");
    endpoint ep_4 = get_endpoint(nullptr, "");
    endpoint ep_5 = get_endpoint(test_gate_1, "");

    // Add them to a set
    std::set<endpoint> ep_set = {ep_0, ep_1, ep_2, ep_3, ep_4, ep_5};

    // Search them in the set
    EXPECT_NE(ep_set.find(ep_0), ep_set.end());
    EXPECT_NE(ep_set.find(ep_1), ep_set.end());
    EXPECT_NE(ep_set.find(ep_2), ep_set.end());
    EXPECT_NE(ep_set.find(ep_3), ep_set.end());
    EXPECT_NE(ep_set.find(ep_4), ep_set.end());
    EXPECT_NE(ep_set.find(ep_5), ep_set.end());

    // Search an endpoint which isn't part of the set
    endpoint ep_not_in_set = get_endpoint(test_gate_1, "O");

    EXPECT_EQ(ep_set.find(ep_not_in_set), ep_set.end());

    TEST_END
}

/**
 * Testing the unequal operator
 *
 * Functions: operator!=
 */
TEST_F(endpoint_test, check_unequal_operator)
{
    TEST_START
    std::shared_ptr<netlist> nl     = create_empty_netlist(0);
    std::shared_ptr<gate> test_gate = nl->create_gate(123, get_gate_type_by_name("INV"), "test_gate");

    endpoint ep       = get_endpoint(test_gate, "O");
    endpoint other_ep = get_endpoint(test_gate, "O");
    EXPECT_FALSE(ep != other_ep);
    other_ep = get_endpoint(test_gate, "Other_Pin");
    EXPECT_TRUE(ep != other_ep);

    TEST_END
}