#include "netlist/module.h"
#include "netlist/event_system/gate_event_handler.h"
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

class module_test : public ::testing::Test
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
 * Testing the access on the id, the type and the stored netlist after calling the constructor
 *
 * Functions: constructor, get_id, get_name
 */
TEST_F(module_test, check_constructor){
    TEST_START
        {
            // Creating a module of id 123 and type "test module"
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> test_module = nl->create_module(MIN_MODULE_ID+123, "test module", nl->get_top_module());

            EXPECT_EQ(test_module->get_id(), (u32)(MIN_MODULE_ID+123));
            EXPECT_EQ(test_module->get_name(), "test module");
            EXPECT_EQ(test_module->get_netlist(), nl);
        }
    TEST_END
}

/**
 * Testing the set_name function of module
 *
 * Functions: set_name
 */
TEST_F(module_test, check_set_id){
    TEST_START
        {
            // Set a new name for module
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> test_module = nl->create_module(MIN_MODULE_ID+0, "test_module", nl->get_top_module());

            test_module->set_name("new_name");
            EXPECT_EQ(test_module->get_name(), "new_name");
        }
        {
            // Set an already set name
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> test_module = nl->create_module(MIN_MODULE_ID+0, "test_module", nl->get_top_module());

            test_module->set_name("test_module");
            EXPECT_EQ(test_module->get_name(), "test_module");
        }
        {
            // Set an empty name
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> test_module = nl->create_module(MIN_MODULE_ID+0, "test_module", nl->get_top_module());

            test_module->set_name("");
            EXPECT_EQ(test_module->get_name(), "test_module");
        }
    TEST_END
}

/**
 * Testing the set_parent_module function
 *
 * Functions: set_parent_module
 */
TEST_F(module_test, check_set_parent_module){
    TEST_START
        // POSITIVE
        {
            /*  Consider the module scheme below. We set the parent_module of m_0 from the top_module to m_1
             *
             *                  .--> m_2
             *        .--> m_0 -|
             *   top -|         '--> m_3     ==>      top-.                 .--> m_2
             *        '--> m_1                            '-> m_1 --> m_0 --|
             *                                                              '--> m_3
             *
             */
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0, "test_module_0", nl->get_top_module());
            std::shared_ptr<module> m_1 = nl->create_module(MIN_MODULE_ID+1, "test_module_1", nl->get_top_module());
            std::shared_ptr<module> m_2 = nl->create_module(MIN_MODULE_ID+2, "test_module_2", m_0);
            std::shared_ptr<module> m_3 = nl->create_module(MIN_MODULE_ID+3, "test_module_3", m_0);

            m_0->set_parent_module(m_1);
            EXPECT_EQ(m_0->get_parent_module(), m_1);
            EXPECT_FALSE(m_1->get_submodules(module_name_filter("test_module_0"),false).empty());
            EXPECT_FALSE(m_1->get_submodules(module_name_filter("test_module_2"),true).empty());
            EXPECT_FALSE(m_1->get_submodules(module_name_filter("test_module_3"),true).empty());
        }
        {
            /*  Hang m_0 to one of its childs (m_1). m_1 should be connected to the top_module afterwards
             *
             *
             *               .--- m_1
             *  top --- m_0 -|              ==>     top --- m_1 --- m_0 --- m_2
             *               '--- m_2
             *
             */
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0, "test_module_0", nl->get_top_module());
            std::shared_ptr<module> m_1 = nl->create_module(MIN_MODULE_ID+1, "test_module_1", m_0);
            std::shared_ptr<module> m_2 = nl->create_module(MIN_MODULE_ID+2, "test_module_2", m_0);

            m_0->set_parent_module(m_1);
            EXPECT_EQ(m_1->get_parent_module(), nl->get_top_module());
            EXPECT_EQ(m_0->get_parent_module(), m_1);
            EXPECT_EQ(m_2->get_parent_module(), m_0);
        }
        // NEGATIVE
        /*{ // FAILS: new_parent == m_parent wrong! => new_parent == this
            // Hang a module to itself
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0, "test_module_0", nl->get_top_module());

            m_0->set_parent_module(m_0);
            EXPECT_EQ(m_0->get_parent_module(), nl->get_top_module());
        }*/
        {
            // Try to change the parent of the top_module
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0, "test_module_0", nl->get_top_module());

            nl->get_top_module()->set_parent_module(m_0);
            EXPECT_EQ(m_0->get_parent_module(), nl->get_top_module());
            EXPECT_EQ(nl->get_top_module()->get_parent_module(), nullptr);
        }
        /*{ // FAILS with SIGSEGV
            // new_parent is a nullptr
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0, "test_module_0", nl->get_top_module());

            m_0->set_parent_module(nullptr);
            nl->get_top_module()->set_parent_module(m_0);
            EXPECT_EQ(m_0->get_parent_module(), nl->get_top_module());
        }*/
        /*{ // FAILS (necessary?)
            // new_parent not part of the netlist (anymore)
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<netlist> nl_other = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0, "test_module_0", nl->get_top_module());
            std::shared_ptr<module> m_1 = nl->create_module(MIN_MODULE_ID+1, "test_module_1", nl->get_top_module());
            nl->delete_module(m_0); // m_0 is removed from the netlist

            m_1->set_parent_module(m_0);
            EXPECT_EQ(m_1->get_parent_module(), nl->get_top_module());
        }*/
    TEST_END
}

