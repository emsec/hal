#include "netlist_test_utils.h"
#include "netlist/netlist.h"
#include "netlist/gate.h"
#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/net.h"
#include "netlist/netlist_factory.h"
#include "netlist/module.h"
#include "core/plugin_manager.h"
#include "gtest/gtest.h"
#include <core/log.h>
#include <iostream>

using namespace test_utils;

class netlist_test : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

/**
 * Testing the get_shared function which returns a shared_ptr on itselves
 *
 * Functions: get_shared
 */
TEST_F(netlist_test, check_get_shared)
{
    TEST_START
        std::shared_ptr<netlist> nl = create_empty_netlist();
        EXPECT_EQ(nl->get_shared(), nl);
    TEST_END
}

/**
 * Testing the access on the id
 *
 * Functions: get_id, set_id
 */
TEST_F(netlist_test, check_id_access)
{
    TEST_START
        // Create an empty netlist
        std::shared_ptr<netlist> nl = create_empty_netlist();
        // Set the id to another value
        nl->set_id(MIN_NETLIST_ID+123);
        EXPECT_EQ(nl->get_id(), (u32)(MIN_NETLIST_ID+123));
        // Set the id to the same value again
        nl->set_id(MIN_NETLIST_ID+123);
        EXPECT_EQ(nl->get_id(), (u32)(MIN_NETLIST_ID+123));
    TEST_END
}

/**
* Testing the access on the input filename
*
* Functions: get_id, set_id
*/
TEST_F(netlist_test, check_input_filename_access)
{
    TEST_START
        // Create an empty netlist
        std::shared_ptr<netlist> nl = create_empty_netlist();
        // The filename should be empty initially
        EXPECT_EQ(nl->get_input_filename(), hal::path(""));
        // Set a filename
        nl->set_input_filename("/this/is/a/filename");
        EXPECT_EQ(nl->get_input_filename(), hal::path("/this/is/a/filename"));
        // Set the same filename again
        nl->set_input_filename("/this/is/a/filename");
        EXPECT_EQ(nl->get_input_filename(), hal::path("/this/is/a/filename"));
    TEST_END
}

/**
* Testing the access on the design name
*
* Functions: get_design_name, set_design_name
*/
TEST_F(netlist_test, check_design_access)
{
    TEST_START
        // Create an empty netlist
        std::shared_ptr<netlist> nl = create_empty_netlist();
        // Set a design name
        nl->set_design_name("design_name");
        EXPECT_EQ(nl->get_design_name(), "design_name");
        // Set the same design name again
        nl->set_design_name("design_name");
        EXPECT_EQ(nl->get_design_name(), "design_name");
    TEST_END
}

/**
* Testing the access on the device name
*
* Functions: get_device_name, set_device_name
*/
TEST_F(netlist_test, check_device_access)
{
    TEST_START
        // Create an empty netlist
        std::shared_ptr<netlist> nl = create_empty_netlist();
        // Set a design name
        nl->set_device_name("device_name");
        EXPECT_EQ(nl->get_device_name(), "device_name");
        // Set the same design name again
        nl->set_device_name("device_name");
        EXPECT_EQ(nl->get_device_name(), "device_name");
    TEST_END
}

/**
* Testing the access on the stored pointers of the netlist_internal_manager,
 * the module_manager and the module_manager
*
* Functions: get_gate_library
*/
TEST_F(netlist_test, check_pointer_access)
{
    TEST_START
        // Create an empty netlist
        std::shared_ptr<netlist> nl = create_empty_netlist();
        EXPECT_NE(nl->get_gate_library(), nullptr);
    TEST_END
}

/**
 * Testing the get_unique_gate_id function by creating a netlist with gates of the id
 * 0,1,2,4. The new gate id should be 3.
 *
 * Functions: get_unique_gate_id
 */
TEST_F(netlist_test, check_get_unique_gate_id)
{
    TEST_START
        // Create an empty netlist with some gates
        std::shared_ptr<netlist> nl = create_empty_netlist();
        std::shared_ptr<gate> g_0   = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("gate_1_to_1"), "gate_0");
        std::shared_ptr<gate> g_1   = nl->create_gate(MIN_GATE_ID+1, get_gate_type_by_name("gate_1_to_1"), "gate_1");
        std::shared_ptr<gate> g_2   = nl->create_gate(MIN_GATE_ID+3, get_gate_type_by_name("gate_1_to_1"), "gate_2");
        std::set<u32> used_ids = {MIN_GATE_ID+0,MIN_GATE_ID+1,MIN_GATE_ID+3};

        // Get a unique id
        u32 unique_id = nl->get_unique_gate_id();
        EXPECT_TRUE(used_ids.find(unique_id) == used_ids.end());
        EXPECT_NE(unique_id, INVALID_GATE_ID);

        // Insert the unique id gate and get a new unique id
        std::shared_ptr<gate> g_new   = nl->create_gate(unique_id, get_gate_type_by_name("gate_1_to_1"), "gate_2");
        used_ids.insert(unique_id);

        unique_id = nl->get_unique_gate_id();
        EXPECT_TRUE(used_ids.find(unique_id) == used_ids.end());
        EXPECT_NE(unique_id, INVALID_GATE_ID);

        // Remove a gate and get a new unique id
        nl->delete_gate(g_1);
        used_ids.erase(MIN_GATE_ID+1);

        unique_id = nl->get_unique_gate_id();
        EXPECT_TRUE(used_ids.find(unique_id) == used_ids.end());
        EXPECT_NE(unique_id, INVALID_GATE_ID);

    TEST_END
}

/**
 * Testing get_num_of_gates function
 *
 * Functions: get_num_of_gates
 */
