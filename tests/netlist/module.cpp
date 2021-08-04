#include "hal_core/netlist/module.h"
#include "hal_core/netlist/event_system/event_handler.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "netlist_test_utils.h"

namespace hal {

    using test_utils::MIN_MODULE_ID;
    using test_utils::MIN_GATE_ID;
    using test_utils::MIN_NET_ID;

    class ModuleTest : public ::testing::Test {
    protected:
        virtual void SetUp() 
        {
            test_utils::init_log_channels();
        }

        virtual void TearDown() 
        {
        }
    };

    /**
     * Test basic functionality.
     *
     * Functions: constructor, get_id, get_name, is_top_module
     */
    TEST_F(ModuleTest, check_constructor) {
        TEST_START
            {
                std::unique_ptr<Netlist> netlist = test_utils::create_empty_netlist();
                
                Module* top_module = netlist->get_top_module();
                ASSERT_NE(top_module, nullptr);
                EXPECT_EQ(top_module->get_id(), 1);
                EXPECT_EQ(top_module->get_name(), "top_module");
                EXPECT_TRUE(top_module->is_top_module());
                EXPECT_TRUE(top_module->get_type().empty());
                EXPECT_EQ(top_module->get_parent_module(), nullptr);

                Module* dummy_module = netlist->create_module(12, "dummy_module", top_module);
                ASSERT_NE(dummy_module, nullptr);
                EXPECT_FALSE(dummy_module->is_top_module());
                EXPECT_EQ(dummy_module->get_id(), 12);
                EXPECT_EQ(dummy_module->get_name(), "dummy_module");
                EXPECT_EQ(dummy_module->get_netlist(), netlist.get());
                EXPECT_TRUE(dummy_module->get_type().empty());
                EXPECT_EQ(dummy_module->get_parent_module(), top_module);
            }
        TEST_END
    }

