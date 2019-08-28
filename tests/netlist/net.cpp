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

class net_test : public ::testing::Test
{
protected:
    /*const std::string g_lib_name = "EXAMPLE_GATE_LIBRARY";
    // Minimum id for netlists, gates as well as nets
    const u32 MIN_ID = 1;*/

    virtual void SetUp()
    {
        NO_COUT_BLOCK;
        gate_library_manager::load_all();
    }

    virtual void TearDown()
    {
    }

    /*
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

    // Creates a simple gate (AND3) with multiple inputs
    std::shared_ptr<gate> create_test_gate(std::shared_ptr<netlist> nl, const u32 id)
    {
        //NO_COUT_BLOCK;
        std::shared_ptr<gate> res_gate = nl->create_gate(id, "AND3", "gate_" + std::to_string(id));

        return res_gate;
    }

    // returns true if an endpoint is empty
    bool is_empty(endpoint ep)
    {
        return ((ep.gate == nullptr) && (ep.pin_type == ""));
    }

    // Creates an endpoint from a gate and a pin_type
    endpoint get_endpoint(std::shared_ptr<gate> g, std::string pin_type)
    {
        endpoint ep;
        ep.gate     = g;
        ep.pin_type = pin_type;
        return ep;
    }

    // Checks if two endpoint vectors have the same content regardless of their order
    bool vectors_have_same_content(std::vector<endpoint> vec_1, std::vector<endpoint> vec_2)
    {
        if (vec_1.size() != vec_2.size())
        {
            return false;
        }

        // Each element of vec_1 must be found in vec_2
        while (vec_1.size() > 0)
        {
            auto it_1       = vec_1.begin();
            bool found_elem = false;
            for (auto it_2 = vec_2.begin(); it_2 != vec_2.end(); it_2++)
            {
                if (*it_1 == *it_2)
                {
                    found_elem = true;
                    vec_2.erase(it_2);
                    break;
                }
            }
            if (!found_elem)
            {
                return false;
            }
            vec_1.erase(it_1);
        }

        return true;
    }
*/
    /*
     *      Example netlist circuit diagram (Id in brackets). Used for get fan in and
     *      out nets.
     *
     *
     *      GND (1) =-= INV (3) =--=             ------= INV (4) =
     *                                        AND2 (0) =-
     *      VCC (2) =----------------=             ------=
     *                                                            AND2 (5) =
     *                                                          =
     *
     *     =                       =               =----------=                =
     *       BUF (6)      ... OR2 (7)          ... OR2 (8)
     *     =                       =               =          =                =
     */
/*
    // Creates a simple netlist shown in the diagram above
    std::shared_ptr<netlist> create_example_netlist(int id = -1)
    {
        NO_COUT_BLOCK;
        std::shared_ptr<gate_library> gl = gate_library_manager::get_gate_library(g_lib_name);
        std::shared_ptr<netlist> nl(new netlist(gl));
        if (id >= 0)
        {
            nl->set_id(id);
        }

        // Create the gates
        std::shared_ptr<gate> gate_0 = nl->create_gate(MIN_GATE_ID+0, "AND2", "gate_0");
        std::shared_ptr<gate> gate_1 = nl->create_gate(MIN_ID+1, "GND", "gate_1");
        std::shared_ptr<gate> gate_2 = nl->create_gate(MIN_ID+2, "VCC", "gate_2");
        std::shared_ptr<gate> gate_3 = nl->create_gate(MIN_ID+3, "INV", "gate_3");
        std::shared_ptr<gate> gate_4 = nl->create_gate(MIN_ID+4, "INV", "gate_4");
        std::shared_ptr<gate> gate_5 = nl->create_gate(MIN_ID+5, "AND2", "gate_5");
        std::shared_ptr<gate> gate_6 = nl->create_gate(MIN_ID+6, "BUF", "gate_6");
        std::shared_ptr<gate> gate_7 = nl->create_gate(MIN_ID+7, "OR2", "gate_7");
        std::shared_ptr<gate> gate_8 = nl->create_gate(MIN_ID+8, "OR2", "gate_8");

        // Add the nets (net_x_y1_y2... := net between the gate with id x and the gates y1,y2,...)
        std::shared_ptr<net> net_1_3 = nl->create_net(MIN_ID+13, "net_1_3");
        net_1_3->set_src(gate_1, "O");
        net_1_3->add_dst(gate_3, "I");

        std::shared_ptr<net> net_3_0 = nl->create_net(MIN_ID+30, "net_3_0");
        net_3_0->set_src(gate_3, "O");
        net_3_0->add_dst(gate_0, "I0");

        std::shared_ptr<net> net_2_0 = nl->create_net(MIN_ID+20, "net_2_0");
        net_2_0->set_src(gate_2, "O");
        net_2_0->add_dst(gate_0, "I1");

        std::shared_ptr<net> net_0_4_5 = nl->create_net(MIN_ID+045, "net_0_4_5");
        net_0_4_5->set_src(gate_0, "O");
        net_0_4_5->add_dst(gate_4, "I");
        net_0_4_5->add_dst(gate_5, "I0");

        std::shared_ptr<net> net_7_8 = nl->create_net(MIN_ID+78, "net_7_8");
        net_7_8->set_src(gate_7, "O");
        net_7_8->add_dst(gate_8, "I0");

        return nl;
    }*/
};