TEST_F(netlist_test, check_get_num_of_gates)
{
    TEST_START
        // Create an empty netlist with 4 gates
        std::shared_ptr<netlist> nl = create_empty_netlist();
        std::shared_ptr<gate> g_0   = nl->create_gate(nl->get_unique_gate_id(), get_gate_type_by_name("gate_1_to_1"), "gate_0");
        std::shared_ptr<gate> g_1   = nl->create_gate(nl->get_unique_gate_id(), get_gate_type_by_name("gate_1_to_1"), "gate_1");
        std::shared_ptr<gate> g_2   = nl->create_gate(nl->get_unique_gate_id(), get_gate_type_by_name("gate_1_to_1"), "gate_2");
        std::shared_ptr<gate> g_3   = nl->create_gate(nl->get_unique_gate_id(), get_gate_type_by_name("gate_1_to_1"), "gate_4");

        EXPECT_EQ(nl->get_gates().size(), (size_t)4);

    TEST_END
}

/**
 * Testing addition of gates (calls the function create_gate in netlist_internal_manager)
 *
 * Functions: create_gate
 */
TEST_F(netlist_test, check_add_gate){
    TEST_START
        {
            // Add a gate the normal way
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<gate> g_0 = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("gate_1_to_1"), "gate_0");
            EXPECT_TRUE(nl->is_gate_in_netlist(g_0));
        }
        {
            // Add a gate, remove it afterwards and add it again (used to test the free_ids logic)
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<gate> g_0 = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("gate_1_to_1"), "gate_0");
            nl->delete_gate(g_0);
            std::shared_ptr<gate> g_new = nl->create_gate(get_gate_type_by_name("gate_1_to_1"), "gate_0");
            EXPECT_TRUE(nl->is_gate_in_netlist(g_new));
        }
        // NEGATIVE
        {
            // Try to add the same gate twice
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<gate> g_0   = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("gate_1_to_1"), "gate_0");
            std::shared_ptr<gate> g_1   = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("gate_1_to_1"), "gate_0");
            EXPECT_NE(g_0, nullptr);
            EXPECT_EQ(g_1, nullptr);
        }
        {
            // Try to add two gates with the same id
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<gate> g_0 = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("gate_1_to_1"), "gate_0");
            std::shared_ptr<gate> g_0_other = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("gate_1_to_1"), "gate_0_other");
            EXPECT_EQ(g_0_other, nullptr);
        }
        {
            // Try to add a gate with an invalid id
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<gate> g_invalid   = nl->create_gate(INVALID_GATE_ID, get_gate_type_by_name("gate_1_to_1"), "gate_0");
            EXPECT_EQ(g_invalid, nullptr);
        }
        {
            // Try to add a gate with an invalid name (empty string)
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<gate> g_0   = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("gate_1_to_1"), "");
            EXPECT_EQ(g_0, nullptr);
        }
    TEST_END
}

/**
 * Testing deletion of gates (calls the function delete_gate in netlist_internal_manager)
 *
 * Functions: delete_gate
 */
TEST_F(netlist_test, check_delete_gate){
    TEST_START
        // POSITIVE
        {
            // Add and delete an unconnected gate in a normal way
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<gate> g_0 = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("gate_1_to_1"), "gate_0");
            bool suc                  = nl->delete_gate(g_0);
            EXPECT_TRUE(suc);
            EXPECT_FALSE(nl->is_gate_in_netlist(g_0));
        }
        {
            // Delete a gate, which is connected to some in and output nets.
            // The nets source and destination should be updated
            std::shared_ptr<netlist> nl  = create_example_netlist();
            std::shared_ptr<gate> gate_0 = nl->get_gate_by_id(MIN_GATE_ID+0);
            bool suc                     = nl->delete_gate(gate_0);
            EXPECT_TRUE(suc);
            NO_COUT_TEST_BLOCK;
            EXPECT_FALSE(nl->get_net_by_id(MIN_NET_ID+30)->is_a_destination(get_endpoint(gate_0, "I0")));
            EXPECT_FALSE(nl->get_net_by_id(MIN_NET_ID+20)->is_a_destination(get_endpoint(gate_0, "I1")));
            EXPECT_EQ(nl->get_net_by_id(MIN_NET_ID+045)->get_source(), get_endpoint(nullptr, ""));
        }
        {
            // Add and delete global_gnd gate
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<gate> g_0   = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("gnd"), "gate_0");
            nl->mark_gnd_gate(g_0);
            bool suc = nl->delete_gate(g_0);
            EXPECT_TRUE(suc);
            EXPECT_FALSE(nl->is_gate_in_netlist(g_0));
            EXPECT_TRUE(nl->get_gnd_gates().empty());
        }
        {
            // Add and delete global_vcc gate
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<gate> g_0   = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("vcc"), "gate_0");
            nl->mark_vcc_gate(g_0);
            bool suc = nl->delete_gate(g_0);
            EXPECT_TRUE(suc);
            EXPECT_FALSE(nl->is_gate_in_netlist(g_0));
            EXPECT_TRUE(nl->get_vcc_gates().empty());
        }
        // NEGATIVE
        {
            // Try to delete a nullptr
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            bool suc                    = nl->delete_gate(nullptr);
            EXPECT_FALSE(suc);
        }
        {
            // Try to delete a gate which is not part of the netlist
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<gate> g_0   = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("gate_1_to_1"), "gate_0");
            bool suc                    = nl->delete_gate(g_0);
            EXPECT_TRUE(suc);
        }

    TEST_END
}

/**
 * Testing the function is_gate_in_netlist
 *
 * Functions: is_gate_in_netlist
 */