/**
 * Testing the contains_gate function
 *
 * Functions: contains_gate
 */
TEST_F(module_test, check_contains_gate){
    TEST_START
        // POSITIVE
        {
            // Check a gate, that is part of the module (not recursive)
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0, "test_module", nl->get_top_module());
            std::shared_ptr<gate> gate_0 = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("INV"), "gate_0");
            m_0->assign_gate(gate_0);

            EXPECT_TRUE(m_0->contains_gate(gate_0));
        }
        {
            // Check a gate, that isn't part of the module (not recursive)
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0, "test_module", nl->get_top_module());
            std::shared_ptr<gate> gate_0 = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("INV"), "gate_0");

            EXPECT_FALSE(m_0->contains_gate(gate_0));
        }
        {
            // Check a gate, that isn't part of the module, but of a submodule (not recursive)
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0, "test_module", nl->get_top_module());
            std::shared_ptr<module> submodule = nl->create_module(MIN_MODULE_ID+1, "test_module", m_0);
            ASSERT_NE(submodule, nullptr);
            std::shared_ptr<gate> gate_0 = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("INV"), "gate_0");
            submodule->assign_gate(gate_0);

            EXPECT_FALSE(m_0->contains_gate(gate_0));
        }
        {
            // Check a gate, that isn't part of the module, but of a submodule (recursive)
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0, "test_module", nl->get_top_module());
            std::shared_ptr<module> submodule = nl->create_module(MIN_MODULE_ID+1, "test_module", m_0);
            ASSERT_NE(submodule, nullptr);
            std::shared_ptr<gate> gate_0 = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("INV"), "gate_0");
            submodule->assign_gate(gate_0);

            EXPECT_TRUE(m_0->contains_gate(gate_0, true));
        }
    TEST_END
}

/**
 * Testing the addition of gates to the module. Verify the addition by call the
 * get_gates function and the contains_gate function
 *
 * Functions: assign_gate
 */
TEST_F(module_test, check_assign_gate){
    TEST_START
        {
            // Add some gates to the module
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<gate> gate_0 = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("INV"), "gate_0");
            std::shared_ptr<gate> gate_1 = nl->create_gate(MIN_GATE_ID+1, get_gate_type_by_name("INV"), "gate_1");
            // this gate is not part of the module
            std::shared_ptr<gate> gate_not_in_m = nl->create_gate(MIN_GATE_ID+2, get_gate_type_by_name("INV"), "gate_not_in_m");

            // Add gate_0 and gate_1 to a module
            std::shared_ptr<module> test_module = nl->create_module(MIN_MODULE_ID+0, "test module", nl->get_top_module());
            test_module->assign_gate(gate_0);
            test_module->assign_gate(gate_1);

            std::set<std::shared_ptr<gate>> expRes = {gate_0, gate_1};

            EXPECT_EQ(test_module->get_gates(), expRes);
            EXPECT_TRUE(test_module->contains_gate(gate_0));
            EXPECT_TRUE(test_module->contains_gate(gate_1));
            EXPECT_FALSE(test_module->contains_gate(gate_not_in_m));
        }
        {
            // Add the same gate twice to the module
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl  = create_empty_netlist();
            std::shared_ptr<gate> gate_0 = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("INV"), "gate_0");

            // Add gate_0 twice
            std::shared_ptr<module> test_module = nl->create_module(MIN_MODULE_ID+0, "test module", nl->get_top_module());
            test_module->assign_gate(gate_0);
            test_module->assign_gate(gate_0);

            std::set<std::shared_ptr<gate>> expRes = {
                    gate_0,
            };

            EXPECT_EQ(test_module->get_gates(), expRes);
            EXPECT_TRUE(test_module->contains_gate(gate_0));
        }
        {
            // Insert a gate owned by a submodule
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl  = create_empty_netlist();
            std::shared_ptr<gate> gate_0 = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("INV"), "gate_0");

            std::shared_ptr<module> test_module = nl->create_module(MIN_MODULE_ID+0, "test module", nl->get_top_module());
            std::shared_ptr<module> submodule = nl->create_module(MIN_MODULE_ID+1, "submodule", test_module);
            submodule->assign_gate(gate_0);
            ASSERT_TRUE(submodule->contains_gate(gate_0));
            ASSERT_FALSE(test_module->contains_gate(gate_0));

            test_module->assign_gate(gate_0);

            std::set<std::shared_ptr<gate>> expRes = {
                    gate_0
            };

            EXPECT_EQ(test_module->get_gates(), expRes);
            EXPECT_FALSE(submodule->contains_gate(gate_0));
        }

        // NEGATIVE
        {
            // Gate is a nullptr
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> test_module = nl->create_module(MIN_MODULE_ID+0, "test module", nl->get_top_module());
            test_module->assign_gate(nullptr);
            EXPECT_TRUE(test_module->get_gates().empty());
        }
    TEST_END
}

