#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/handler/net_handler.h"
#include "netlist/netlist.h"
#include "netlist/netlist_factory.h"
#include "test_def.h"
#include "gtest/gtest.h"
#include <core/log.h>
#include <iostream>
#include <netlist/gate.h>
#include <netlist/net.h>

using namespace hal;

namespace net_handler_test_counter
{
    std::vector<std::shared_ptr<Net>> removed_nets;
    std::vector<std::shared_ptr<Net>> updated_nets;
}    // namespace net_handler_test_counter

class net_handler_test : public ::testing::Test
{
protected:
    const std::string g_lib_name = "XILINX_SIMPRIM";

    virtual void SetUp()
    {
        NO_COUT_BLOCK;
        reset_removed_nets();
        reset_updated_nets();
    }

    virtual void TearDown()
    {
    }

    // Creates an empty netlist with a certain id if passed
    std::shared_ptr<Netlist> create_empty_netlist(const int id = -1)
    {
        NO_COUT_BLOCK;
        std::shared_ptr<GateLibrary> gl = gate_library_manager::get_gate_library(g_lib_name);
        std::shared_ptr<Netlist> nl(new Netlist(gl));

        if (id >= 0)
        {
            nl->set_id(id);
        }
        return nl;
    }

    /**
     * These functions are passed the register_update/remove_callback functions.
     * They store their passed nets in a vector to check when they were called.
     *
     */
    static void add_removed_net(const std::shared_ptr<Net> n)
    {
        net_handler_test_counter::removed_nets.push_back(n);
    }

    static void add_updated_net(const std::shared_ptr<Net> n)
    {
        net_handler_test_counter::updated_nets.push_back(n);
    }

    /**
     * Reset the global vectors which contain the nets which are passed the
     * add_removed/updated function
     */
    void reset_removed_nets()
    {
        net_handler_test_counter::removed_nets.clear();
    }

    void reset_updated_nets()
    {
        net_handler_test_counter::updated_nets.clear();
    }

    /**
     * Access on the global vectors removed_nets/updated_nets
     */

    std::vector<std::shared_ptr<Net>> get_removed_nets()
    {
        return net_handler_test_counter::removed_nets;
    }

    std::vector<std::shared_ptr<Net>> get_updated_nets()
    {
        return net_handler_test_counter::updated_nets;
    }

    // Checks if two vectors have the same content regardless of their order
    template<typename T>
    bool vectors_have_same_content(std::vector<T> vec_1, std::vector<T> vec_2)
    {
        if (vec_1.size() != vec_2.size())
            return false;

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
};

/**
 * Testing the registration of update_callbacks (called whenever data of a Net is changed).
 * The notify_data_updated is called by the nets.
 *
 * Functions: register_update_callback
 */
TEST_F(net_handler_test, check_data_updated){TEST_START{// Create a netlist with 3 nets
                                                        std::shared_ptr<Netlist> nl = create_empty_netlist(0);
std::shared_ptr<Net> net_0(new Net(nl, 0, "net_0"));
std::shared_ptr<Net> net_1(new Net(nl, 1, "net_1"));
std::shared_ptr<Net> net_2(new Net(nl, 2, "net_2"));
nl->add_net(net_0);
nl->add_net(net_1);
nl->add_net(net_2);

// Register the update callback
net_handler::enable(true);
net_handler::register_update_callback("test_update_callback", add_updated_net);

// Change the name of net_0 and net_1 (should call the callback)
{
    NO_COUT_TEST_BLOCK;
    net_0->set_name("new_name_0");
    net_1->set_name("new_name_1");
}

std::vector<std::shared_ptr<Net>> exp_updated = {net_0, net_1};

EXPECT_TRUE(vectors_have_same_content(get_updated_nets(), exp_updated));

net_handler::unregister_update_callback("test_update_callback");
}
TEST_END
}

/**
 * Testing the registration of remove_callbacks (called whenever a Net is removed).
 * The notify_data_removed is called by the nets.
 *
 * Functions: register_remove_callback
 */
TEST_F(net_handler_test, check_data_removed){TEST_START{// Create a netlist with 3 nets
                                                        std::shared_ptr<Netlist> nl = create_empty_netlist(0);
std::shared_ptr<Net> net_0(new Net(nl, 0, "net_0"));
std::shared_ptr<Net> net_1(new Net(nl, 1, "net_1"));
std::shared_ptr<Net> net_2(new Net(nl, 2, "net_2"));
nl->add_net(net_0);
nl->add_net(net_1);
nl->add_net(net_2);

// Register the remove callback
net_handler::enable(true);
net_handler::register_remove_callback("test_remove_callback", add_removed_net);

// Delete net_0 and net_1 from the netlist
{
    NO_COUT_TEST_BLOCK;
    nl->delete_net(net_0);
    nl->delete_net(net_1);
}

std::vector<std::shared_ptr<Net>> exp_removed = {net_0, net_1};

EXPECT_TRUE(vectors_have_same_content(get_removed_nets(), exp_removed));

net_handler::unregister_remove_callback("test_remove_callback");
}
TEST_END
}

/**
 * Testing the unregister functions
 *
 * Functions: unregister_update_callback, unregister_remove_callback
 */
TEST_F(net_handler_test, check_unregister){TEST_START{// Create a netlist with one Net
                                                      std::shared_ptr<Netlist> nl = create_empty_netlist(0);
std::shared_ptr<Net> net_0(new Net(nl, 0, "net_0"));
nl->add_net(net_0);

// Register the two callbacks and unregister them immediately
net_handler::enable(true);
net_handler::register_update_callback("test_update_callback", add_updated_net);
net_handler::register_remove_callback("test_remove_callback", add_removed_net);

net_handler::unregister_update_callback("test_update_callback");
net_handler::unregister_remove_callback("test_remove_callback");

// Update and remove the Net
{
    NO_COUT_TEST_BLOCK;
    net_0->set_name("new_name");
    nl->delete_net(net_0);
}

// The callback shouldn't be called
EXPECT_TRUE(get_updated_nets().empty());
EXPECT_TRUE(get_removed_nets().empty());
}
TEST_END
}

/**
 * Testing the callbacks with the enable flag set to false
 *
 * Functions: enable
 */
TEST_F(net_handler_test, check_enable)
{
    TEST_START
    {
        // Create a netlist with one Net
        std::shared_ptr<Netlist> nl = create_empty_netlist(0);
        std::shared_ptr<Net> net_0(new Net(nl, 0, "net_0"));
        nl->add_net(net_0);

        // Register the two callbacks but set enable to false
        net_handler::enable(false);
        net_handler::register_update_callback("test_update_callback", add_updated_net);
        net_handler::register_remove_callback("test_remove_callback", add_removed_net);

        // Update and remove the Net
        {
            NO_COUT_TEST_BLOCK;
            net_0->set_name("new_name");
            nl->delete_net(net_0);
        }

        // The callback shouldn't be called
        EXPECT_TRUE(get_updated_nets().empty());
        EXPECT_TRUE(get_removed_nets().empty());
    }
    TEST_END
}