TEST_F(netlist_test, check_is_gate_in_netlist){
    TEST_START
        {
            // Gate is part of the netlist
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<gate> g_0 = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("gate_1_to_1"), "gate_0");
            EXPECT_TRUE(nl->is_gate_in_netlist(g_0));
        }
        {
            // Gate is not part of the netlist
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<gate> g_0   = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("gate_1_to_1"), "gate_0");
            nl->delete_gate(g_0);
            // Gate isn't added
            EXPECT_FALSE(nl->is_gate_in_netlist(g_0));
        }
        {
            // Gate is a nullptr
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            EXPECT_FALSE(nl->is_gate_in_netlist(nullptr));
        }
    TEST_END
}

/**
 * Testing the function get_gate_by_id
 *
 * Functions: get_gate_by_id
 */
TEST_F(netlist_test, check_get_gate_by_id){
    TEST_START
        {
            // Get (existing) gate with id 3
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<gate> g_0 = nl->create_gate(MIN_GATE_ID+3, get_gate_type_by_name("gate_1_to_1"), "gate_0");
            EXPECT_EQ(nl->get_gate_by_id(MIN_GATE_ID+3), g_0);
        }
        {
            // Get not existing gate
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            // Gate isn't added
            EXPECT_EQ(nl->get_gate_by_id(MIN_GATE_ID+3), nullptr);
        }
    TEST_END
}

/**
 * Testing the function get_gates by using a namefilter
 *
 * Functions: get_gates
 */
TEST_F(netlist_test, check_get_gates_by_name){
    TEST_START
        {
            // Get an existing gate of the example netlist by its name
            std::shared_ptr<netlist> nl = create_example_netlist();
            std::shared_ptr<gate> g_0 = nl->get_gate_by_id(MIN_GATE_ID+0);
            std::shared_ptr<gate> g_5 = nl->get_gate_by_id(MIN_GATE_ID+5);
            EXPECT_EQ(nl->get_gates(gate_name_filter("gate_0")), std::set<std::shared_ptr<gate>>({g_0}));
            EXPECT_EQ(nl->get_gates(gate_name_filter("gate_5")), std::set<std::shared_ptr<gate>>({g_5}));
        }
        {
            // Call with an non existing gate name
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            EXPECT_EQ(nl->get_gates(gate_name_filter("not_existing_gate")), std::set<std::shared_ptr<gate>>());
        }
    TEST_END
}

/**
 * Testing the get_gates function using the example netlist (see above). Also tests
 * the get_gate_by_type and the get_gates_by_decorator_type (these functions calls
 * the get_gates function)
 *
 * Functions: get_gates
 */
TEST_F(netlist_test, check_get_gates){
    TEST_START
        {
            // Get all gates of the example netlist
            std::shared_ptr<netlist> nl = create_example_netlist();
            // The expected result
            std::set<std::shared_ptr<gate>> ex_gates;
            for (int id = 0; id <= 8; id++)
            {
                ex_gates.insert(nl->get_gate_by_id(MIN_GATE_ID+id));
            }

            EXPECT_EQ(nl->get_gates(), ex_gates);
        }
        {
            // Get all INV gates of the example netlist
            std::shared_ptr<netlist> nl = create_example_netlist();
            // The expected result
            std::set<std::shared_ptr<gate>> ex_gates = {nl->get_gate_by_id(MIN_GATE_ID+3), nl->get_gate_by_id(MIN_GATE_ID+4)};

            EXPECT_EQ(nl->get_gates(gate_type_filter("gate_1_to_1")), ex_gates);
            EXPECT_EQ(nl->get_gates(gate_type_filter("gate_1_to_1")), ex_gates);
        }
    TEST_END
}

/**
 * Testing the addition and deletion of vcc and gnd gates. The success is
 * verified via the functions get_vcc_gates, get_gnd_gates,
 * is_vcc_gate, is_gnd_gate
 *
 * Functions: mark_vcc_gate, mark_gnd_gate, is_vcc_gate
 *           ,is_gnd_gate, get_vcc_gates, get_gnd_gates
 */