/**
 * Testing the deletion of gates from modules
 *
 * Functions: remove_gate
 */
TEST_F(module_test, check_remove_gate){
    TEST_START
        {
            // Delete a gate from a module (gate owned by the modules)
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0, "test_module", nl->get_top_module());
            std::shared_ptr<gate> gate_0 = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("INV"), "gate_0");
            m_0->assign_gate(gate_0);

            ASSERT_TRUE(m_0->contains_gate(gate_0));
            m_0->remove_gate(gate_0);
            EXPECT_FALSE(m_0->contains_gate(gate_0));
        }
        {
            // Try to delete a gate from a module (gate owned by another module)
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0, "test_module", nl->get_top_module());
            std::shared_ptr<module> m_other = nl->create_module(MIN_MODULE_ID+1, "other_test_module", nl->get_top_module());
            std::shared_ptr<gate> gate_0 = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("INV"), "gate_0");
            m_other->assign_gate(gate_0);

            m_0->remove_gate(gate_0);
            EXPECT_FALSE(m_0->contains_gate(gate_0));
            EXPECT_TRUE(m_other->contains_gate(gate_0));
        }
        // NEGATIVE
        {
            // Try to delete a gate from the top-module (should change nothing)
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<gate> gate_0 = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("INV"), "gate_0");
            std::shared_ptr<module> tm =  nl->get_top_module();

            ASSERT_TRUE(tm->contains_gate(gate_0));
            tm->remove_gate(gate_0);
            EXPECT_TRUE(tm->contains_gate(gate_0));
        }
        {
            // Try to delete a nullptr (should not crash)
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0, "test_module", nl->get_top_module());

            m_0->remove_gate(nullptr);
        }
    TEST_END
}

/**
 * Testing the get_gate_by_id function
 *
 * Functions: get_gate_by_id
 */
TEST_F(module_test, check_get_gate_by_id){
    TEST_START
        // POSITIVE
        {
            // get a gate by its id (gate owned by module)(not recursive)
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0, "test_module", nl->get_top_module());
            std::shared_ptr<gate> gate_123 = nl->create_gate(MIN_GATE_ID+123, get_gate_type_by_name("INV"), "gate_123");
            m_0->assign_gate(gate_123);

            ASSERT_TRUE(m_0->contains_gate(gate_123));
            EXPECT_EQ(m_0->get_gate_by_id(MIN_GATE_ID+123), gate_123);
        }
        {
            // get a gate by its id (not owned by a submodule)(not recursive)
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0, "test_module", nl->get_top_module());
            std::shared_ptr<module> submodule = nl->create_module(MIN_MODULE_ID+1, "other_module", m_0);
            std::shared_ptr<gate> gate_123 = nl->create_gate(MIN_GATE_ID+123, get_gate_type_by_name("INV"), "gate_123");
            submodule->assign_gate(gate_123);

            EXPECT_EQ(m_0->get_gate_by_id(MIN_GATE_ID+123), nullptr);
        }
        {
            // get a gate by its id (not owned by a submodule)(recursive)
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0, "test_module", nl->get_top_module());
            std::shared_ptr<module> submodule = nl->create_module(MIN_MODULE_ID+1, "other_module", m_0);
            std::shared_ptr<gate> gate_123 = nl->create_gate(MIN_GATE_ID+123, get_gate_type_by_name("INV"), "gate_123");
            submodule->assign_gate(gate_123);

            EXPECT_EQ(m_0->get_gate_by_id(MIN_GATE_ID+123, true), gate_123);
        }
    TEST_END
}