    /**
     * Test operators for equality and inequality.
     * 
     * Functions: operator==, operator!=
     */
    TEST_F(ModuleTest, check_operators)
    {
        TEST_START
        {
            std::unique_ptr<Netlist> nl_1 = test_utils::create_empty_netlist();
            ASSERT_NE(nl_1, nullptr);
            std::unique_ptr<Netlist> nl_2 = test_utils::create_empty_netlist();
            ASSERT_NE(nl_2, nullptr);
            const GateLibrary* gl = nl_1->get_gate_library();
            ASSERT_NE(gl, nullptr);
            ASSERT_EQ(gl, nl_2->get_gate_library());

            Gate* nl1_dummy_1 = nl_1->create_gate(gl->get_gate_type_by_name("BUF"), "dummy_1");
            ASSERT_NE(nl1_dummy_1, nullptr);
            Gate* nl1_dummy_2 = nl_1->create_gate(gl->get_gate_type_by_name("BUF"), "dummy_2");
            ASSERT_NE(nl1_dummy_2, nullptr);
            Gate* nl1_dummy_3 = nl_1->create_gate(gl->get_gate_type_by_name("BUF"), "dummy_3");
            ASSERT_NE(nl1_dummy_3, nullptr);
            Gate* nl2_dummy_1 = nl_2->create_gate(gl->get_gate_type_by_name("BUF"), "dummy_1");
            ASSERT_NE(nl2_dummy_1, nullptr);
            Gate* nl2_dummy_2 = nl_2->create_gate(gl->get_gate_type_by_name("BUF"), "dummy_2");
            ASSERT_NE(nl2_dummy_2, nullptr);
            Gate* nl2_dummy_3 = nl_2->create_gate(gl->get_gate_type_by_name("BUF"), "dummy_3");
            ASSERT_NE(nl2_dummy_3, nullptr);

            // standard stuff
            Module* nl1_m1 = nl_1->create_module(2, "module_1", nl_1->get_top_module());
            ASSERT_NE(nl1_m1, nullptr);
            Module* nl2_m1 = nl_2->create_module(2, "module_1", nl_2->get_top_module());
            ASSERT_NE(nl2_m1, nullptr);
            Module* nl1_m2 = nl_1->create_module(3, "module_1", nl_1->get_top_module());
            ASSERT_NE(nl1_m2, nullptr);
            Module* nl2_m2 = nl_2->create_module(3, "module_2", nl_2->get_top_module());
            ASSERT_NE(nl2_m2, nullptr);
            Module* nl1_m3 = nl_1->create_module(4, "module_3", nl_1->get_top_module());
            ASSERT_NE(nl1_m3, nullptr);
            nl1_m3->set_type("type_1");
            Module* nl2_m3 = nl_2->create_module(4, "module_3", nl_2->get_top_module());
            ASSERT_NE(nl2_m3, nullptr);
            nl2_m3->set_type("type_2");

            EXPECT_TRUE(*nl1_m1 == *nl1_m1);        // identical module pointer
            EXPECT_TRUE(*nl2_m1 == *nl2_m1);
            EXPECT_FALSE(*nl1_m1 != *nl1_m1);
            EXPECT_FALSE(*nl2_m1 != *nl2_m1);
            EXPECT_TRUE(*nl1_m1 == *nl2_m1);        // identical modules, but different netlists
            EXPECT_TRUE(*nl2_m1 == *nl1_m1);
            EXPECT_FALSE(*nl1_m1 != *nl2_m1);
            EXPECT_FALSE(*nl2_m1 != *nl1_m1);
            EXPECT_FALSE(*nl1_m1 == *nl1_m2);       // different IDs
            EXPECT_FALSE(*nl2_m1 == *nl2_m2);
            EXPECT_TRUE(*nl1_m1 != *nl1_m2);
            EXPECT_TRUE(*nl2_m1 != *nl2_m2);
            EXPECT_FALSE(*nl1_m2 == *nl2_m2);       // different names
            EXPECT_FALSE(*nl2_m2 == *nl1_m2);
            EXPECT_TRUE(*nl1_m2 != *nl2_m2);
            EXPECT_TRUE(*nl2_m2 != *nl1_m2);
            EXPECT_FALSE(*nl1_m3 == *nl2_m3);       // different types
            EXPECT_FALSE(*nl2_m3 == *nl1_m3);
            EXPECT_TRUE(*nl1_m3 != *nl2_m3);
            EXPECT_TRUE(*nl2_m3 != *nl1_m3);

            // different parent modules
            Module* nl1_m4 = nl_1->create_module(5, "module_4", nl1_m1);
            ASSERT_NE(nl1_m4, nullptr);
            Module* nl2_m4 = nl_2->create_module(5, "module_4", nl2_m2);
            ASSERT_NE(nl2_m4, nullptr);
            EXPECT_TRUE(*nl1_m4 == *nl2_m4);       // different parent modules
            EXPECT_TRUE(*nl2_m4 == *nl1_m4);
            EXPECT_FALSE(*nl1_m4 != *nl2_m4);
            EXPECT_FALSE(*nl2_m4 != *nl1_m4);

            // different submodules
            Module* nl1_m5 = nl_1->create_module(6, "module_5", nl_1->get_top_module());
            ASSERT_NE(nl1_m5, nullptr);
            Module* nl2_m5 = nl_2->create_module(6, "module_5", nl_2->get_top_module());
            ASSERT_NE(nl2_m5, nullptr);
            Module* nl1_m6 = nl_1->create_module(7, "module_6a", nl1_m5);
            ASSERT_NE(nl1_m6, nullptr);
            Module* nl2_m6 = nl_2->create_module(7, "module_6b", nl2_m5);
            ASSERT_NE(nl2_m6, nullptr);
            EXPECT_FALSE(*nl1_m5 == *nl2_m5);       // different submodules
            EXPECT_FALSE(*nl2_m5 == *nl1_m5);
            EXPECT_TRUE(*nl1_m5 != *nl2_m5);
            EXPECT_TRUE(*nl2_m5 != *nl1_m5);

            // different gates
            Module* nl1_m7 = nl_1->create_module(8, "module_7", nl_1->get_top_module(), {nl1_dummy_1, nl1_dummy_2});
            ASSERT_NE(nl1_m7, nullptr);
            Module* nl2_m7 = nl_2->create_module(8, "module_7", nl_2->get_top_module(), {nl2_dummy_1, nl2_dummy_3});
            ASSERT_NE(nl2_m7, nullptr);
            EXPECT_FALSE(*nl1_m7 == *nl2_m7);       // different gates
            EXPECT_FALSE(*nl2_m7 == *nl1_m7);
            EXPECT_TRUE(*nl1_m7 != *nl2_m7);
            EXPECT_TRUE(*nl2_m7 != *nl1_m7);

            // different input port names
            Module* nl1_m8 = nl_1->create_module(9, "module_8", nl_1->get_top_module(), {nl1_dummy_1, nl1_dummy_2});
            ASSERT_NE(nl1_m8, nullptr);
            Net* nl1_net_in = test_utils::connect(nl_1.get(), nl1_dummy_3, "O", nl1_dummy_1, "I", "net_in");
            ASSERT_NE(nl1_net_in, nullptr);
            Module* nl2_m8 = nl_2->create_module(9, "module_8", nl_2->get_top_module(), {nl2_dummy_1, nl2_dummy_2});
            ASSERT_NE(nl2_m8, nullptr);
            Net* nl2_net_in = test_utils::connect(nl_2.get(), nl2_dummy_3, "O", nl2_dummy_1, "I", "net_in");
            ASSERT_NE(nl2_net_in, nullptr);

            EXPECT_TRUE(*nl1_m8 == *nl2_m8);
            EXPECT_TRUE(*nl2_m8 == *nl1_m8);
            EXPECT_FALSE(*nl1_m8 != *nl2_m8);
            EXPECT_FALSE(*nl2_m8 != *nl1_m8);
            nl1_m8->set_input_port_name(nl1_net_in, "in_a");
            nl2_m8->set_input_port_name(nl2_net_in, "in_b");
            EXPECT_FALSE(*nl1_m8 == *nl2_m8);       // different input port names
            EXPECT_FALSE(*nl2_m8 == *nl1_m8);
            EXPECT_TRUE(*nl1_m8 != *nl2_m8);
            EXPECT_TRUE(*nl2_m8 != *nl1_m8);

            // different input port names
            Module* nl1_m9 = nl_1->create_module(10, "module_9", nl_1->get_top_module(), {nl1_dummy_1, nl1_dummy_2});
            ASSERT_NE(nl1_m9, nullptr);
            Net* nl1_net_out = test_utils::connect(nl_1.get(), nl1_dummy_1, "O", nl1_dummy_3, "I", "net_out");
            ASSERT_NE(nl1_net_out, nullptr);
            Module* nl2_m9 = nl_2->create_module(10, "module_9", nl_2->get_top_module(), {nl2_dummy_1, nl2_dummy_2});
            ASSERT_NE(nl2_m9, nullptr);
            Net* nl2_net_out = test_utils::connect(nl_2.get(), nl2_dummy_1, "O", nl2_dummy_3, "I", "net_out");
            ASSERT_NE(nl2_net_out, nullptr);

            EXPECT_TRUE(*nl1_m9 == *nl2_m9);
            EXPECT_TRUE(*nl2_m9 == *nl1_m9);
            EXPECT_FALSE(*nl1_m9 != *nl2_m9);
            EXPECT_FALSE(*nl2_m9 != *nl1_m9);
            nl1_m9->set_output_port_name(nl1_net_out, "out_a");
            nl2_m9->set_output_port_name(nl2_net_out, "out_b");
            EXPECT_FALSE(*nl1_m9 == *nl2_m9);       // different input port names
            EXPECT_FALSE(*nl2_m9 == *nl1_m9);
            EXPECT_TRUE(*nl1_m9 != *nl2_m9);
            EXPECT_TRUE(*nl2_m9 != *nl1_m9);
        }
        TEST_END
    }