TEST_F(netlist_test, check_mark_vcc_gate){
    TEST_START
        {
            // Add a global vcc gate which wasn't added to the netlist before and unmark it after
            std::shared_ptr<netlist> nl = create_empty_netlist();

            std::shared_ptr<gate> vcc_gate = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("vcc"), "gate_vcc");
            bool suc_mark                       = nl->mark_vcc_gate(vcc_gate);
            EXPECT_TRUE(suc_mark);
            EXPECT_TRUE(nl->is_vcc_gate(vcc_gate));
            EXPECT_EQ(nl->get_vcc_gates(), std::set<std::shared_ptr<gate>>({vcc_gate}));

            bool suc_unmark = nl->unmark_vcc_gate(vcc_gate);
            EXPECT_TRUE(suc_unmark);
            EXPECT_FALSE(nl->is_vcc_gate(vcc_gate));
        }

        {
            // Add a global gnd gate which which wasn't added to the netlist before and unmark it after
            std::shared_ptr<netlist> nl = create_empty_netlist();

            std::shared_ptr<gate> gnd_gate = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("gnd"), "gate_gnd");
            bool suc_mark                       = nl->mark_gnd_gate(gnd_gate);
            EXPECT_TRUE(suc_mark);
            EXPECT_TRUE(nl->is_gnd_gate(gnd_gate));
            EXPECT_EQ(nl->get_gnd_gates(), std::set<std::shared_ptr<gate>>({gnd_gate}));

            bool suc_unmark = nl->unmark_gnd_gate(gnd_gate);
            EXPECT_TRUE(suc_unmark);
            EXPECT_FALSE(nl->is_gnd_gate(gnd_gate));
        }
        {
            // Add the same global vcc gate twice
            std::shared_ptr<netlist> nl = create_empty_netlist();

            std::shared_ptr<gate> vcc_gate = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("vcc"), "gate_vcc");
            nl->mark_vcc_gate(vcc_gate);
            bool suc = nl->mark_vcc_gate(vcc_gate);
            EXPECT_TRUE(suc);
            EXPECT_TRUE(nl->is_vcc_gate(vcc_gate));
            EXPECT_EQ(nl->get_vcc_gates(), std::set<std::shared_ptr<gate>>({vcc_gate}));
        }
        {
            // Add the same global gnd gate twice
            std::shared_ptr<netlist> nl = create_empty_netlist();

            std::shared_ptr<gate> gnd_gate = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("gnd"), "gate_gnd");
            nl->mark_gnd_gate(gnd_gate);
            bool suc = nl->mark_gnd_gate(gnd_gate);
            EXPECT_TRUE(suc);
            EXPECT_TRUE(nl->is_gnd_gate(gnd_gate));
            EXPECT_EQ(nl->get_gnd_gates(), std::set<std::shared_ptr<gate>>({gnd_gate}));
        }

        // NEGATIVE
        {
            // Mark a global_gnd gate, which is not part of the netlist (part of another netlist)
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<netlist> nl_other = create_empty_netlist();
            std::shared_ptr<gate> gnd_gate  = nl_other->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("gnd"), "gate_0");

            bool suc = nl->mark_gnd_gate(gnd_gate);

            EXPECT_FALSE(suc);
        }
        {
            // Mark a global_vcc gate, which is not part of the netlist (part of another netlist)
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<netlist> nl_other = create_empty_netlist();
            std::shared_ptr<gate> vcc_gate  = nl_other->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("vcc"), "gate_0");

            bool suc = nl->mark_vcc_gate(vcc_gate);

            EXPECT_FALSE(suc);
        }
        {
            // Unmark a global_gnd gate, which is not part of the netlist (part of another netlist)
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<netlist> nl_other = create_empty_netlist();
            std::shared_ptr<gate> gnd_gate  = nl_other->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("gnd"), "gate_0");
            nl_other->mark_gnd_gate(gnd_gate);

            nl->unmark_gnd_gate(gnd_gate);

            EXPECT_TRUE(nl_other->is_gnd_gate(gnd_gate));
        }
        {
            // Unmark a global_vcc gate, which is not part of the netlist (part of another netlist)
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<netlist> nl_other = create_empty_netlist();
            std::shared_ptr<gate> vcc_gate  = nl_other->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("vcc"), "gate_0");
            nl_other->mark_vcc_gate(vcc_gate);

            nl->unmark_vcc_gate(vcc_gate);

            EXPECT_TRUE(nl_other->is_vcc_gate(vcc_gate));
        }

    TEST_END
}

/**
 * Testing the get_unique_net_id function by creating a netlist with nets of the id
 * 0,1,2,4. The new net id shouldn't be one of them.
 *
 * Functions: get_unique_net_id
 */
TEST_F(netlist_test, check_get_unique_net_id)
{
    TEST_START
        {
            // Create an empty netlist with some nets and get a unique id
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> n_0 = nl->create_net(MIN_NET_ID + 0, "net_0");
            std::shared_ptr<net> n_1 = nl->create_net(MIN_NET_ID + 1, "net_1");
            std::shared_ptr<net> n_3 = nl->create_net(MIN_NET_ID + 3, "net_3");
            std::set<u32> used_ids = {MIN_NET_ID + 0, MIN_NET_ID + 1, MIN_NET_ID + 3};

            u32 new_net_id = nl->get_unique_net_id();
            EXPECT_TRUE(used_ids.find(new_net_id) == used_ids.end());
        }
        {
            // Create an empty netlist with some nets, delete a net, and get the id
            // (used to test the free_net ids logic)
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> n_0 = nl->create_net(MIN_NET_ID + 0, "net_0");
            std::shared_ptr<net> n_1 = nl->create_net(MIN_NET_ID + 1, "net_1");
            std::shared_ptr<net> n_2 = nl->create_net(MIN_NET_ID + 2, "net_2");

            nl->delete_net(n_1);

            std::set<u32> used_ids = {MIN_NET_ID + 0, MIN_NET_ID + 2};
            u32 new_net_id = nl->get_unique_net_id();
            EXPECT_TRUE(used_ids.find(new_net_id) == used_ids.end());
        }

    TEST_END
}

/**
 * Testing get_num_of_nets by call it for the example netlist
 *
 * Functions: get_num_of_nets
 */
TEST_F(netlist_test, check_get_num_of_nets)
{
    TEST_START
        // Create the example netlist (has 5 nets)
        std::shared_ptr<netlist> nl = create_example_netlist();
        EXPECT_EQ(nl->get_nets().size(), (size_t)5);
    TEST_END
}

/**
 * Testing addition of nets (calls the function create_net in netlist_internal_manager).
 * To validate success the function is_net_in_netlist is used.
 *
 * Functions: create_net
 */
TEST_F(netlist_test, check_add_net){
    TEST_START
        {
            // Add a net the normal way (unrouted)
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_0 = nl->create_net(MIN_NET_ID+0, "net_0");
            EXPECT_NE(net_0, nullptr);
            EXPECT_TRUE(nl->is_net_in_netlist(net_0));
        }
        {
            // Add a net the normal way by passing no id
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_0 = nl->create_net("net_0");
            EXPECT_NE(net_0, nullptr);
            EXPECT_TRUE(nl->is_net_in_netlist(net_0));
        }
        {
            // Add a net, remove it, and add a net with the same id (used to test the free_net_ids logic)
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_0 = nl->create_net(MIN_NET_ID+0, "net_0");
            nl->delete_net(net_0);
            std::shared_ptr<net> net_0_other = nl->create_net(MIN_NET_ID+0, "net_0_other");
            EXPECT_NE(net_0_other, nullptr);
            EXPECT_TRUE(nl->is_net_in_netlist(net_0_other));
        }
        // NEGATIVE
        {
            // Create a net with an invalid id
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_invalid = nl->create_net(INVALID_NET_ID, "net_invalid");
            EXPECT_TRUE(net_invalid == nullptr);
        }
        {
            // Create a net with an already used id
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_0 = nl->create_net(MIN_NET_ID+0, "net_0");
            std::shared_ptr<net> net_0_other = nl->create_net(MIN_NET_ID+0, "net_0_other");
            EXPECT_TRUE(net_0_other == nullptr);
        }
        {
            // Create a net with an invalid name (empty string)
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_0 = nl->create_net(MIN_NET_ID+0, "");
            EXPECT_TRUE(net_0 == nullptr);
        }
    TEST_END
}