/**
 * Testing the access on submodules as well as the contains module function. Therefore we build up a module tree like this:
 *
 *               .----> MODULE_0
 *               |
 * TOP_MODULE ---+                .--> MODULE_2
 *               |                |
 *               '----> MODULE_1 -+
 *                                |
 *                                '--> MODULE_3
 *
 *   (Remark: MODULE_0 and MODULE_2 are both named "even_module", while MODULE_1 and MODULE_3 are named "odd_module")
 *
 * Functions: get_submodules, contains_module
 */
TEST_F(module_test, check_get_submodules){
    TEST_START
        // Set up the module tree
        std::shared_ptr<netlist> nl = create_empty_netlist();
        std::shared_ptr<module> tm = nl->get_top_module();
        ASSERT_NE(tm, nullptr);
        std::shared_ptr<module> m_0 = nl->create_module(MIN_MODULE_ID+0, "even_module", tm);
        ASSERT_NE(m_0, nullptr);
        std::shared_ptr<module> m_1 = nl->create_module(MIN_MODULE_ID+1, "odd_module", tm);
        ASSERT_NE(m_1, nullptr);
        std::shared_ptr<module> m_2 = nl->create_module(MIN_MODULE_ID+2, "even_module", m_1);
        ASSERT_NE(m_2, nullptr);
        std::shared_ptr<module> m_3 = nl->create_module(MIN_MODULE_ID+3, "odd_module", m_1);
        ASSERT_NE(m_3, nullptr);
        {
            // Testing the access on submodules (no module_name_filter, not recursive)
            {
                // Submodules of TOP_MODULE;
                std::set<std::shared_ptr<module>> exp_result = {m_0, m_1};
                EXPECT_EQ(tm->get_submodules(nullptr, false), exp_result);
                EXPECT_TRUE(tm->contains_module(m_0, false));
                EXPECT_TRUE(tm->contains_module(m_1, false));
                EXPECT_FALSE(tm->contains_module(m_2, false));
                EXPECT_FALSE(tm->contains_module(m_3, false));
            }
            {
                // Submodules of MODULE_1;
                std::set<std::shared_ptr<module>> exp_result = {m_2, m_3};
                EXPECT_EQ(m_1->get_submodules(nullptr, false), exp_result);
            }
            {
                // Submodules of MODULE_0;
                std::set<std::shared_ptr<module>> exp_result = {};
                EXPECT_EQ(m_0->get_submodules(nullptr, false), exp_result);
            }
        }
        {
            // Testing the access on submodules (module_name_filter set, not recursive)
            {
                // Submodules of TOP_MODULE;
                std::set<std::shared_ptr<module>> exp_result = {m_0};
                EXPECT_EQ(tm->get_submodules(module_name_filter("even_module"), false), exp_result);
            }
            {
                // Submodules of MODULE_1;
                std::set<std::shared_ptr<module>> exp_result = {m_2};
                EXPECT_EQ(m_1->get_submodules(module_name_filter("even_module"), false), exp_result);
            }
            {
                // Submodules of TOP_MODULE (name does not exists);
                std::set<std::shared_ptr<module>> exp_result = {};
                EXPECT_EQ(tm->get_submodules(module_name_filter("non_existing_name"), false), exp_result);
            }
        }
        {
            // Testing the access on submodules (recursive)
            {
                // Submodules of TOP_MODULE;
                std::set<std::shared_ptr<module>> exp_result = {m_0,m_1,m_2,m_3};
                EXPECT_EQ(tm->get_submodules(nullptr, true), exp_result);
                EXPECT_TRUE(tm->contains_module(m_0, true));
                EXPECT_TRUE(tm->contains_module(m_1, true));
                EXPECT_TRUE(tm->contains_module(m_2, true));
                EXPECT_TRUE(tm->contains_module(m_3, true));
            }
            {
                // Submodules of TOP_MODULE (with module_name_filter);
                std::set<std::shared_ptr<module>> exp_result = {m_0,m_2};
                EXPECT_EQ(tm->get_submodules(module_name_filter("even_module"), true), exp_result);
            }
            {
                // Submodules of MODULE_0
                std::set<std::shared_ptr<module>> exp_result = {};
                EXPECT_EQ(m_0->get_submodules(nullptr, true), exp_result);
            }
        }
        {
            // Testing edge cases of contains_module

            // -- the passed module is a nullptr
            EXPECT_FALSE(tm->contains_module(nullptr, false));
            // -- the calling module is a leave
            EXPECT_FALSE(m_2->contains_module(tm, false));
            // -- the passed module is the same as the calling one
            EXPECT_FALSE(m_2->contains_module(m_2, false));
        }
    TEST_END
}