    /**
     * Test changing a module's name.
     *
     * Functions: set_name
     */
    TEST_F(ModuleTest, check_set_id) {
        TEST_START
            {
                std::unique_ptr<Netlist> netlist = test_utils::create_empty_netlist();
                Module* dummy_module = netlist->create_module("dummy_module", netlist->get_top_module());
                ASSERT_NE(dummy_module, nullptr);

                dummy_module->set_name("new_name");
                EXPECT_EQ(dummy_module->get_name(), "new_name");

                dummy_module->set_name("");
                EXPECT_EQ(dummy_module->get_name(), "new_name");
            }
        TEST_END
    }

    /**
     * Test setting a module's type.
     *
     * Functions: set_type, get_type
     */
    TEST_F(ModuleTest, check_module_type) {
        TEST_START
            {
                std::unique_ptr<Netlist> netlist = test_utils::create_empty_netlist();
                Module* dummy_module = netlist->create_module("dummy_module", netlist->get_top_module());
                ASSERT_NE(dummy_module, nullptr);

                EXPECT_TRUE(dummy_module->get_type().empty());
                dummy_module->set_type("new_type");
                EXPECT_EQ(dummy_module->get_type(), "new_type");
            }
        TEST_END
    }

    /**
     * Test changing the parent module.
     *
     * Functions: set_parent_module, get_parent_module, get_submodules
     */
    TEST_F(ModuleTest, check_set_parent_module) {
        TEST_START
            // POSITIVE
            {
                /*  Consider the Module scheme below. We set the parent_module of m_0 from the top_module to m_1
                 *
                 *                  .--> d_3
                 *        .--> d_1 -|
                 *   top -|         '--> d_4     ==>      top-.                 .--> d_3
                 *        '--> d_2                            '-> d_2 --> d_1 --|
                 *                                                              '--> d_4
                 *
                 */
                std::unique_ptr<Netlist> netlist = test_utils::create_empty_netlist();
                Module* dummy_module_1 = netlist->create_module("dummy_module_1", netlist->get_top_module());
                ASSERT_NE(dummy_module_1, nullptr);
                Module* dummy_module_2 = netlist->create_module("dummy_module_2", netlist->get_top_module());
                ASSERT_NE(dummy_module_2, nullptr);
                Module* dummy_module_3 = netlist->create_module("dummy_module_3", dummy_module_1);
                ASSERT_NE(dummy_module_3, nullptr);
                Module* dummy_module_4 = netlist->create_module("dummy_module_4", dummy_module_1);
                ASSERT_NE(dummy_module_4, nullptr);

                dummy_module_1->set_parent_module(dummy_module_2);
                EXPECT_EQ(dummy_module_1->get_parent_module(), dummy_module_2);
                EXPECT_EQ(dummy_module_3->get_parent_module(), dummy_module_1);
                EXPECT_EQ(dummy_module_4->get_parent_module(), dummy_module_1);
                EXPECT_EQ(dummy_module_2->get_submodules([](const Module* module){ return module->get_name() == "dummy_module_1";}, false).size(), 1);
                EXPECT_TRUE(dummy_module_2->get_submodules([](const Module* module){ return module->get_name() == "dummy_module_3";}, false).empty());
                EXPECT_TRUE(dummy_module_2->get_submodules([](const Module* module){ return module->get_name() == "dummy_module_4";}, false).empty());
                EXPECT_EQ(dummy_module_2->get_submodules([](const Module* module){ return module->get_name() == "dummy_module_3";}, true).size(), 1);
                EXPECT_EQ(dummy_module_2->get_submodules([](const Module* module){ return module->get_name() == "dummy_module_4";}, true).size(), 1);
                EXPECT_EQ(dummy_module_1->get_submodules([](const Module* module){ return module->get_name() == "dummy_module_3";}, false).size(), 1);
                EXPECT_EQ(dummy_module_1->get_submodules([](const Module* module){ return module->get_name() == "dummy_module_4";}, false).size(), 1);
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
                std::unique_ptr<Netlist> netlist = test_utils::create_empty_netlist();
                Module* m_0 = netlist->create_module("test_module_0", netlist->get_top_module());
                Module* m_1 = netlist->create_module("test_module_1", m_0);
                Module* m_2 = netlist->create_module("test_module_2", m_0);

                m_0->set_parent_module(m_1);
                EXPECT_EQ(m_1->get_parent_module(), netlist->get_top_module());
                EXPECT_EQ(m_0->get_parent_module(), m_1);
                EXPECT_EQ(m_2->get_parent_module(), m_0);
            }
            // NEGATIVE
            {
                // make module parent module of itself
                NO_COUT_TEST_BLOCK;
                std::unique_ptr<Netlist> netlist = test_utils::create_empty_netlist();
                Module* m_0 = netlist->create_module("test_module_0", netlist->get_top_module());

                m_0->set_parent_module(m_0);
                EXPECT_EQ(m_0->get_parent_module(), netlist->get_top_module());
            }
            {
                // assign a parent module to the top_module
                NO_COUT_TEST_BLOCK;
                std::unique_ptr<Netlist> netlist = test_utils::create_empty_netlist();
                Module* m_0 = netlist->create_module("test_module_0", netlist->get_top_module());

                netlist->get_top_module()->set_parent_module(m_0);
                EXPECT_EQ(m_0->get_parent_module(), netlist->get_top_module());
                EXPECT_EQ(netlist->get_top_module()->get_parent_module(), nullptr);
            }
            {
                // assign nullptr as parent module
                NO_COUT_TEST_BLOCK;
                std::unique_ptr<Netlist> netlist = test_utils::create_empty_netlist();
                Module* m_0 = netlist->create_module("test_module_0", netlist->get_top_module());

                m_0->set_parent_module(nullptr);
                netlist->get_top_module()->set_parent_module(m_0);
                EXPECT_EQ(m_0->get_parent_module(), netlist->get_top_module());
            }
            {
                // assign module that is (no longer) part of the netlist
                NO_COUT_TEST_BLOCK;
                std::unique_ptr<Netlist> netlist = test_utils::create_empty_netlist();
                Module* m_0 = netlist->create_module("test_module_0", netlist->get_top_module());
                Module* m_1 = netlist->create_module("test_module_1", netlist->get_top_module());
                netlist->delete_module(m_0); // m_0 is removed from the netlist

                m_1->set_parent_module(m_0);
                EXPECT_EQ(m_1->get_parent_module(), netlist->get_top_module());
            }
        TEST_END
    }