/**
 * Testing deletion of nets (calls the function delete_net in netlist_internal_manager)
 *
 * Functions: delete_net
 */
TEST_F(netlist_test, check_delete_net){
    TEST_START
        // POSITIVE
        {
            // Add and delete an unrouted net in a normal way
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_0 = nl->create_net(MIN_NET_ID+0, "net_0");
            bool suc                   = nl->delete_net(net_0);
            EXPECT_TRUE(suc);
            EXPECT_FALSE(nl->is_net_in_netlist(net_0));
        }
        {
            // Delete a net, which isn't unrouted by using the example netlist
            std::shared_ptr<netlist> nl  = create_example_netlist();
            std::shared_ptr<net> net_045 = nl->get_net_by_id(MIN_NET_ID+045);
            bool suc                     = nl->delete_net(net_045);

            EXPECT_TRUE(suc);
            EXPECT_FALSE(nl->is_net_in_netlist(net_045));
            // Check if the netlist was updated correctly
            EXPECT_TRUE(nl->get_gate_by_id(MIN_GATE_ID+0)->get_successors().empty());
            EXPECT_TRUE(nl->get_gate_by_id(MIN_GATE_ID+4)->get_predecessors().empty());
            EXPECT_TRUE(nl->get_gate_by_id(MIN_GATE_ID+5)->get_predecessors().empty());
        }
        {
            // Delete a global input net
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_0  = nl->create_net(MIN_NET_ID+0, "net_0");
            nl->mark_global_input_net(net_0);
            bool suc = nl->delete_net(net_0);

            EXPECT_TRUE(suc);
            EXPECT_FALSE(nl->is_net_in_netlist(net_0));
            EXPECT_FALSE(nl->is_global_input_net(net_0));
        }
        {
            // Delete a global output net
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_0  = nl->create_net(MIN_NET_ID+0, "net_0");
            nl->mark_global_output_net(net_0);
            bool suc = nl->delete_net(net_0);

            EXPECT_TRUE(suc);
            EXPECT_FALSE(nl->is_net_in_netlist(net_0));
            EXPECT_FALSE(nl->is_global_output_net(net_0));
        }
        // NEGATIVE
        {
            // Try to delete a nullptr
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            bool suc                    = nl->delete_net(nullptr);

            EXPECT_FALSE(suc);
            EXPECT_FALSE(nl->is_net_in_netlist(nullptr));
        }
        {
            // Try to delete a net which is not part of the netlist
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_0  = nl->create_net(MIN_NET_ID+0, "net_0");
            // net_0 wasn't added
            bool suc = nl->delete_net(net_0);

            EXPECT_TRUE(suc);
            EXPECT_FALSE(nl->is_net_in_netlist(net_0));
        }
    TEST_END
}

/**
 * Testing the function is_net_in_netlist
 *
 * Functions: is_net_in_netlist
 */
TEST_F(netlist_test, check_is_net_in_netlist){
    TEST_START
        {
            // Net is part of the netlist
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_0 = nl->create_net(MIN_NET_ID+1, "net_0");

            EXPECT_TRUE(nl->is_net_in_netlist(net_0));
        }

        {
            // Net is a nullptr
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            EXPECT_FALSE(nl->is_net_in_netlist(nullptr));
        }
    TEST_END
}

/**
 * Testing the get_nets function using the example netlist (see above).
 *
 * Functions: get_nets
 */
TEST_F(netlist_test, check_get_nets){
    TEST_START
        {
            // Get all nets of the example netlist
            std::shared_ptr<netlist> nl = create_example_netlist();
            // The expected result
            std::unordered_set<std::shared_ptr<net>> ex_nets;
            for (int id : std::set<int>({(int) MIN_NET_ID+13, (int) MIN_NET_ID+30,(int) MIN_NET_ID+20, (int) MIN_NET_ID+045, (int) MIN_NET_ID+78}))
            {
                ex_nets.insert(nl->get_net_by_id(id));
            }

            EXPECT_EQ(nl->get_nets(), ex_nets);
        }
    TEST_END
}

/**
 * Testing the function get_net_by_id
 *
 * Functions: get_net_by_id
 */
TEST_F(netlist_test, check_get_net_by_id){
    TEST_START
        {
            // Net exists (ID: 123)
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_0 = nl->create_net(MIN_NET_ID+123, "net_0");

            EXPECT_EQ(nl->get_net_by_id(MIN_NET_ID+123), net_0);
        }
        {
            // Net doesn't exists (ID: 123)
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            EXPECT_EQ(nl->get_net_by_id(MIN_NET_ID+123), nullptr);
        }
    TEST_END
}

/**
 * Testing the function get_nets_by_name
 *
 * Functions: get_nets_by_name
 */