/**
 * Testing the constructor of the net
 *
 * Functions: constructor, get_id, get_name, get_netlist
 */
TEST_F(net_test, check_constructor)
{
    TEST_START
    // Create a net (id = 100) and append it to its netlist
    std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+100, "test_net");

    EXPECT_EQ(test_net->get_id(), (u32)(MIN_NET_ID+100));
    EXPECT_EQ(test_net->get_name(), "test_net");
    EXPECT_EQ(test_net->get_netlist()->get_id(), (u32)(MIN_NETLIST_ID+0));

    TEST_END
}

/**
 * Testing the function set_name and get_name
 *
 * Functions: get_name, set_name
 */
TEST_F(net_test, check_set_and_get_name)
{
    TEST_START
        // Create a net and append it to its netlist
        std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
        std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");

        EXPECT_EQ(test_net->get_name(), "test_net");

        // Set a new name
        NO_COUT(test_net->set_name("new_name"));
        EXPECT_EQ(test_net->get_name(), "new_name");

        // Set the name to the same new name again
        NO_COUT(test_net->set_name("new_name"));
        EXPECT_EQ(test_net->get_name(), "new_name");

        // Set an empty name (should do nothing)
        NO_COUT(test_net->set_name("name"));
        NO_COUT(test_net->set_name(""));
        EXPECT_EQ(test_net->get_name(), "name");

    TEST_END
}

/**
 * Testing the function set_src
 *
 * Functions: set_src
 */