    /**
     * Testing the contains_gate function
     *
     * Functions: contains_gate
     */
    TEST_F(ModuleTest, check_contains_gate) {
        TEST_START
            // POSITIVE
            {
                // Check a Gate, that is part of the Module (not recursive)
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module("test_module", nl->get_top_module());
                ASSERT_NE(m_0, nullptr);
                Gate* gate_0 = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_0");
                ASSERT_NE(gate_0, nullptr);
                m_0->assign_gate(gate_0);

                EXPECT_TRUE(m_0->contains_gate(gate_0, false));
                EXPECT_TRUE(m_0->contains_gate(gate_0, true));
            }
            {
                // Check a Gate, that isn't part of the Module (not recursive)
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module("test_module", nl->get_top_module());
                ASSERT_NE(m_0, nullptr);
                Gate* gate_0 = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_0");
                ASSERT_NE(gate_0, nullptr);

                EXPECT_FALSE(m_0->contains_gate(gate_0, false));
                EXPECT_FALSE(m_0->contains_gate(gate_0, true));
            }
            {
                // Check a Gate, that isn't part of the Module, but of a submodule (not recursive)
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module("test_module", nl->get_top_module());
                ASSERT_NE(m_0, nullptr);
                Module* submodule = nl->create_module("test_module", m_0);
                ASSERT_NE(submodule, nullptr);
                Gate* gate_0 = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_0");
                ASSERT_NE(gate_0, nullptr);
                submodule->assign_gate(gate_0);

                EXPECT_FALSE(m_0->contains_gate(gate_0, false));
                EXPECT_TRUE(m_0->contains_gate(gate_0, true));
            }
        TEST_END
    }

    /**
     * Testing the addition of gates to the Module. Verify the addition by call the
     * get_gates function and the contains_gate function
     *
     * Functions: assign_gate, contains_gate
     */
    TEST_F(ModuleTest, check_assign_gate) {
        TEST_START
            {
                // Add some gates to the Module
                auto nl = test_utils::create_empty_netlist();
                Gate* gate_0 = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_0");
                Gate* gate_1 = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_1");
                // this gate is not part of the Module
                Gate* gate_not_in_m = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_not_in_m");

                // Add gate_0 and gate_1 to a Module
                Module* test_module = nl->create_module("test Module", nl->get_top_module());
                test_module->assign_gate(gate_0);
                test_module->assign_gate(gate_1);

                std::vector<Gate*> expRes = {gate_0, gate_1};

                EXPECT_TRUE(test_utils::vectors_have_same_content(test_module->get_gates(), expRes));
                EXPECT_TRUE(test_module->contains_gate(gate_0));
                EXPECT_TRUE(test_module->contains_gate(gate_1));
                EXPECT_FALSE(test_module->contains_gate(gate_not_in_m));
            }
            {
                // Add the same Gate twice to the Module
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Gate* gate_0 = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_0");

                // Add gate_0 twice
                Module* test_module = nl->create_module("test Module", nl->get_top_module());
                test_module->assign_gate(gate_0);
                test_module->assign_gate(gate_0);

                std::vector<Gate*> expRes = {gate_0};

                EXPECT_EQ(test_module->get_gates(), expRes);
                EXPECT_TRUE(test_module->contains_gate(gate_0));
            }
            {
                // Insert a Gate owned by a submodule
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Gate* gate_0 = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_0");

                Module* test_module = nl->create_module("test Module", nl->get_top_module());
                Module* submodule = nl->create_module("submodule", test_module);
                submodule->assign_gate(gate_0);
                ASSERT_TRUE(submodule->contains_gate(gate_0));
                ASSERT_FALSE(test_module->contains_gate(gate_0));

                test_module->assign_gate(gate_0);

                std::vector<Gate*> expRes = {gate_0};

                EXPECT_EQ(test_module->get_gates(), expRes);
                EXPECT_FALSE(submodule->contains_gate(gate_0));
            }

            // NEGATIVE
            {
                // Assigned Gate is a nullptr
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Module* test_module = nl->create_module("test Module", nl->get_top_module());
                test_module->assign_gate(nullptr);
                EXPECT_TRUE(test_module->get_gates().empty());
            }
            {
                // Call contains_gate with a nullptr
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Module* test_module = nl->create_module("test Module", nl->get_top_module());
                EXPECT_FALSE(test_module->contains_gate(nullptr));
            }
        TEST_END
    }