TEST_F(netlist_test, check_get_nets_by_name){
    TEST_START
        {
            // Get an existing net by its name
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_0 = nl->create_net(MIN_NET_ID+0, "net_name");
            std::shared_ptr<net> net_1 = nl->create_net(MIN_NET_ID+1, "other_net_name");

            EXPECT_EQ(nl->get_nets(), std::unordered_set<std::shared_ptr<net>>({net_0, net_1}));
            EXPECT_EQ(nl->get_nets(net_name_filter("net_name")), std::unordered_set<std::shared_ptr<net>>({net_0}));
        }
        {
            // Call with an non existing net name
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            EXPECT_EQ(nl->get_nets(net_name_filter("not_existing_net")), std::unordered_set<std::shared_ptr<net>>());
        }

    TEST_END
}

/**
 * Testing the addition of global input/output nets. For verification
 * the function is_global_input/output_net is called
 *
 * Functions: mark_global_input_net, mark_global_output_net,
 *            unmark_global_input_net, unmark_global_output_net
 */
TEST_F(netlist_test, check_add_global_net){
    TEST_START
        // Mark functions (normal usage)
        {
            // Add a global input net which isn't part of the netlist yet
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_0 = nl->create_net(MIN_NET_ID+0, "net_0");
            bool suc                   = nl->mark_global_input_net(net_0);

            EXPECT_TRUE(suc);
            EXPECT_TRUE(nl->is_global_input_net(net_0));
        }
        {
            // Add a global output net which isn't part of the netlist yet
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_0  = nl->create_net(MIN_NET_ID+0, "net_0");
            bool suc                    = nl->mark_global_output_net(net_0);

            EXPECT_TRUE(suc);
            EXPECT_TRUE(nl->is_global_output_net(net_0));
        }
        // Unmark functions (normal usage)
        {
            // Mark and unmark a global input net
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_0 = nl->create_net(MIN_NET_ID+0, "net_0");
            bool suc_mark              = nl->mark_global_input_net(net_0);

            ASSERT_TRUE(suc_mark);
            ASSERT_TRUE(nl->is_global_input_net(net_0));

            bool suc_unmark = nl->unmark_global_input_net(net_0);
            ASSERT_TRUE(suc_unmark);
            ASSERT_FALSE(nl->is_global_input_net(net_0));
        }
        {
            // Mark and unmark a global output net
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_0 = nl->create_net(MIN_NET_ID+0, "net_0");
            bool suc_mark              = nl->mark_global_output_net(net_0);

            ASSERT_TRUE(suc_mark);
            ASSERT_TRUE(nl->is_global_output_net(net_0));

            bool suc_unmark = nl->unmark_global_output_net(net_0);
            ASSERT_TRUE(suc_unmark);
            ASSERT_FALSE(nl->is_global_output_net(net_0));
        }
        {
            // Add a global input net which was already added (as normal net)
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_0  = nl->create_net(MIN_NET_ID+0, "net_0");
            bool suc                    = nl->mark_global_input_net(net_0);

            EXPECT_TRUE(suc);
            EXPECT_TRUE(nl->is_global_input_net(net_0));
        }
        {
            // Add a global output net which was already added (as normal net)
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_0  = nl->create_net(MIN_NET_ID+0, "net_0");
            bool suc                    = nl->mark_global_output_net(net_0);

            EXPECT_TRUE(suc);
            EXPECT_TRUE(nl->is_global_output_net(net_0));
        }
        {
            // Add the same global input net twice
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_0  = nl->create_net(MIN_NET_ID+0, "net_0");
            nl->mark_global_input_net(net_0);
            bool suc = nl->mark_global_input_net(net_0);

            EXPECT_TRUE(suc);
            EXPECT_TRUE(nl->is_global_input_net(net_0));
        }
        {
            // Add the same global output net twice
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_0  = nl->create_net(MIN_NET_ID+0, "net_0");
            nl->mark_global_output_net(net_0);
            bool suc = nl->mark_global_output_net(net_0);

            EXPECT_TRUE(suc);
            EXPECT_TRUE(nl->is_global_output_net(net_0));
        }
        // NEGATIVE
        // Mark functions
        {
            // Mark an input-net, which is not part of the netlist (part of another netlist)
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<netlist> nl_other = create_empty_netlist();
            std::shared_ptr<net> net_0 = nl_other->create_net(MIN_NET_ID + 0, "net_0");

            bool suc = nl->mark_global_input_net(net_0);

            EXPECT_FALSE(suc);
        }
        {
            // Mark an output-net, which is not part of the netlist (part of another netlist)
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<netlist> nl_other = create_empty_netlist();
            std::shared_ptr<net> net_0  = nl_other->create_net(MIN_NET_ID+0, "net_0");

            bool suc = nl->mark_global_output_net(net_0);

            EXPECT_FALSE(suc);
            EXPECT_FALSE(nl->is_global_output_net(net_0));
        }
        // Unmark functions
        {
            // Unmark a global input-net, which isn't marked as such
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_0  = nl->create_net(MIN_NET_ID+0, "net_0");

            bool suc = nl->unmark_global_input_net(net_0);

            EXPECT_FALSE(suc);
        }
        {
            // Unmark a global output-net, which isn't marked as such
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_0  = nl->create_net(MIN_NET_ID+0, "net_0");

            bool suc = nl->unmark_global_output_net(net_0);

            EXPECT_FALSE(suc);
        }
        {
            // Unmark a global input-net, which is part of another netlist
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<netlist> nl_other = create_empty_netlist();
            std::shared_ptr<net> net_0  = nl_other->create_net(MIN_NET_ID+0, "net_0");
            nl_other->mark_global_input_net(net_0);

            bool suc = nl->unmark_global_input_net(net_0);

            EXPECT_FALSE(suc);
            EXPECT_TRUE(nl_other->is_global_input_net(net_0));
        }
        {
            // Unmark a global output-net, which is part of another netlist
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<netlist> nl_other = create_empty_netlist();
            std::shared_ptr<net> net_0  = nl_other->create_net(MIN_NET_ID+0, "net_0");
            nl_other->mark_global_output_net(net_0);

            bool suc = nl->unmark_global_output_net(net_0);

            EXPECT_FALSE(suc);
            EXPECT_TRUE(nl_other->is_global_output_net(net_0));
        }
    TEST_END
}

