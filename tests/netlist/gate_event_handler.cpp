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

namespace hal
{
namespace gate_event_handler_test_counter
{
    std::vector<std::shared_ptr<Gate>> removed_gates;
    std::vector<std::shared_ptr<Gate>> updated_gates;
}    // namespace gate_event_handler_test_counter

class gate_event_handler_test : public ::testing::Test
{
protected:
    const std::string g_lib_name = "XILINX_SIMPRIM";

    virtual void SetUp()
    {
        NO_COUT_BLOCK;
        reset_removed_gates();
        reset_updated_gates();
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
     * They store their passed Gate in a vector to check when they were called.
     *
     */
    static void add_removed_gate(const std::shared_ptr<Gate> g)
    {
        gate_event_handler_test_counter::removed_gates.push_back(g);
    }

    static void add_updated_gate(const std::shared_ptr<Gate> g)
    {
        gate_event_handler_test_counter::updated_gates.push_back(g);
    }

    /**
     * Reset the global vectors which contain the gates which are passed the
     * add_removed/updated function
     */
    void reset_removed_gates()
    {
        gate_event_handler_test_counter::removed_gates.clear();
    }

    void reset_updated_gates()
    {
        gate_event_handler_test_counter::updated_gates.clear();
    }

    /**
     * Access on the global vectors removed_gates/updated_gates
     */

    std::vector<std::shared_ptr<Gate>> get_removed_gates()
    {
        return gate_event_handler_test_counter::removed_gates;
    }

    std::vector<std::shared_ptr<Gate>> get_updated_gates()
    {
        return gate_event_handler_test_counter::updated_gates;
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
 * Testing the registration of update_callbacks (called whenever data of a Gate is changed).
 * The notify_data_updated is called by the gates.
 *
 * Functions: register_update_callback
 */
TEST_F(gate_event_handler_test, check_data_updated){TEST_START{// Create a netlist with 3 gates
                                                               std::shared_ptr<Netlist> nl = create_empty_netlist(0);
std::shared_ptr<Gate> gate_0 = nl->create_gate(0, "X_INV", "gate_0");
std::shared_ptr<Gate> gate_1 = nl->create_gate(1, "X_INV", "gate_1");
std::shared_ptr<Gate> gate_2 = nl->create_gate(2, "X_INV", "gate_2");

// Register the update callback
gate_event_handler::enable(true);
gate_event_handler::register_callback("test_update_callback", std::bind(gate_event_handler_test::add_updated_gate));

// Change the name of gate_0 and gate_1 (should call the callback)
{
    NO_COUT_TEST_BLOCK;
    gate_0->set_name("new_name_0");
    gate_1->set_name("new_name_1");
}

std::vector<std::shared_ptr<Gate>> exp_updated = {gate_0, gate_1};

EXPECT_TRUE(vectors_have_same_content(get_updated_gates(), exp_updated));

gate_event_handler::unregister_callback("test_update_callback");
}
TEST_END
}

/**
 * Testing the registration of remove_callbacks (called whenever a Gate is removed).
 * The notify_data_removed is called by the gates.
 *
 * Functions: register_remove_callback
 */
TEST_F(gate_event_handler_test, check_data_removed){TEST_START{// Create a netlist with 3 gates
                                                               std::shared_ptr<Netlist> nl = create_empty_netlist(0);
std::shared_ptr<Gate> gate_0 = nl->create_gate(0, "X_INV", "gate_0");
std::shared_ptr<Gate> gate_1 = nl->create_gate(1, "X_INV", "gate_1");
std::shared_ptr<Gate> gate_2 = nl->create_gate(2, "X_INV", "gate_2");
nl->add_gate(gate_0);
nl->add_gate(gate_1);
nl->add_gate(gate_2);

// Register the remove callback
gate_event_handler::enable(true);
gate_event_handler::register_remove_callback("test_remove_callback", add_removed_gate);

// Delete gate_0 and gate_1 from the netlist
{
    NO_COUT_TEST_BLOCK;
    nl->delete_gate(gate_0);
    nl->delete_gate(gate_1);
}

std::vector<std::shared_ptr<Gate>> exp_removed = {gate_0, gate_1};

EXPECT_TRUE(vectors_have_same_content(get_removed_gates(), exp_removed));

gate_event_handler::unregister_remove_callback("test_remove_callback");
}
TEST_END
}

/**
 * Testing the unregister functions
 *
 * Functions: unregister_update_callback, unregister_remove_callback
 */
TEST_F(gate_event_handler_test, check_unregister){TEST_START{// Create a netlist with one Gate
                                                             std::shared_ptr<Netlist> nl = create_empty_netlist(0);
std::shared_ptr<Gate> gate_0 = nl->create_gate(0, "X_INV", "gate_0");
nl->add_gate(gate_0);

// Register the two callbacks and unregister them immediately
gate_event_handler::enable(true);
gate_event_handler::register_update_callback("test_update_callback", add_updated_gate);
gate_event_handler::register_remove_callback("test_remove_callback", add_removed_gate);

gate_event_handler::unregister_update_callback("test_update_callback");
gate_event_handler::unregister_remove_callback("test_remove_callback");

// Update and remove the Gate
{
    NO_COUT_TEST_BLOCK;
    gate_0->set_name("new_name");
    nl->delete_gate(gate_0);
}

// The callback shouldn't be called
EXPECT_TRUE(get_updated_gates().empty());
EXPECT_TRUE(get_removed_gates().empty());
}
TEST_END
}

/**
 * Testing the callbacks with the enable flag set to false
 *
 * Functions: enable
 */
TEST_F(gate_event_handler_test, check_enable)
{
    TEST_START
    {
        // Create a netlist with one Gate
        std::shared_ptr<Netlist> nl  = create_empty_netlist(0);
        std::shared_ptr<Gate> gate_0 = nl->create_gate(0, "X_INV", "gate_0");
        nl->add_gate(gate_0);

        // Register the two callbacks but set enable to false
        gate_event_handler::enable(false);
        gate_event_handler::register_update_callback("test_update_callback", add_updated_gate);
        gate_event_handler::register_remove_callback("test_remove_callback", add_removed_gate);

        // Update and remove the Gate
        {
            NO_COUT_TEST_BLOCK;
            gate_0->set_name("new_name");
            nl->delete_gate(gate_0);
        }

        // The callback shouldn't be called
        EXPECT_TRUE(get_updated_gates().empty());
        EXPECT_TRUE(get_removed_gates().empty());
    }
    TEST_END
}
}