    /**
     * Testing the deletion of gates from modules
     *
     * Functions: remove_gate
     */
    TEST_F(ModuleTest, check_remove_gate) {
        TEST_START
            {
                // Delete a Gate from a Module (Gate owned by the modules)
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module("test_module", nl->get_top_module());
                Gate* gate_0 = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_0");
                m_0->assign_gate(gate_0);

                ASSERT_TRUE(m_0->contains_gate(gate_0));
                m_0->remove_gate(gate_0);
                EXPECT_FALSE(m_0->contains_gate(gate_0));
            }
            {
                // Try to delete a Gate from a Module (Gate owned by another Module)
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module("test_module", nl->get_top_module());
                Module* m_other = nl->create_module("other_test_module", nl->get_top_module());
                Gate* gate_0 = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_0");
                m_other->assign_gate(gate_0);

                m_0->remove_gate(gate_0);
                EXPECT_FALSE(m_0->contains_gate(gate_0));
                EXPECT_TRUE(m_other->contains_gate(gate_0));
            }
            // NEGATIVE
            {
                // Try to delete a Gate from the top-Module (should change nothing)
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Gate* gate_0 = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_0");
                Module* tm = nl->get_top_module();

                ASSERT_TRUE(tm->contains_gate(gate_0));
                tm->remove_gate(gate_0);
                EXPECT_TRUE(tm->contains_gate(gate_0));
            }
            {
                // Try to delete a nullptr (should not crash)
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module(MIN_MODULE_ID + 0, "test_module", nl->get_top_module());

                m_0->remove_gate(nullptr);
            }
        TEST_END
    }