/**
 * Testing the functions is_global_input/output_net. Also tests the access via
 * get_global_input/output_nets
 *
 * Functions: is_global_input_net, is_global_output_net, is_global_net,
 *            get_global_input_nets, get_global_out_nets, get_global_out_nets
 */
TEST_F(netlist_test, check_is_global_net)
{
    TEST_START
        {
            // The net is a global input net
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_0 = nl->create_net(MIN_NET_ID+0, "net_0");
            nl->mark_global_input_net(net_0);

            EXPECT_TRUE(nl->is_global_input_net(net_0));
            EXPECT_EQ(nl->get_global_input_nets(), std::set<std::shared_ptr<net>>({net_0}));
        }
        {
            // The net is a global output net
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_0  = nl->create_net(MIN_NET_ID+0, "net_0");
            nl->mark_global_output_net(net_0);

            EXPECT_TRUE(nl->is_global_output_net(net_0));
            EXPECT_EQ(nl->get_global_output_nets(), std::set<std::shared_ptr<net>>({net_0}));
        }
        {
            // The net isn't a global input net
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_0  = nl->create_net(MIN_NET_ID+0, "net_0");

            EXPECT_FALSE(nl->is_global_input_net(net_0));
            EXPECT_TRUE(nl->get_global_input_nets().empty());
        }
        {
            // The net isn't a global output net
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<net> net_0  = nl->create_net(MIN_NET_ID+0, "net_0");

            EXPECT_FALSE(nl->is_global_output_net(net_0));
            EXPECT_TRUE(nl->get_global_output_nets().empty());
        }
    TEST_END
}


/***************************************************
 *               Module Functions
 ***************************************************/

/**
 * Testing the get_top_module function
 *
 * Functions: get_top_module
 */
TEST_F(netlist_test, check_get_top_module)
{
    TEST_START
        {
            // Testing the access on the topmodule
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> tm = nl->get_top_module();
            ASSERT_NE(tm, nullptr);
            EXPECT_EQ(tm->get_parent_module(), nullptr);
            // If only this test fails you may update the TOP_MODULE_ID constant
            EXPECT_EQ(tm->get_id(), TOP_MODULE_ID);
        }
    TEST_END
}

/**
* Testing the creation of a module
*
* Functions: create_module
*/
TEST_F(netlist_test, check_create_module)
{
    TEST_START
        // Positive
        {
            // Create a module
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0,"module_0", nl->get_top_module());
            EXPECT_TRUE(nl->is_module_in_netlist(m_0));
        }
        {
            // Create a module without passing an id
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module("module_0", nl->get_top_module());
            EXPECT_TRUE(nl->is_module_in_netlist(m_0));
        }
        {
            // Add a module, remove it and add a module with the same id (to test the free_module_id logic)
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0,"module_0", nl->get_top_module());
            nl->delete_module(m_0);
            std::shared_ptr<module> m_0_other = nl->create_module(MIN_MODULE_ID+0,"module_0_other", nl->get_top_module());
            //EXPECT_FALSE(nl->is_module_in_netlist(m_0)); //ISSUE: should be  true
            EXPECT_TRUE(nl->is_module_in_netlist(m_0_other));
        }

        // Negative
        {
            // Create a module with an invalid id
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(INVALID_MODULE_ID,"module_0", nl->get_top_module());
            EXPECT_EQ(m_0, nullptr);
        }
        {
            // Create a module with an id, which is already used
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0,"module_0", nl->get_top_module());
            std::shared_ptr<module> m_0_other = nl->create_module(MIN_MODULE_ID+0,"module_0_other", nl->get_top_module());
            EXPECT_EQ(m_0_other, nullptr);
        }
        {
            // Create a module with the id, used of the top module
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(TOP_MODULE_ID,"module_0", nl->get_top_module());
            EXPECT_EQ(m_0, nullptr);
            EXPECT_EQ(nl->get_module_by_id(TOP_MODULE_ID), nl->get_top_module());
        }
        {
            // Create a module with an invalid name (empty string)
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0,"", nl->get_top_module());
            EXPECT_EQ(m_0, nullptr);
        }
        {
            // Create a module with no parent-module
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0,"module_0", nullptr);
            EXPECT_EQ(m_0, nullptr);
        }
        /*{
            // Create a module where the parrent module is part of ANOTHER netlist ISSUE: fails
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<netlist> nl_other = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0,"module_0", nl_other->get_top_module());
            EXPECT_EQ(m_0, nullptr);
        }*/

    TEST_END
}

/**
 * Testing the get_module_by_id function
 *
 * Functions: get_module_by_id
 */
TEST_F(netlist_test, check_get_module_by_id)
{
    TEST_START
        // Positive
        {
            // Testing the access on a module by its name
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_123 = nl->create_module(MIN_MODULE_ID+123, "module_123", nl->get_top_module());
            if (m_123 != nullptr) {
                std::shared_ptr<module> m_123_by_id = nl->get_module_by_id(MIN_MODULE_ID+123);
                EXPECT_EQ(m_123_by_id, m_123);

            }
        }
        {
            // Testing the access on the topmodule by its id
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> tm = nl->get_top_module();
            if (tm != nullptr) {
                std::shared_ptr<module> tm_by_id = nl->get_module_by_id(tm->get_id());
                EXPECT_EQ(tm, tm_by_id);

            }
        }
        // Negative
        {
            // The passed id is not taken
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0,"module_0", nl->get_top_module());

            EXPECT_EQ(nl->get_module_by_id(MIN_MODULE_ID+123), nullptr);
        }
    TEST_END
}