/*
 *      Testing the get_input_nets, get_output_nets, get_internal_nets by using the following example netlist with a module
 *
 *                     ################################################
 *                     # TEST_MODULE                                  #
 *                     #                                              #
 *      global_in -----§---------------=  INV (0)  = -----------------§----- global_out
 *                     #                                              #
 *                 .---§--= INV (1) =--=                              #
 *                 |   #                  AND2 (2) =--+---------------§----= INV (5)
 *    = INV (4) =--+---§---------------=              |               #
 *                     #                              '--= INV (3) =  #
 *                     #                                              #
 *                     ################################################
 *
 *
 */

/**
 * Testing the get_input_nets, get_output_nets and get_internal_nets function
 *
 * Functions: get_input_nets, get_output_nets, get_internal_nets
 */
TEST_F(module_test, check_get_input_nets){
    TEST_START
        // +++ Create the example netlist (see above)

        std::shared_ptr<netlist> nl = create_empty_netlist();

        // Add the gates
        std::shared_ptr<gate> gate_0 = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("INV") , "gate_0");
        std::shared_ptr<gate> gate_1 = nl->create_gate(MIN_GATE_ID+1, get_gate_type_by_name("INV") , "gate_1");
        std::shared_ptr<gate> gate_2 = nl->create_gate(MIN_GATE_ID+2, get_gate_type_by_name("AND2"), "gate_2");
        std::shared_ptr<gate> gate_3 = nl->create_gate(MIN_GATE_ID+3, get_gate_type_by_name("INV") , "gate_3");
        std::shared_ptr<gate> gate_4 = nl->create_gate(MIN_GATE_ID+4, get_gate_type_by_name("INV") , "gate_4");
        std::shared_ptr<gate> gate_5 = nl->create_gate(MIN_GATE_ID+5, get_gate_type_by_name("INV") , "gate_5");

        // Add the nets (net_x_y1_y2_... is net from x to y1,y2,... (g = global input/output))
        std::shared_ptr<net> net_g_0   = nl->create_net(MIN_NET_ID+0, "name_0");
        std::shared_ptr<net> net_0_g   = nl->create_net(MIN_NET_ID+1, "name_0");
        std::shared_ptr<net> net_1_2   = nl->create_net(MIN_NET_ID+3, "name_0");
        std::shared_ptr<net> net_4_1_2 = nl->create_net(MIN_NET_ID+4, "name_1");
        std::shared_ptr<net> net_2_3_5 = nl->create_net(MIN_NET_ID+5, "name_1");

        // Connect the nets
        net_g_0->add_dst(gate_0, "I");

        net_0_g->set_src(gate_0, "O");

        net_4_1_2->set_src(gate_4, "O");
        net_4_1_2->add_dst(gate_1, "I");
        net_4_1_2->add_dst(gate_2, "I1");

        net_1_2->set_src(gate_1, "O");
        net_1_2->add_dst(gate_2, "I0");

        net_2_3_5->set_src(gate_2, "O");
        net_2_3_5->add_dst(gate_3, "I");
        net_2_3_5->add_dst(gate_5, "I");

        // Mark global nets
        nl->mark_global_input_net(net_g_0);
        nl->mark_global_output_net(net_0_g);

        // Create the module
        std::shared_ptr<module> test_module = nl->create_module(MIN_MODULE_ID+0, "test_module", nl->get_top_module());
        for(auto g : std::set<std::shared_ptr<gate>>({gate_0, gate_1, gate_2, gate_3})){
            test_module->assign_gate(g);
        }
        {
            // Get input nets of the test module
            std::set<std::shared_ptr<net>> exp_result = {net_g_0, net_4_1_2};
            EXPECT_EQ(test_module->get_input_nets(), exp_result);
        }
        {
            // Get output nets of the test module
            std::set<std::shared_ptr<net>> exp_result = {net_0_g, net_2_3_5};
            EXPECT_EQ(test_module->get_output_nets(), exp_result);
        }
        {
            // Get internal nets of the test module
            std::set<std::shared_ptr<net>> exp_result = {net_1_2, net_2_3_5};
            EXPECT_EQ(test_module->get_internal_nets(), exp_result);
        }

    TEST_END
}


