#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/handler/netlist_handler.h"
#include "netlist/netlist.h"
#include "netlist/netlist_factory.h"
#include "test_def.h"
#include "gtest/gtest.h"
#include <core/log.h>
#include <iostream>
#include <netlist/gate.h>
#include <netlist/net.h>

namespace netlist_handler_test_counter
{
    std::vector<std::shared_ptr<netlist>> removed_netlists;
    std::vector<std::shared_ptr<netlist>> updated_netlists;
}    // namespace netlist_handler_test_counter

class netlist_handler_test : public ::testing::Test
{
protected:
    const std::string g_lib_name = "XILINX_SIMPRIM";

    virtual void SetUp()
    {
        test_utils::init_log_channels();
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

    /**
     * These functions are passed the register_update/remove_callback functions.
     * They store their passed netlist in a vector to check when they were called.
     *
     */
    static void add_removed_netlist(const std::shared_ptr<netlist> nl)
    {
        netlist_handler_test_counter::removed_netlists.push_back(nl);
    }

    static void add_updated_netlist(const std::shared_ptr<netlist> nl)
    {
        netlist_handler_test_counter::updated_netlists.push_back(nl);
    }

    /**
     * Reset the global vectors which contain the netlists which are passed the
     * add_removed/updated function
     */
    void reset_removed_netlists()
    {
        netlist_handler_test_counter::removed_netlists.clear();
    }

    void reset_updated_netlists()
    {
        netlist_handler_test_counter::updated_netlists.clear();
    }

    /**
     * Access on the global vectors removed_netlists/updated_netlists
     */

    std::vector<std::shared_ptr<netlist>> get_removed_netlists()
    {
        return netlist_handler_test_counter::removed_netlists;
    }

    std::vector<std::shared_ptr<netlist>> get_updated_netlists()
    {
        return netlist_handler_test_counter::updated_netlists;
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
 * Testing the registration of update_callbacks (called whenever data of a netlist is changed).
 * The notify_data_updated is called by the netlists.
 *
 * Functions: register_update_callback
 */
TEST_F(netlist_handler_test, check_data_updated){TEST_START{// Create 3 netlists
                                                            std::shared_ptr<netlist> netlist_0 = create_empty_netlist(0);
std::shared_ptr<netlist> netlist_1 = create_empty_netlist(1);
std::shared_ptr<netlist> netlist_2 = create_empty_netlist(2);

// Register the update callback
netlist_handler::enable(true);
netlist_handler::register_update_callback("test_update_callback", add_updated_netlist);

// Change the id of netlist_0 and netlist_1 (should call the callback)
{
    NO_COUT_TEST_BLOCK;
    netlist_0->set_id(10);
    netlist_1->set_id(11);
}

std::vector<std::shared_ptr<netlist>> exp_updated = {netlist_0, netlist_1};

EXPECT_TRUE(vectors_have_same_content(get_updated_netlists(), exp_updated));

netlist_handler::unregister_update_callback("test_update_callback");
}
TEST_END
}

/**
 * Testing the registration of remove_callbacks (called whenever a netlist is removed).
 * The notify_data_removed is called by the netlists.
 *
 * Functions: register_remove_callback
 */
TEST_F(netlist_handler_test, check_data_removed){TEST_START{// Create 3 netlists
                                                            std::shared_ptr<netlist> netlist_0 = create_empty_netlist(0);
std::shared_ptr<netlist> netlist_1 = create_empty_netlist(1);
std::shared_ptr<netlist> netlist_2 = create_empty_netlist(2);

// Register the remove callback
netlist_handler::enable(true);
netlist_handler::register_remove_callback("test_remove_callback", add_removed_netlist);

// Because no class uses the notify_data_removed function, the function is
// called manually
{
    NO_COUT_TEST_BLOCK;
    netlist_handler::notify_data_removed(netlist_0);
    netlist_handler::notify_data_removed(netlist_1);
}

std::vector<std::shared_ptr<netlist>> exp_removed = {netlist_0, netlist_1};

EXPECT_TRUE(vectors_have_same_content(get_removed_netlists(), exp_removed));

netlist_handler::unregister_remove_callback("test_remove_callback");
}
TEST_END
}

/**
 * Testing the unregister functions
 *
 * Functions: unregister_update_callback, unregister_remove_callback
 */
TEST_F(netlist_handler_test, check_unregistered){TEST_START{// Create a netlist
                                                            std::shared_ptr<netlist> netlist_0 = create_empty_netlist(0);

// Register the two callbacks and unregister them immediately
netlist_handler::enable(true);
netlist_handler::register_update_callback("test_update_callback", add_updated_netlist);
netlist_handler::register_remove_callback("test_remove_callback", add_removed_netlist);

netlist_handler::unregister_update_callback("test_update_callback");
netlist_handler::unregister_remove_callback("test_remove_callback");

// Update the netlist and call the notify_data_updated function manually (because no class uses it)
{
    NO_COUT_TEST_BLOCK;
    netlist_0->set_id(10);
    netlist_handler::notify_data_removed(netlist_0);
}

// The callback shouldn't be called
EXPECT_TRUE(get_updated_netlists().empty());
EXPECT_TRUE(get_removed_netlists().empty());
}
TEST_END
}

/**
 * Testing the callbacks with the enable flag set to false
 *
 * Functions: enable
 */
TEST_F(netlist_handler_test, check_enable)
{
    TEST_START
    {
        // Create a netlist
        std::shared_ptr<netlist> netlist_0 = create_empty_netlist(0);

        // Register the two callbacks but set enable to false
        netlist_handler::enable(false);
        netlist_handler::register_update_callback("test_update_callback", add_updated_netlist);
        netlist_handler::register_remove_callback("test_remove_callback", add_removed_netlist);

        // Update the netlist and call the notify_data_updated function manually (because no class uses it)
        {
            NO_COUT_BLOCK;
            netlist_0->set_id(10);
            netlist_handler::notify_data_removed(netlist_0);
        }

        // The callback shouldn't be called
        EXPECT_TRUE(get_updated_netlists().empty());
        EXPECT_TRUE(get_removed_netlists().empty());
    }
    TEST_END
}