/**
* Testing the deletion of modules from the netlist. Verification of success by is_module_in_netlist
*
* Functions: delete_module, is_module_in_netlist
*/
TEST_F(netlist_test, check_delete_module)
{
    TEST_START
        // Positive
        {
            // Add a module and delete it after
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0, "module_0", nl->get_top_module());
            nl->delete_module(m_0);
            EXPECT_FALSE(nl->is_module_in_netlist(m_0));
        }
        {
            // Remove modules which own submodules
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> parent = nl->create_module(MIN_MODULE_ID+0, "module_0", nl->get_top_module());
            std::shared_ptr<module> test_module = nl->create_module(MIN_MODULE_ID+1, "module_1", parent);
            std::shared_ptr<module> child = nl->create_module(MIN_MODULE_ID+2, "module_2", test_module);

            // Add a net and a gate to the test_module
            std::shared_ptr<gate> gate_0 = nl->create_gate(get_gate_type_by_name("gate_1_to_1"), "gate_0");
            std::shared_ptr<net> net_0 = nl->create_net("net_0");
            test_module->assign_gate(gate_0);
            //test_module->assign_net(net_0);

            nl->delete_module(test_module);

            EXPECT_FALSE(nl->is_module_in_netlist(test_module));
            EXPECT_TRUE(parent->contains_gate(gate_0));
            //EXPECT_TRUE(parent->contains_net(net_0));
            EXPECT_TRUE(parent->get_submodules().find(child) != parent->get_submodules().end());


        }

        // NEGATIVE
        /*{
            // Deleted module is part of another netlist
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<netlist> nl_other = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl_other->create_module(MIN_MODULE_ID+0, "module_0", nl->get_top_module());
            nl->delete_module(m_0);
            EXPECT_FALSE(nl->is_module_in_netlist(m_0));
            EXPECT_TRUE(nl_other->is_module_in_netlist(m_0));
        }*/
        {
            // Try to delete the top module
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> tm = nl->get_top_module();
            nl->delete_module(tm);
            EXPECT_TRUE(nl->is_module_in_netlist(tm));
        }

    TEST_END
}

/**
* Testing the function is_module_in_netlist
*
* Functions: is_module_in_netlist
*/
TEST_F(netlist_test, check_is_module_in_netlist)
{
    TEST_START
        // Positive
        {
            // Add a module and check if it is in the netlist
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0, "module_0", nl->get_top_module());
            EXPECT_TRUE(nl->is_module_in_netlist(m_0));
        }
        /*{
            // Create a module, delete it and create a new module with the same id and check if the !old_one! is in the netlist
            // ISSUE: fails
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0_old = nl->create_module(MIN_MODULE_ID+0, "module_0_old", nl->get_top_module());
            nl->delete_module(m_0_old);
            std::shared_ptr<module> m_0_other = nl->create_module(MIN_MODULE_ID+0, "module_0_other", nl->get_top_module());
            EXPECT_FALSE(nl->is_module_in_netlist(m_0_old));
        }
        // Negative
        {
            // Pass a nullptr
            // ISSUE: fails (SIGSEGV)
            std::shared_ptr<netlist> nl = create_empty_netlist();
            EXPECT_TRUE(nl->is_module_in_netlist(nullptr));
        }*/
    TEST_END
}

/**
* Testing the function get_unique_module_id
*
* Functions: get_unique_module_id
*/
TEST_F(netlist_test, check_get_unique_module_id) {
    TEST_START
        // Positive
        {
            // Create some modules and get unique module ids
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0, "module_0", nl->get_top_module());
            std::shared_ptr<module> m_1 = nl->create_module(MIN_MODULE_ID+1, "module_1", nl->get_top_module());
            std::shared_ptr<module> m_3 = nl->create_module(MIN_MODULE_ID+3, "module_3", nl->get_top_module());
            std::set<u32> used_ids = {TOP_MODULE_ID,MIN_MODULE_ID+0,MIN_MODULE_ID+1,MIN_MODULE_ID+3};

            u32 new_module_id_1 = nl->get_unique_module_id();
            ASSERT_TRUE(used_ids.find(new_module_id_1) == used_ids.end());

            std::shared_ptr<module> m_new = nl->create_module(new_module_id_1, "module_new", nl->get_top_module());
            used_ids.insert(new_module_id_1);

            u32 new_module_id_2 = nl->get_unique_module_id();
            EXPECT_TRUE(used_ids.find(new_module_id_2) == used_ids.end());
        }
        {
            // Create some modules, delete some and get a unique module id (for testing the free_module_ids logic)
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0, "module_0", nl->get_top_module());
            std::shared_ptr<module> m_1 = nl->create_module(MIN_MODULE_ID+1, "module_1", nl->get_top_module());
            std::shared_ptr<module> m_2 = nl->create_module(MIN_MODULE_ID+2, "module_2", nl->get_top_module());
            std::shared_ptr<module> m_3 = nl->create_module(MIN_MODULE_ID+3, "module_3", nl->get_top_module());

            nl->delete_module(m_0);
            nl->delete_module(m_2);
            std::set<u32> used_ids = {TOP_MODULE_ID,MIN_MODULE_ID+1,MIN_MODULE_ID+3};

            u32 new_module_id_1 = nl->get_unique_module_id();
            ASSERT_TRUE(used_ids.find(new_module_id_1) == used_ids.end());

            std::shared_ptr<module> m_new = nl->create_module(new_module_id_1, "module_new", nl->get_top_module());
            used_ids.insert(new_module_id_1);

            u32 new_module_id_2 = nl->get_unique_module_id();
            EXPECT_TRUE(used_ids.find(new_module_id_2) == used_ids.end());
        }
    TEST_END
}