TEST_F(net_test, check_set_src){
TEST_START
{
    // Set the source of the net (valid gate and pin_type)
    std::shared_ptr<netlist> nl = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");
    std::shared_ptr<gate> t_gate                   = create_test_gate(nl, MIN_GATE_ID+1);
    bool suc                      = test_net->set_src(t_gate, "O");
    EXPECT_TRUE(suc);
    EXPECT_EQ(test_net->get_src(), get_endpoint(t_gate, "O"));
}
{
    // Change the source of the net (valid gate and pin_type)
    std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");
    auto t_gate_0                 = create_test_gate(nl, MIN_GATE_ID+1);
    auto t_gate_1                 = create_test_gate(nl, MIN_GATE_ID+2);
    test_net->set_src(t_gate_0, "O");
    bool suc = test_net->set_src(t_gate_1, "O");
    EXPECT_TRUE(suc);
    EXPECT_EQ(test_net->get_src(), get_endpoint(t_gate_1, "O"));
}

// Negative
{
    // Set the source of the net (gate is nullptr)
    NO_COUT_TEST_BLOCK;
    std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");
    bool suc                      = test_net->set_src(nullptr, "O");
    EXPECT_FALSE(suc);
    EXPECT_TRUE(is_empty(test_net->get_src()));
}
{
    // Pin is an input pin (not an output/inout pin)
    NO_COUT_TEST_BLOCK;
    std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");
    auto t_gate_0                 = create_test_gate(nl, MIN_GATE_ID+1);
    bool suc                      = test_net->set_src(t_gate_0, "I0");    // <- input pin
    EXPECT_FALSE(suc);
    EXPECT_TRUE(is_empty(test_net->get_src()));
}
{
    // Pin is already occupied (example netlist is used)
    NO_COUT_TEST_BLOCK;
    std::shared_ptr<netlist> nl   = create_example_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");
    bool suc                      = test_net->set_src(nl->get_gate_by_id(MIN_NET_ID+1), "O");
    EXPECT_FALSE(suc);
    EXPECT_TRUE(is_empty(test_net->get_src()));
}
{
    // Set the source of the net (invalid pin type)
    std::shared_ptr<netlist> nl   = create_empty_netlist(0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");
    auto t_gate                   = create_test_gate(nl, MIN_GATE_ID+1);
    testing::internal::CaptureStdout();
    bool suc = test_net->set_src(t_gate, "NEx_PIN");
    testing::internal::GetCapturedStdout();
    EXPECT_FALSE(suc);
    EXPECT_TRUE(is_empty(test_net->get_src()));
}

TEST_END
}

/**
 * Testing the function get_src
 *
 * Functions: get_src, get_src_by_type
 */
TEST_F(net_test, check_get_src){
TEST_START{
    // Get the source (valid gate)
    std::shared_ptr<netlist> nl = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");
    auto t_gate                   = create_test_gate(nl, MIN_GATE_ID+1);
    test_net->set_src(t_gate, "O");
    EXPECT_EQ(test_net->get_src(), get_endpoint(t_gate, "O"));
}
{
    // Get the source (pass a connected gate type)
    std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");
    auto t_gate                   = create_test_gate(nl, MIN_GATE_ID+1);
    test_net->set_src(t_gate, "O");
    EXPECT_EQ(test_net->get_src("AND3"), get_endpoint(t_gate, "O"));
    EXPECT_EQ(test_net->get_src_by_type("AND3"), get_endpoint(t_gate, "O"));
}
{
    // Get the source (pass a not connected gate type)
    std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");
    auto t_gate                   = create_test_gate(nl, MIN_GATE_ID+1);
    test_net->set_src(t_gate, "O");
    EXPECT_TRUE(is_empty(test_net->get_src("INV")));    //t_gate has type AND3 not x_INV
    EXPECT_TRUE(is_empty(test_net->get_src_by_type("INV")));
}
{
    // Get the source if the gate has no source
    std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");
    EXPECT_TRUE(is_empty(test_net->get_src()));
    EXPECT_TRUE(is_empty(test_net->get_src()));
}
TEST_END
}

/**
 * Testing the function remove_src
 *
 * Functions: remove_src
 */
TEST_F(net_test, check_remove_src){TEST_START{
    // Remove a set source
    std::shared_ptr<netlist> nl = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");
    auto t_gate                   = create_test_gate(nl, MIN_GATE_ID+1);
    test_net->set_src(t_gate, "O");
    bool suc = test_net->remove_src();
    EXPECT_TRUE(is_empty(test_net->get_src()));
    EXPECT_TRUE(suc);
}
{
    // Remove the source if no source exists
    NO_COUT_TEST_BLOCK;
    std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");
    bool suc                      = test_net->remove_src();
    EXPECT_TRUE(is_empty(test_net->get_src()));
    EXPECT_FALSE(suc);
}

// NEGATIVE
TEST_END
}

/**
 * Testing the function which removes a destination
 *
 * Functions: remove_dst, get_num_of_dsts
 */
TEST_F(net_test, check_add_remove_dst){
TEST_START
{
    // Remove a destination in the normal way
    std::shared_ptr<netlist> nl = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");
    auto t_gate                   = create_test_gate(nl, MIN_GATE_ID+1);
    test_net->add_dst(t_gate, "I0");

    bool suc = test_net->remove_dst(t_gate, "I0");

    EXPECT_TRUE(suc);
    EXPECT_TRUE(test_net->get_dsts().empty());
    EXPECT_EQ(test_net->get_num_of_dsts(), (size_t)0);
}
{
    // Remove the same destination twice
    std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");
    auto t_gate                   = create_test_gate(nl, MIN_GATE_ID+1);
    test_net->add_dst(t_gate, "I0");

    test_net->remove_dst(t_gate, "I0");
    NO_COUT_TEST_BLOCK;                                 // temporary
    bool suc = test_net->remove_dst(t_gate, "I0");

    EXPECT_FALSE(suc);
    EXPECT_TRUE(test_net->get_dsts().empty());
    EXPECT_EQ(test_net->get_num_of_dsts(), 0);
}
// NEGATIVE
{
    // The gate is a nullptr
    NO_COUT_TEST_BLOCK;
    std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");

    bool suc = test_net->remove_dst(nullptr, "I0");

    EXPECT_FALSE(suc);
}
{
    // The gate wasn't added to the netlist
    NO_COUT_TEST_BLOCK;
    std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");
    std::shared_ptr<gate> t_gate  = nl->create_gate(MIN_GATE_ID+0, "INV", "t_gate");

    bool suc = test_net->remove_dst(t_gate, "I0");

    EXPECT_FALSE(suc);
}

TEST_END
}

/**
 * Testing the function which adds a destination
 *
 * Functions: add_dst, get_num_of_dsts
 */
TEST_F(net_test, check_add_dst){
TEST_START
{
    // Add a destination in the normal way
    std::shared_ptr<netlist> nl = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");

    auto t_gate                = create_test_gate(nl, MIN_GATE_ID+1);
    bool suc                   = test_net->add_dst(t_gate, "I0");
    std::vector<endpoint> dsts = {get_endpoint(t_gate, "I0")};
    EXPECT_EQ(test_net->get_dsts(), dsts);
    EXPECT_EQ(test_net->get_num_of_dsts(), (size_t)1);
    EXPECT_TRUE(suc);
}
{
    // Add the same destination twice
    NO_COUT_TEST_BLOCK;
    std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");

    auto t_gate = create_test_gate(nl, MIN_GATE_ID+1);
    test_net->add_dst(t_gate, "I0");
    bool suc                   = test_net->add_dst(t_gate, "I0");
    std::vector<endpoint> dsts = {get_endpoint(t_gate, "I0")};
    EXPECT_EQ(test_net->get_dsts(), dsts);
    EXPECT_EQ(test_net->get_num_of_dsts(), (size_t)1);
    EXPECT_FALSE(suc);
}

// NEGATIVE
{
    // The gate is a nullptr
    NO_COUT_TEST_BLOCK;
    std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");
    bool suc                      = test_net->add_dst(nullptr, "I0");

    EXPECT_FALSE(suc);
    EXPECT_TRUE(test_net->get_dsts().empty());
    EXPECT_EQ(test_net->get_num_of_dsts(), (size_t)0);
}
{
    // The gate isn't part of the netlist
    NO_COUT_TEST_BLOCK;
    std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");
    std::shared_ptr<gate> t_gate  = nl->create_gate(MIN_GATE_ID+0, "INV", "t_gate");
    // gate isn't added
    bool suc = test_net->add_dst(t_gate, "I0");

    EXPECT_FALSE(suc);
    EXPECT_TRUE(test_net->get_dsts().empty());
    EXPECT_EQ(test_net->get_num_of_dsts(), (size_t)0);
}
{
    // The pin to connect is weather an input pin nor an inout pin (but an output pin)
    NO_COUT_TEST_BLOCK;
    std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");
    auto t_gate                   = create_test_gate(nl, MIN_GATE_ID+1);
    bool suc                      = test_net->add_dst(t_gate, "O");

    EXPECT_FALSE(suc);
    EXPECT_TRUE(test_net->get_dsts().empty());
    EXPECT_EQ(test_net->get_num_of_dsts(), (size_t)0);
}
{
    // The pin is already occupied (example netlist is used)
    NO_COUT_TEST_BLOCK;
    std::shared_ptr<netlist> nl   = create_example_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");
    bool suc                      = test_net->add_dst(nl->get_gate_by_id(MIN_GATE_ID+0), "I1");

    EXPECT_FALSE(suc);
    EXPECT_TRUE(test_net->get_dsts().empty());
    EXPECT_EQ(test_net->get_num_of_dsts(), (size_t)0);
}

TEST_END
}

/**
 * Testing the function is_a_dst
 *
 * Functions: is_a_dst
 */
TEST_F(net_test, check_is_a_dst){
TEST_START
{
    // Gate is a destination
    std::shared_ptr<netlist> nl = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");
    auto t_gate                   = create_test_gate(nl, MIN_GATE_ID+1);
    test_net->add_dst(t_gate, "I2");
    EXPECT_TRUE(test_net->is_a_dst(t_gate));
}
{
    // Gate is not a destination
    std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");
    auto t_gate                   = create_test_gate(nl, MIN_GATE_ID+1);
    auto no_dest_gate             = create_test_gate(nl, MIN_GATE_ID+2);
    test_net->add_dst(t_gate, "I2");
    EXPECT_FALSE(test_net->is_a_dst(no_dest_gate));
}
{
    // Gate is a destination (pass the connected pin type)
    std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");
    auto t_gate                   = create_test_gate(nl, MIN_GATE_ID+1);
    test_net->add_dst(t_gate, "I2");
    EXPECT_TRUE(test_net->is_a_dst(t_gate, "I2"));
}
{
    // Gate is a destination but the pin type doesn't match
    std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");
    auto t_gate                   = create_test_gate(nl, MIN_GATE_ID+1);
    test_net->add_dst(t_gate, "I2");
    EXPECT_FALSE(test_net->is_a_dst(t_gate, "A(4)"));
}
{
    // Gate is a destination but the pin type doesn't exist
    std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");
    auto t_gate                   = create_test_gate(nl, MIN_GATE_ID+1);
    test_net->add_dst(t_gate, "I2");
    EXPECT_FALSE(test_net->is_a_dst(t_gate, "NEx_PIN"));
}

// NEGATIVE

{
    // Gate is a nullptr
    NO_COUT_TEST_BLOCK;
    std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");

    EXPECT_FALSE(test_net->is_a_dst(nullptr));
}

TEST_END
}

/**
 * Testing the function get_dsts
 *
 * Functions: get_dsts, get_dsts_by_type
 */
TEST_F(net_test, check_get_dsts)
{
    TEST_START
    // Create a net with two different destinations (AND3 and INV gate)
    std::shared_ptr<netlist> nl    = create_empty_netlist(MIN_NETLIST_ID+0);
    std::shared_ptr<net> test_net  = nl->create_net(MIN_NET_ID+1, "test_net");
    auto mult_gate                 = create_test_gate(nl, MIN_GATE_ID+1);
    std::shared_ptr<gate> inv_gate = nl->create_gate(MIN_GATE_ID+2, "INV", "gate_1");
    test_net->add_dst(mult_gate, "I0");
    test_net->add_dst(inv_gate, "I");

    {
        // Get the destinations
        std::vector<endpoint> dsts = {get_endpoint(mult_gate, "I0"), get_endpoint(inv_gate, "I")};

        EXPECT_TRUE(vectors_have_same_content(test_net->get_dsts(), dsts));
    }
    {
        // Get the destinations by passing a gate type
        std::vector<endpoint> dsts = {get_endpoint(inv_gate, "I")};
        EXPECT_TRUE(vectors_have_same_content(test_net->get_dsts("INV"), dsts));
        EXPECT_TRUE(vectors_have_same_content(test_net->get_dsts_by_type("INV"), dsts));
    }
    TEST_END
}

/**
 * Testing the function is_unrouted
 *
 * Functions: is_unrouted
 */
TEST_F(net_test, check_is_unrouted)
{
    TEST_START
    {
        // Net has a source and a destination
        std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
        std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");

        auto t_gate_src = create_test_gate(nl, MIN_GATE_ID+1);
        auto t_gate_dst = create_test_gate(nl, MIN_GATE_ID+2);
        test_net->set_src(t_gate_src, "O");
        test_net->add_dst(t_gate_dst, "I0");

        EXPECT_FALSE(test_net->is_unrouted());
    }
    {
        // Net has no destination
        std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
        std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");

        auto t_gate_src = create_test_gate(nl, MIN_GATE_ID+1);
        test_net->set_src(t_gate_src, "O");

        EXPECT_TRUE(test_net->is_unrouted());
    }
    {
        // Net has no source
        std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
        std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");

        auto t_gate_dst = create_test_gate(nl, MIN_GATE_ID+1);
        test_net->add_dst(t_gate_dst, "I0");

        EXPECT_TRUE(test_net->is_unrouted());
    }

    TEST_END
}

/**
 * Testing the handling of global nets
 *
 * Functions: mark_global_input_net, mark_global_input_net, mark_global_inout_net,
 *            unmark_global_input_net, unmark_global_input_net, unmark_global_inout_net
 *            is_global_input_net, is_global_input_net, is_global_inout_net
 */
TEST_F(net_test, check_global_nets)
{
    TEST_START
        {
            // mark and unmark a global input net
            std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
            std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");

            test_net->mark_global_input_net();
            EXPECT_TRUE(test_net->is_global_input_net());
            EXPECT_TRUE(nl->is_global_input_net(test_net));

            test_net->unmark_global_input_net();
            EXPECT_FALSE(test_net->is_global_input_net());
            EXPECT_FALSE(nl->is_global_input_net(test_net));
        }
        {
            // mark and unmark a global output net
            std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
            std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");

            test_net->mark_global_output_net();
            EXPECT_TRUE(test_net->is_global_output_net());
            EXPECT_TRUE(nl->is_global_output_net(test_net));

            test_net->unmark_global_output_net();
            EXPECT_FALSE(test_net->is_global_output_net());
            EXPECT_FALSE(nl->is_global_output_net(test_net));
        }
        {
            // mark and unmark a global inout net
            std::shared_ptr<netlist> nl   = create_empty_netlist(MIN_NETLIST_ID+0);
            std::shared_ptr<net> test_net = nl->create_net(MIN_NET_ID+1, "test_net");

            test_net->mark_global_inout_net();
            EXPECT_TRUE(test_net->is_global_inout_net());
            EXPECT_TRUE(nl->is_global_inout_net(test_net));

            test_net->unmark_global_inout_net();
            EXPECT_FALSE(test_net->is_global_inout_net());
            EXPECT_FALSE(nl->is_global_inout_net(test_net));
        }

    TEST_END
}