    /**
     * Testing the get_gate_by_id function
     *
     * Functions: get_gate_by_id
     */
    TEST_F(ModuleTest, check_get_gate_by_id) {
        TEST_START
            // POSITIVE
            {
                // get a Gate by its id (Gate owned by Module)(not recursive)
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module("test_module", nl->get_top_module());
                Gate* gate_123 = nl->create_gate(123, nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_123");
                m_0->assign_gate(gate_123);

                ASSERT_TRUE(m_0->contains_gate(gate_123));
                EXPECT_EQ(m_0->get_gate_by_id(123, false), gate_123);
                EXPECT_EQ(m_0->get_gate_by_id(123, true), gate_123);
            }
            {
                // get a Gate by its id (not owned by a submodule)(not recursive)
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module("test_module", nl->get_top_module());
                Module* submodule = nl->create_module("other_module", m_0);
                Gate* gate_123 = nl->create_gate(123, nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_123");
                submodule->assign_gate(gate_123);

                EXPECT_EQ(m_0->get_gate_by_id(123, false), nullptr);
                EXPECT_EQ(m_0->get_gate_by_id(123, true), gate_123);
            }
        TEST_END
    }

    /**
     * Testing the access on submodules as well as the contains Module function. Therefore we build up a Module tree like this:
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
    TEST_F(ModuleTest, check_get_submodules) {
        TEST_START
            // Set up the Module tree
            auto nl = test_utils::create_empty_netlist();
            Module* tm = nl->get_top_module();
            ASSERT_NE(tm, nullptr);
            Module* m_0 = nl->create_module("even_module", tm);
            ASSERT_NE(m_0, nullptr);
            Module* m_1 = nl->create_module("odd_module", tm);
            ASSERT_NE(m_1, nullptr);
            Module* m_2 = nl->create_module("even_module", m_1);
            ASSERT_NE(m_2, nullptr);
            Module* m_3 = nl->create_module("odd_module", m_1);
            ASSERT_NE(m_3, nullptr);
            {
                // Testing the access on submodules (no module_name_filter, not recursive)
                {
                    // Submodules of TOP_MODULE;
                    std::vector<Module*> exp_result = {m_0, m_1};
                    EXPECT_EQ(tm->get_submodules(nullptr, false), exp_result);
                    EXPECT_TRUE(tm->contains_module(m_0, false));
                    EXPECT_TRUE(tm->contains_module(m_1, false));
                    EXPECT_FALSE(tm->contains_module(m_2, false));
                    EXPECT_FALSE(tm->contains_module(m_3, false));
                }
                {
                    // Submodules of MODULE_1;
                    std::vector<Module*> exp_result = {m_2, m_3};
                    EXPECT_EQ(m_1->get_submodules(nullptr, false), exp_result);
                }
                {
                    // Submodules of MODULE_0;
                    std::vector<Module*> exp_result = {};
                    EXPECT_EQ(m_0->get_submodules(nullptr, false), exp_result);
                }
            }
            {
                // Testing the access on submodules (module_name_filter set, not recursive)
                {
                    // Submodules of TOP_MODULE;
                    std::vector<Module*> exp_result = {m_0};
                    EXPECT_EQ(tm->get_submodules([](const Module* module){ return module->get_name() == "even_module"; }, false), exp_result);
                }
                {
                    // Submodules of MODULE_1;
                    std::vector<Module*> exp_result = {m_2};
                    EXPECT_EQ(m_1->get_submodules([](const Module* module){ return module->get_name() == "even_module"; }, false), exp_result);
                }
                {
                    // Submodules of TOP_MODULE (name does not exists);
                    std::vector<Module*> exp_result = {};
                    EXPECT_EQ(tm->get_submodules([](const Module* module){ return module->get_name() == "INVALID"; }, false), exp_result);
                }
            }
            {
                // Testing the access on submodules (recursive)
                {
                    // Submodules of TOP_MODULE;
                    std::vector<Module*> exp_result = {m_0, m_1, m_2, m_3};
                    EXPECT_EQ(tm->get_submodules(nullptr, true), exp_result);
                    EXPECT_TRUE(tm->contains_module(m_0, true));
                    EXPECT_TRUE(tm->contains_module(m_1, true));
                    EXPECT_TRUE(tm->contains_module(m_2, true));
                    EXPECT_TRUE(tm->contains_module(m_3, true));
                }
                {
                    // Submodules of TOP_MODULE (with module_name_filter);
                    std::vector<Module*> exp_result = {m_0, m_2};
                    EXPECT_TRUE(test_utils::vectors_have_same_content(tm->get_submodules([](const Module* module){ return module->get_name() == "even_module"; }, true), exp_result));
                }
                {
                    // Submodules of MODULE_0
                    std::vector<Module*> exp_result = {};
                    EXPECT_EQ(m_0->get_submodules(nullptr, true), exp_result);
                }
            }
            {
                // Testing edge cases of contains_module

                // -- the passed Module is a nullptr
                EXPECT_FALSE(tm->contains_module(nullptr, false));
                // -- the calling Module is a leave
                EXPECT_FALSE(m_2->contains_module(tm, false));
                // -- the passed Module is the same as the calling one
                EXPECT_FALSE(m_2->contains_module(m_2, false));
            }
        TEST_END
    }

    /*
     *      Testing the get_input_nets, get_output_nets, get_internal_nets by using the following example netlist with a Module
     *
     *                     ################################################
     *                     # TEST_MODULE                                  #
     *                     #                                              #
     *      global_in -----§---------------=  BUF (0)  = -----------------§----- global_out
     *                     #                                              #
     *                 .---§--= BUF (1) =--=                              #
     *                 |   #                  AND2 (2) =--+---------------§----= BUF (5)
     *    = BUF (4) =--+---§---------------=              |               #
     *                     #                              '--= BUF (3) =  #
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
    TEST_F(ModuleTest, check_get_input_nets) {
        TEST_START
            // +++ Create the example netlist (see above)

            auto nl = test_utils::create_empty_netlist();

            // Add the gates
            Gate* gate_0 = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_0");
            Gate* gate_1 = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_1");
            Gate* gate_2 = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("AND2"), "gate_2");
            Gate* gate_3 = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_3");
            Gate* gate_4 = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_4");
            Gate* gate_5 = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_5");

            Net* net_g_0 = nl->create_net("name_0");
            nl->mark_global_input_net(net_g_0);
            net_g_0->add_destination(gate_0, "I");

            Net* net_0_g = nl->create_net("name_1");
            net_0_g->add_source(gate_0, "O");
            nl->mark_global_output_net(net_0_g);

            Net* net_4_1_2 = test_utils::connect(nl.get(), gate_4, "O", gate_1, "I");
            test_utils::connect(nl.get(), gate_4, "O", gate_2, "I1");

            Net* net_1_2 = test_utils::connect(nl.get(), gate_1, "O", gate_2, "I0");

            Net* net_2_3_5 = test_utils::connect(nl.get(), gate_2, "O", gate_3, "I");
            test_utils::connect(nl.get(), gate_2, "O", gate_5, "I");

            // Create the Module
            auto test_module = nl->create_module("test_module", nl->get_top_module());
            for (auto g : std::set<Gate*>({gate_0, gate_1, gate_2, gate_3})) {
                test_module->assign_gate(g);
            }
            {
                // Get input nets of the test Module
                std::vector<Net*> exp_result = {net_g_0, net_4_1_2};
                std::sort(exp_result.begin(), exp_result.end());
                EXPECT_EQ(test_module->get_input_nets(), exp_result);
            }
            {
                // Get output nets of the test Module
                std::vector<Net*> exp_result = {net_0_g, net_2_3_5};
                std::sort(exp_result.begin(), exp_result.end());
                EXPECT_EQ(test_module->get_output_nets(), exp_result);
            }
            {
                // Get internal nets of the test Module
                std::vector<Net*> exp_result = {net_1_2, net_2_3_5};
                std::sort(exp_result.begin(), exp_result.end());
                EXPECT_EQ(test_module->get_internal_nets(), exp_result);
            }
        TEST_END
    }

    /**
     * Testing the usage of port names
     *
     * Functions: get_input_port_name, set_input_port_name, get_output_port_name, set_output_port_name,
     *            get_input_port_names, get_output_port_names, get_input_port_net, get_output_port_net
     */
    TEST_F(ModuleTest, check_port_names) {
        TEST_START
            // Add some modules to the example netlist
            auto nl = test_utils::create_example_netlist();
            Module* m_0 = nl->create_module("mod_0", nl->get_top_module(), {nl->get_gate_by_id(MIN_GATE_ID + 0), nl->get_gate_by_id(MIN_GATE_ID + 3)});
            {
                // Get the input port name of a Net, which port name was not specified yet
                EXPECT_EQ(m_0->get_input_port_name(nl->get_net_by_id(MIN_NET_ID + 13)), "I(0)");
            }
            {
                // Get the output port name of a Net, which port name was not specified yet
                EXPECT_EQ(m_0->get_output_port_name(nl->get_net_by_id(MIN_NET_ID + 045)), "O(0)");
            }
            {
                // Set and get an input port name
                m_0->set_input_port_name(nl->get_net_by_id(MIN_NET_ID + 13), "port_name_net_1_3");
                EXPECT_EQ(m_0->get_input_port_name(nl->get_net_by_id(MIN_NET_ID + 13)), "port_name_net_1_3");
                EXPECT_EQ(m_0->get_input_port_net("port_name_net_1_3"), nl->get_net_by_id(MIN_NET_ID + 13));
            }
            {
                // Set and get an output port name
                m_0->set_output_port_name(nl->get_net_by_id(MIN_NET_ID + 045), "port_name_net_0_4_5");
                EXPECT_EQ(m_0->get_output_port_name(nl->get_net_by_id(MIN_NET_ID + 045)), "port_name_net_0_4_5");
                EXPECT_EQ(m_0->get_output_port_net("port_name_net_0_4_5"), nl->get_net_by_id(MIN_NET_ID + 045));
            }
            // Create a new Module with more modules (with 2 input and ouput nets)
            Module* m_1 = nl->create_module("mod_1", nl->get_top_module(), {nl->get_gate_by_id(MIN_GATE_ID + 0), nl->get_gate_by_id(MIN_GATE_ID + 3), nl->get_gate_by_id(MIN_GATE_ID + 7)});
            // Specify exactly one input and output port name
            m_1->set_input_port_name(nl->get_net_by_id(MIN_NET_ID + 13), "port_name_net_1_3");
            m_1->set_output_port_name(nl->get_net_by_id(MIN_NET_ID + 045), "port_name_net_0_4_5");

            {
                // Get all input port names
                std::map<Net*, std::string> exp_input_port_names = {
                    {nl->get_net_by_id(MIN_NET_ID + 13), "port_name_net_1_3"},
                    {nl->get_net_by_id(MIN_NET_ID + 20), "I(0)"}
                };
                EXPECT_EQ(m_1->get_input_port_names(), exp_input_port_names);
            }
            {
                // Get all output port names
                std::map<Net*, std::string> exp_output_port_names = {
                    {nl->get_net_by_id(MIN_NET_ID + 045), "port_name_net_0_4_5"},
                    {nl->get_net_by_id(MIN_NET_ID + 78), "O(0)"}
                };
                EXPECT_EQ(m_1->get_output_port_names(), exp_output_port_names);
            }
            // Create a new Module with more modules (with 2 input and ouput nets)
            Module* m_2 = nl->create_module("mod_2", nl->get_top_module(), {nl->get_gate_by_id(MIN_GATE_ID + 3)});
            // Add an input and an output port name
            m_2->set_input_port_name(nl->get_net_by_id(MIN_NET_ID + 13), "port_name_net_1_3");
            m_2->set_output_port_name(nl->get_net_by_id(MIN_NET_ID + 30), "port_name_net_3_0");
            // Add additional gates to the Module so that the port name nets are no longer input/output nets of the Module
            m_2->assign_gate(nl->get_gate_by_id(MIN_GATE_ID + 1));
            m_2->assign_gate(nl->get_gate_by_id(MIN_GATE_ID + 0));
            {
                // Get all input port names. The old ports shouldn't be contained.
                std::map<Net*, std::string> exp_input_port_names = {
                    {nl->get_net_by_id(MIN_NET_ID + 20), "I(0)"}
                };
                EXPECT_EQ(m_2->get_input_port_names(), exp_input_port_names);
            }
            {
                // Get all output port names. The old ports shouldn't be contained.
                std::map<Net*, std::string> exp_output_port_names = {
                    {nl->get_net_by_id(MIN_NET_ID + 045), "O(0)"}
                };
                EXPECT_EQ(m_2->get_output_port_names(), exp_output_port_names);
            }

            // NEGATIVE
            {
                // Set the input port name of a Net that is no input Net of the Module
                NO_COUT_TEST_BLOCK;
                m_0->set_input_port_name(nl->get_net_by_id(MIN_NET_ID + 78), "port_name");
                EXPECT_EQ(m_0->get_input_port_name(nl->get_net_by_id(MIN_NET_ID + 78)), "");
                EXPECT_EQ(m_0->get_input_port_net("port_name"), nullptr);
            }
            {
                // Set the output port name of a Net, that is no input Net of the Module
                NO_COUT_TEST_BLOCK;
                m_0->set_output_port_name(nl->get_net_by_id(MIN_NET_ID + 78), "port_name");
                EXPECT_EQ(m_0->get_output_port_name(nl->get_net_by_id(MIN_NET_ID + 78)), "");
                EXPECT_EQ(m_0->get_output_port_net("port_name"), nullptr);
            }
            {
                // Pass a nullptr
                NO_COUT_TEST_BLOCK;
                m_0->set_input_port_name(nullptr, "port_name");
                m_0->set_output_port_name(nullptr, "port_name");
                EXPECT_EQ(m_0->get_input_port_name(nullptr), "");
                EXPECT_EQ(m_0->get_output_port_name(nullptr), "");
            }
            {
                // Pass an empty string to get_input_port_net/get_output_port_net
                NO_COUT_TEST_BLOCK;
                EXPECT_EQ(m_0->get_input_port_net(""), nullptr);
                EXPECT_EQ(m_0->get_output_port_net(""), nullptr);
            }
        TEST_END
    }

    /**
     * Testing the get_grouping function
     *
     * Functions: get_grouping
     */
    TEST_F(ModuleTest, check_get_grouping) {
        TEST_START
            {
                // get the grouping of a module (nullptr), then add it to another grouping and check again
                auto nl = test_utils::create_empty_netlist();
                Module* test_module = nl->create_module("test_module", nl->get_top_module());

                EXPECT_EQ(test_module->get_grouping(), nullptr);

                // -- move the module in the test_grouping
                Grouping* test_grouping = nl->create_grouping("test_grouping");
                test_grouping->assign_module(test_module);

                EXPECT_EQ(test_module->get_grouping(), test_grouping);

                // -- delete the test_grouping, so the module should be nullptr again
                nl->delete_grouping(test_grouping);
                EXPECT_EQ(test_module->get_grouping(), nullptr);
            }
        TEST_END
    }  
      
    /*************************************
     * Event System
     *************************************/

    /**
     * Testing the triggering of events.
     */
    TEST_F(ModuleTest, check_events) {
        TEST_START
            const u32 NO_DATA = 0xFFFFFFFF;

            std::unique_ptr<Netlist> test_nl = test_utils::create_example_netlist();
            Module* top_mod = test_nl->get_top_module();
            Module* test_mod = test_nl->create_module("test_mod", test_nl->get_top_module());
            Module* other_mod = test_nl->create_module("other_mod", test_nl->get_top_module());
            Module* other_mod_sub = test_nl->create_module("other_mod_sub", test_nl->get_top_module());
            Gate* test_gate = test_nl->get_gate_by_id(MIN_GATE_ID + 0);

            // Small functions that should trigger certain events exactly once (these operations are executed in this order)
            std::function<void(void)> trigger_name_changed = [=](){test_mod->set_name("new_name");};
            std::function<void(void)> trigger_type_changed = [=](){test_mod->set_type("new_type");};
            std::function<void(void)> trigger_parent_changed = [=](){test_mod->set_parent_module(other_mod);};
            std::function<void(void)> trigger_submodule_added = [=](){other_mod_sub->set_parent_module(test_mod);};
            std::function<void(void)> trigger_submodule_removed = [=](){other_mod_sub->set_parent_module(top_mod);};
            std::function<void(void)> trigger_gate_assigned = [=](){test_mod->assign_gate(test_gate);};
            std::function<void(void)> trigger_gate_removed = [=](){test_mod->remove_gate(test_gate);};
            std::function<void(void)> trigger_input_port_name_changed = [=](){
                test_mod->assign_gate(test_gate);
                test_mod->set_input_port_name(test_gate->get_fan_in_net("I0"), "mod_in_0");
            };
            std::function<void(void)> trigger_output_port_name_changed = [=](){
                test_mod->assign_gate(test_gate);
                test_mod->set_output_port_name(test_gate->get_fan_out_net("O"), "mod_out");
            };

            // The events that are tested
            std::vector<ModuleEvent::event> event_type = {
                ModuleEvent::event::name_changed, ModuleEvent::event::type_changed,
                ModuleEvent::event::parent_changed, ModuleEvent::event::submodule_added,
                ModuleEvent::event::submodule_removed, ModuleEvent::event::gate_assigned,
                ModuleEvent::event::gate_removed, ModuleEvent::event::input_port_name_changed,
                ModuleEvent::event::output_port_name_changed};

            // A list of the functions that will trigger its associated event exactly once
            std::vector<std::function<void(void)>> trigger_event = { trigger_name_changed, trigger_type_changed,
                 trigger_parent_changed, trigger_submodule_added, trigger_submodule_removed, trigger_gate_assigned,
                 trigger_gate_removed, trigger_input_port_name_changed, trigger_output_port_name_changed};

            // The parameters of the events that are expected
            std::vector<std::tuple<ModuleEvent::event, Module*, u32>> expected_parameter = {
                std::make_tuple(ModuleEvent::event::name_changed, test_mod, NO_DATA),
                std::make_tuple(ModuleEvent::event::type_changed, test_mod, NO_DATA),
                std::make_tuple(ModuleEvent::event::parent_changed, test_mod, NO_DATA),
                std::make_tuple(ModuleEvent::event::submodule_added, test_mod, other_mod_sub->get_id()),
                std::make_tuple(ModuleEvent::event::submodule_removed, test_mod, other_mod_sub->get_id()),
                std::make_tuple(ModuleEvent::event::gate_assigned, test_mod, test_gate->get_id()),
                std::make_tuple(ModuleEvent::event::gate_removed, test_mod, test_gate->get_id()),
                std::make_tuple(ModuleEvent::event::input_port_name_changed, test_mod, test_gate->get_fan_in_net("I0")->get_id()),
                std::make_tuple(ModuleEvent::event::output_port_name_changed, test_mod, test_gate->get_fan_out_net("O")->get_id())
            };

            // Check all events in a for-loop
            for(u32 event_idx = 0; event_idx < event_type.size(); event_idx++)
            {
                // Create the listener for the tested event
                test_utils::EventListener<void, ModuleEvent::event, Module*, u32> listener;
                std::function<void(ModuleEvent::event, Module*, u32)> cb = listener.get_conditional_callback(
                    [=](ModuleEvent::event ev, Module* m, u32 id){return ev == event_type[event_idx] && m == test_mod;}
                );
                std::string cb_name = "mod_event_callback_" + std::to_string((u32)event_type[event_idx]);
                // Register a callback of the listener
                test_nl->get_event_handler()->register_callback(cb_name, cb);

                // Trigger the event
                trigger_event[event_idx]();

                EXPECT_EQ(listener.get_event_count(), 1);
                EXPECT_EQ(listener.get_last_parameters(), expected_parameter[event_idx]);

                // Unregister the callback
                test_nl->get_event_handler()->unregister_callback(cb_name);
            }

            // Test the events 'created' and 'removed'
            // -- 'created' event
            test_utils::EventListener<void, ModuleEvent::event, Module*, u32> listener_created;
            std::function<void(ModuleEvent::event, Module*, u32)> cb_created = listener_created.get_conditional_callback(
                [=](ModuleEvent::event ev, Module* m, u32 id){return ev == ModuleEvent::event::created;}
            );
            std::string cb_name_created = "mod_event_callback_created";
            test_nl->get_event_handler()->register_callback(cb_name_created, cb_created);

            // Create a new mod
            Module* new_mod = test_nl->create_module("new_mod", test_nl->get_top_module());
            EXPECT_EQ(listener_created.get_event_count(), 1);
            EXPECT_EQ(listener_created.get_last_parameters(), std::make_tuple(ModuleEvent::event::created, new_mod, NO_DATA));

            test_nl->get_event_handler()->unregister_callback(cb_name_created);

            // -- 'removed' event
            test_utils::EventListener<void, ModuleEvent::event, Module*, u32> listener_removed;
            std::function<void(ModuleEvent::event, Module*, u32)> cb_removed = listener_removed.get_conditional_callback(
                [=](ModuleEvent::event ev, Module* m, u32 id){return ev == ModuleEvent::event::removed;}
            );
            std::string cb_name_removed = "mod_event_callback_removed";
            test_nl->get_event_handler()->register_callback(cb_name_removed, cb_removed);

            // Delete the module which was created in the previous part
            test_nl->delete_module(new_mod);
            EXPECT_EQ(listener_removed.get_event_count(), 1);
            EXPECT_EQ(listener_removed.get_last_parameters(), std::make_tuple(ModuleEvent::event::removed, new_mod, NO_DATA));

            test_nl->get_event_handler()->unregister_callback(cb_name_removed);

        TEST_END
    }

} //namespace hal
