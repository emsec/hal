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
                EXPECT_EQ(top_module->get_parent_modules(), std::vector<Module*>());

                Module* dummy_module = netlist->create_module(12, "dummy_module", top_module);
                ASSERT_NE(dummy_module, nullptr);
                EXPECT_FALSE(dummy_module->is_top_module());
                EXPECT_EQ(dummy_module->get_id(), 12);
                EXPECT_EQ(dummy_module->get_name(), "dummy_module");
                EXPECT_EQ(dummy_module->get_netlist(), netlist.get());
                EXPECT_TRUE(dummy_module->get_type().empty());
                EXPECT_EQ(dummy_module->get_parent_module(), top_module);
                EXPECT_EQ(dummy_module->get_parent_modules(), std::vector<Module*>({top_module}));
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
            ModulePin* pin_1 = nl1_m8->get_pin_by_net(nl1_net_in);
            ASSERT_NE(pin_1, nullptr);
            ASSERT_TRUE(nl1_m8->set_pin_name(pin_1, "in_a"));
            ModulePin* pin_2 = nl2_m8->get_pin_by_net(nl2_net_in);
            ASSERT_NE(pin_2, nullptr);
            ASSERT_TRUE(nl2_m8->set_pin_name(pin_2, "in_b"));
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
            ModulePin* pin_3 = nl1_m9->get_pin_by_net(nl1_net_out);
            ASSERT_NE(pin_3, nullptr);
            ASSERT_TRUE(nl1_m9->set_pin_name(pin_3, "out_a"));
            ModulePin* pin_4 = nl2_m9->get_pin_by_net(nl2_net_out);
            ASSERT_NE(pin_4, nullptr);
            ASSERT_TRUE(nl2_m9->set_pin_name(pin_4, "out_b"));
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
     * Functions: set_parent_module, get_parent_module, get_parent_modules, get_submodules, is_parent_module_of, is_submodule_of
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
                Module* top_module = netlist->get_top_module();

                EXPECT_EQ(top_module->get_parent_modules(), std::vector<Module*>());
                EXPECT_EQ(dummy_module_1->get_parent_modules(), std::vector<Module*>({top_module}));
                EXPECT_EQ(dummy_module_2->get_parent_modules(), std::vector<Module*>({top_module}));
                EXPECT_EQ(dummy_module_3->get_parent_modules(), std::vector<Module*>({dummy_module_1, top_module}));
                EXPECT_EQ(dummy_module_3->get_parent_modules(nullptr, false), std::vector<Module*>({dummy_module_1}));
                EXPECT_EQ(dummy_module_4->get_parent_modules(), std::vector<Module*>({dummy_module_1, top_module}));
                EXPECT_EQ(dummy_module_4->get_parent_modules(nullptr, false), std::vector<Module*>({dummy_module_1}));

                EXPECT_TRUE(top_module->is_parent_module_of(dummy_module_1));
                EXPECT_TRUE(top_module->is_parent_module_of(dummy_module_2));
                EXPECT_FALSE(top_module->is_parent_module_of(dummy_module_3));
                EXPECT_TRUE(top_module->is_parent_module_of(dummy_module_3, true));
                EXPECT_FALSE(top_module->is_parent_module_of(dummy_module_4));
                EXPECT_TRUE(top_module->is_parent_module_of(dummy_module_4, true));

                EXPECT_TRUE(dummy_module_1->is_submodule_of(top_module));
                EXPECT_TRUE(dummy_module_2->is_submodule_of(top_module));
                EXPECT_FALSE(dummy_module_3->is_submodule_of(top_module));
                EXPECT_TRUE(dummy_module_3->is_submodule_of(top_module, true));
                EXPECT_FALSE(dummy_module_4->is_submodule_of(top_module));
                EXPECT_TRUE(dummy_module_4->is_submodule_of(top_module, true));

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

                EXPECT_EQ(top_module->get_parent_modules(), std::vector<Module*>());
                EXPECT_EQ(dummy_module_1->get_parent_modules(), std::vector<Module*>({dummy_module_2, top_module}));
                EXPECT_EQ(dummy_module_1->get_parent_modules(nullptr, false), std::vector<Module*>({dummy_module_2}));
                EXPECT_EQ(dummy_module_2->get_parent_modules(), std::vector<Module*>({top_module}));
                EXPECT_EQ(dummy_module_3->get_parent_modules(), std::vector<Module*>({dummy_module_1, dummy_module_2, top_module}));
                EXPECT_EQ(dummy_module_3->get_parent_modules(nullptr, false), std::vector<Module*>({dummy_module_1}));
                EXPECT_EQ(dummy_module_4->get_parent_modules(), std::vector<Module*>({dummy_module_1, dummy_module_2, top_module}));
                EXPECT_EQ(dummy_module_4->get_parent_modules(nullptr, false), std::vector<Module*>({dummy_module_1}));

                EXPECT_FALSE(top_module->is_parent_module_of(dummy_module_1));
                EXPECT_TRUE(top_module->is_parent_module_of(dummy_module_1, true));
                EXPECT_TRUE(top_module->is_parent_module_of(dummy_module_2));
                EXPECT_FALSE(top_module->is_parent_module_of(dummy_module_3));
                EXPECT_TRUE(top_module->is_parent_module_of(dummy_module_3, true));
                EXPECT_FALSE(top_module->is_parent_module_of(dummy_module_4));
                EXPECT_TRUE(top_module->is_parent_module_of(dummy_module_4, true));

                EXPECT_FALSE(dummy_module_1->is_submodule_of(top_module));
                EXPECT_TRUE(dummy_module_1->is_submodule_of(top_module, true));
                EXPECT_TRUE(dummy_module_2->is_submodule_of(top_module));
                EXPECT_FALSE(dummy_module_3->is_submodule_of(top_module));
                EXPECT_TRUE(dummy_module_3->is_submodule_of(top_module, true));
                EXPECT_FALSE(dummy_module_4->is_submodule_of(top_module));
                EXPECT_TRUE(dummy_module_4->is_submodule_of(top_module, true));
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
     * Functions: assign_gate, assign_gates, contains_gate
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
                // Add some gates to the Module
                auto nl = test_utils::create_empty_netlist();
                Gate* gate_0 = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_0");
                Gate* gate_1 = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_1");
                // this gate is not part of the Module
                Gate* gate_not_in_m = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_not_in_m");

                // Add gate_0 and gate_1 to a Module
                Module* test_module = nl->create_module("test Module", nl->get_top_module());
                test_module->assign_gates({gate_0, gate_1});

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
     * Functions: gte_nets, get_input_nets, get_output_nets, get_internal_nets, contains_net, is_input_net, is_output_net, is_internal_net
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
                std::unordered_set<Net*> exp_result = {net_g_0, net_4_1_2};
                EXPECT_EQ(test_module->get_input_nets(), exp_result);
                EXPECT_TRUE(test_module->is_input_net(net_g_0));
                EXPECT_TRUE(test_module->is_input_net(net_4_1_2));
                EXPECT_FALSE(test_module->is_input_net(net_0_g));
                EXPECT_FALSE(test_module->is_input_net(net_2_3_5));
                EXPECT_FALSE(test_module->is_input_net(net_1_2));
            }
            {
                // Get output nets of the test Module
                std::unordered_set<Net*> exp_result = {net_0_g, net_2_3_5};
                EXPECT_EQ(test_module->get_output_nets(), exp_result);
                EXPECT_TRUE(test_module->is_output_net(net_0_g));
                EXPECT_TRUE(test_module->is_output_net(net_2_3_5));
                EXPECT_FALSE(test_module->is_output_net(net_g_0));
                EXPECT_FALSE(test_module->is_output_net(net_4_1_2));
                EXPECT_FALSE(test_module->is_output_net(net_1_2));
            }
            {
                // Get internal nets of the test Module
                std::unordered_set<Net*> exp_result = {net_1_2, net_2_3_5};
                EXPECT_EQ(test_module->get_internal_nets(), exp_result);
                EXPECT_TRUE(test_module->is_internal_net(net_1_2));
                EXPECT_TRUE(test_module->is_internal_net(net_2_3_5));
                EXPECT_FALSE(test_module->is_internal_net(net_g_0));
                EXPECT_FALSE(test_module->is_internal_net(net_4_1_2));
                EXPECT_FALSE(test_module->is_internal_net(net_0_g));
            }
            {
                // Get nets of the test Module
                std::unordered_set<Net*> exp_result = {net_g_0, net_0_g, net_4_1_2, net_1_2, net_2_3_5};
                EXPECT_EQ(test_module->get_nets(), exp_result);
                EXPECT_TRUE(test_module->contains_net(net_1_2));
                EXPECT_TRUE(test_module->contains_net(net_2_3_5));
                EXPECT_TRUE(test_module->contains_net(net_g_0));
                EXPECT_TRUE(test_module->contains_net(net_4_1_2));
                EXPECT_TRUE(test_module->contains_net(net_0_g));
            }
        TEST_END
    }

    /**
     * Testing the usage of module pins
     *
     * Functions: get_pins, get_pin_groups, get_pin, get_pin_group, set_pin_name, set_pin_group_name, set_pin_type, set_pin_group_type, create_pin_group, delete_pin_group, assign_pin_to_group
     */
    TEST_F(ModuleTest, check_pins) {
        TEST_START
        {
            // add module to netlist
            auto nl = test_utils::create_example_netlist();
            ASSERT_NE(nl, nullptr);
            Module* m_0 = nl->create_module("mod_0", nl->get_top_module(), {nl->get_gate_by_id(MIN_GATE_ID + 0), nl->get_gate_by_id(MIN_GATE_ID + 3)});
            ASSERT_NE(m_0, nullptr);
            {
                // get input pin and check name, net, direction, and type
                Net* net = nl->get_net_by_id(MIN_NET_ID + 13);
                ASSERT_NE(net, nullptr);
                ModulePin* pin_by_net = m_0->get_pin_by_net(net);
                ASSERT_NE(pin_by_net, nullptr);
                ModulePin* pin_by_id = m_0->get_pin_by_id(pin_by_net->get_id());
                ASSERT_NE(pin_by_id, nullptr);
                EXPECT_EQ(pin_by_net->get_direction(), PinDirection::input);
                EXPECT_EQ(pin_by_net->get_type(), PinType::none);
                EXPECT_EQ(pin_by_net->get_net(), net);
                EXPECT_TRUE(pin_by_net == pin_by_id);

                // get input pin groups
                PinGroup<ModulePin>* group = m_0->get_pin_group_by_id(pin_by_net->get_group().first->get_id());
                ASSERT_NE(group, nullptr);
                EXPECT_FALSE(group->empty());
                EXPECT_EQ(group->size(), 1);
                EXPECT_EQ(group->get_name(), pin_by_net->get_name());
                EXPECT_EQ(group->get_pins().front(), pin_by_net);
                EXPECT_EQ(group->get_direction(), PinDirection::input);
                EXPECT_EQ(group->get_type(), PinType::none);
                EXPECT_EQ(group->get_index(pin_by_net).get(), 0);
                EXPECT_EQ(group->get_pin_at_index(0).get(), pin_by_net);
                EXPECT_EQ(group->get_start_index(), 0);
                EXPECT_EQ(group->is_ascending(), true);
                EXPECT_EQ(pin_by_net->get_group(), std::pair(group, i32(0)));

                // set pin name
                std::string old_name = pin_by_net->get_name();
                EXPECT_TRUE(m_0->set_pin_name(pin_by_net, "toller_pin"));
                EXPECT_EQ(pin_by_net->get_name(), "toller_pin");

                // set group name
                old_name = group->get_name();
                EXPECT_TRUE(m_0->set_pin_group_name(group, "tolle_gruppe"));
                EXPECT_EQ(group->get_name(), "tolle_gruppe");

                // set pin type
                EXPECT_TRUE(m_0->set_pin_type(pin_by_net, PinType::address));
                EXPECT_EQ(pin_by_net->get_type(), PinType::address);
                EXPECT_EQ(group->get_type(), PinType::none);
            }
            {
                // get output pin and check name, net, direction, and type
                Net* net = nl->get_net_by_id(MIN_NET_ID + 045);
                ASSERT_NE(net, nullptr);
                ModulePin* pin_by_net = m_0->get_pin_by_net(net);
                ASSERT_NE(pin_by_net, nullptr);
                EXPECT_EQ(pin_by_net->get_direction(), PinDirection::output);
                EXPECT_EQ(pin_by_net->get_type(), PinType::none);
                EXPECT_EQ(pin_by_net->get_net(), net);
                EXPECT_EQ(pin_by_net->get_name(), "O(0)");

                // get input pin groups
                PinGroup<ModulePin>* group = m_0->get_pin_group_by_id(pin_by_net->get_group().first->get_id());
                ASSERT_NE(group, nullptr);
                EXPECT_FALSE(group->empty());
                EXPECT_EQ(group->size(), 1);
                EXPECT_EQ(group->get_name(), "O(0)");
                EXPECT_EQ(group->get_pins().front(), pin_by_net);
                EXPECT_EQ(group->get_direction(), PinDirection::output);
                EXPECT_EQ(group->get_type(), PinType::none);
                EXPECT_EQ(group->get_index(pin_by_net).get(), 0);
                EXPECT_EQ(group->get_pin_at_index(0).get(), pin_by_net);
                EXPECT_EQ(group->get_start_index(), 0);
                EXPECT_EQ(group->is_ascending(), true);
                EXPECT_EQ(pin_by_net->get_group(), std::pair(group, i32(0)));

                // set pin name
                EXPECT_TRUE(m_0->set_pin_name(pin_by_net, "bester_pin"));
                EXPECT_EQ(pin_by_net->get_name(), "bester_pin");

                // set group name
                EXPECT_TRUE(m_0->set_pin_group_name(group, "beste_gruppe"));
                EXPECT_EQ(group->get_name(), "beste_gruppe");

                // set pin type
                EXPECT_TRUE(m_0->set_pin_type(pin_by_net, PinType::data));
                EXPECT_EQ(pin_by_net->get_type(), PinType::data);
                EXPECT_EQ(group->get_type(), PinType::none);
                EXPECT_TRUE(m_0->set_pin_group_type(group, PinType::address));
                EXPECT_EQ(group->get_type(), PinType::address);
            }

            // add another module, m_0 now dead
            Module* m_1 = nl->create_module("mod_1", nl->get_top_module(), {nl->get_gate_by_id(MIN_GATE_ID + 0), nl->get_gate_by_id(MIN_GATE_ID + 3), nl->get_gate_by_id(MIN_GATE_ID + 7)});
            ASSERT_NE(m_1, nullptr);
            {
                // get all pins
                ModulePin* in_pin_0 = m_1->get_pin_by_net(nl->get_net_by_id(MIN_NET_ID + 13));
                ASSERT_NE(in_pin_0, nullptr);
                ModulePin* in_pin_1 = m_1->get_pin_by_net(nl->get_net_by_id(MIN_NET_ID + 20));
                ASSERT_NE(in_pin_1, nullptr);                
                ModulePin* out_pin_0 = m_1->get_pin_by_net(nl->get_net_by_id(MIN_NET_ID + 045));
                ASSERT_NE(out_pin_0, nullptr);
                ModulePin* out_pin_1 = m_1->get_pin_by_net(nl->get_net_by_id(MIN_NET_ID + 78));
                ASSERT_NE(out_pin_1, nullptr);
                std::string old_in_pin_0_group_name = in_pin_0->get_group().first->get_name();
                std::string old_in_pin_1_group_name = in_pin_1->get_group().first->get_name();

                std::vector<ModulePin*> expected_pins = {in_pin_0, in_pin_1, out_pin_0, out_pin_1};
                EXPECT_TRUE(test_utils::vectors_have_same_content(expected_pins, m_1->get_pins()));
                EXPECT_TRUE(m_0->get_pins().empty());
                EXPECT_TRUE(m_0->get_pin_groups().empty());

                // create input pin group
                auto res = m_1->create_pin_group("I", {in_pin_0, in_pin_1}, PinDirection::input, PinType::none, true, 3);
                ASSERT_TRUE(res.is_ok());
                PinGroup<ModulePin>* in_group = res.get();
                ASSERT_NE(in_group, nullptr);
                EXPECT_EQ(m_1->get_pin_groups().size(), 3);
                EXPECT_EQ(in_group->size(), 2);
                EXPECT_FALSE(in_group->empty());
                EXPECT_EQ(in_group->get_name(), "I");
                EXPECT_EQ(in_group->get_type(), PinType::none);
                EXPECT_EQ(in_group->is_ascending(), true);
                EXPECT_EQ(in_group->get_start_index(), 3);
                EXPECT_EQ(in_group->get_direction(), PinDirection::input);
                EXPECT_EQ(in_group->get_pins(), std::vector<ModulePin*>({in_pin_0, in_pin_1}));
                EXPECT_EQ(in_group->get_index(in_pin_0).get(), 3);
                EXPECT_EQ(in_group->get_index(in_pin_1).get(), 4);
                EXPECT_EQ(in_group->get_pin_at_index(3).get(), in_pin_0);
                EXPECT_EQ(in_group->get_pin_at_index(4).get(), in_pin_1);
                EXPECT_EQ(in_pin_0->get_group(), std::pair(in_group, i32(3)));
                EXPECT_EQ(in_pin_1->get_group(), std::pair(in_group, i32(4)));

                // set group type
                EXPECT_TRUE(m_1->set_pin_type(in_pin_0, PinType::address));
                EXPECT_TRUE(m_1->set_pin_type(in_pin_1, PinType::address));
                EXPECT_TRUE(m_1->set_pin_group_type(in_group, PinType::address));
                EXPECT_EQ(in_group->get_type(), PinType::address);
                EXPECT_EQ(in_pin_0->get_type(), PinType::address);
                EXPECT_EQ(in_pin_1->get_type(), PinType::address);

                // delete pin group
                u32 group_id = in_group->get_id();
                EXPECT_TRUE(m_1->delete_pin_group(in_group));
                EXPECT_EQ(m_1->get_pin_group_by_id(group_id), nullptr);
                EXPECT_EQ(m_1->get_pin_groups().size(), 4);
                PinGroup<ModulePin>* in_group_0 = in_pin_0->get_group().first;
                ASSERT_NE(in_group_0, nullptr);
                EXPECT_EQ(in_group_0->size(), 1);
                EXPECT_FALSE(in_group_0->empty());
                EXPECT_EQ(in_group_0->get_direction(), PinDirection::input);
                EXPECT_EQ(in_group_0->get_type(), PinType::address);
                EXPECT_EQ(in_group_0->get_start_index(), 0);
                EXPECT_EQ(in_group_0->is_ascending(), true);
                EXPECT_EQ(in_pin_0->get_group(), std::pair(in_group_0, i32(0)));
                EXPECT_EQ(in_group_0->get_index(in_pin_0).get(), 0);

                // assign pin to group
                EXPECT_TRUE(m_1->assign_pin_to_group(in_group_0, in_pin_1));
                EXPECT_EQ(m_1->get_pin_groups().size(), 3);
                EXPECT_EQ(in_group_0->size(), 2);
                EXPECT_EQ(in_pin_0->get_group(), std::pair(in_group_0, i32(0)));
                EXPECT_EQ(in_group_0->get_index(in_pin_0).get(), 0);
                EXPECT_EQ(in_pin_1->get_group(), std::pair(in_group_0, i32(1)));
                EXPECT_EQ(in_group_0->get_index(in_pin_1).get(), 1);
                EXPECT_EQ(in_group_0->get_pin_at_index(0).get(), in_pin_0);
                EXPECT_EQ(in_group_0->get_pin_at_index(1).get(), in_pin_1);

                // move pins within group
                EXPECT_TRUE(m_1->move_pin_within_group(in_group_0, in_pin_1, 0));
                EXPECT_EQ(in_pin_0->get_group(), std::pair(in_group_0, i32(1)));
                EXPECT_EQ(in_group_0->get_index(in_pin_0).get(), 1);
                EXPECT_EQ(in_pin_1->get_group(), std::pair(in_group_0, i32(0)));
                EXPECT_EQ(in_group_0->get_index(in_pin_1).get(), 0);
                EXPECT_EQ(in_group_0->get_pin_at_index(1).get(), in_pin_0);
                EXPECT_EQ(in_group_0->get_pin_at_index(0).get(), in_pin_1);

                EXPECT_TRUE(m_1->move_pin_within_group(in_group_0, in_pin_1, 1));
                EXPECT_EQ(in_pin_0->get_group(), std::pair(in_group_0, i32(0)));
                EXPECT_EQ(in_group_0->get_index(in_pin_0).get(), 0);
                EXPECT_EQ(in_pin_1->get_group(), std::pair(in_group_0, i32(1)));
                EXPECT_EQ(in_group_0->get_index(in_pin_1).get(), 1);
                EXPECT_EQ(in_group_0->get_pin_at_index(0).get(), in_pin_0);
                EXPECT_EQ(in_group_0->get_pin_at_index(1).get(), in_pin_1);

                // remove pin from group
                EXPECT_TRUE(m_1->set_pin_group_name(in_group_0, "I_tmp"));
                EXPECT_TRUE(m_1->remove_pin_from_group(in_group_0, in_pin_0));
                EXPECT_EQ(in_pin_1->get_group(), std::pair(in_group_0, i32(0)));
                EXPECT_EQ(in_group_0->get_index(in_pin_1).get(), 0);
                EXPECT_EQ(in_group_0->get_pin_at_index(0).get(), in_pin_1);
                EXPECT_EQ(in_pin_0->get_group().first->get_name(), in_pin_0->get_name());
                EXPECT_EQ(in_pin_0->get_group().second, 0);
                EXPECT_TRUE(m_1->assign_pin_to_group(in_group_0, in_pin_0));
                EXPECT_EQ(in_group_0->size(), 2);

                // try mixed directions and types
                EXPECT_TRUE(m_1->set_pin_type(out_pin_0, PinType::address));
                EXPECT_TRUE(m_1->set_pin_type(out_pin_1, PinType::data));
                EXPECT_TRUE(m_1->assign_pin_to_group(in_group_0, out_pin_0)); // wrong direction
                EXPECT_TRUE(m_1->assign_pin_to_group(in_group_0, out_pin_1)); // wrong type
                EXPECT_EQ(in_group_0->size(), 4);
                EXPECT_TRUE(m_1->create_pin_group("O1", {out_pin_0, out_pin_1}).is_ok()); // different types
                EXPECT_TRUE(m_1->create_pin_group("O2", {out_pin_0, in_pin_0}).is_ok());  // different directions
                EXPECT_EQ(in_group_0->size(), 1);
                EXPECT_EQ(m_1->get_pin_groups().size(), 3);
            }
            // Create a new Module with more modules (with 2 input and ouput nets)
            Module* m_2 = nl->create_module("mod_2", nl->get_top_module(), {nl->get_gate_by_id(MIN_GATE_ID + 3)});
            ASSERT_NE(m_2, nullptr);
            // Add an input and an output port name
            ModulePin* pin_1 = m_2->get_pin_by_net(nl->get_net_by_id(MIN_NET_ID + 13));
            ASSERT_NE(pin_1, nullptr);
            EXPECT_TRUE(m_2->set_pin_name(pin_1, "port_name_net_1_3"));
            ModulePin* pin_2 = m_2->get_pin_by_net(nl->get_net_by_id(MIN_NET_ID + 30));
            ASSERT_NE(pin_2, nullptr);
            EXPECT_TRUE(m_2->set_pin_name(pin_2, "port_name_net_3_0"));
            // Add additional gates to the Module so that the port name nets are no longer input/output nets of the Module
            ASSERT_TRUE(m_2->assign_gate(nl->get_gate_by_id(MIN_GATE_ID + 1)));
            ASSERT_TRUE(m_2->assign_gate(nl->get_gate_by_id(MIN_GATE_ID + 0)));
            {
                // Get all input port names. The old ports shouldn't be contained.
                ModulePin* pin_3 = m_2->get_pin_by_net(nl->get_net_by_id(MIN_NET_ID + 20));
                ASSERT_NE(pin_3, nullptr);
                std::vector<ModulePin*> expected_input_ports = {pin_3};
                std::vector<ModulePin*> actual_input_ports = m_2->get_pins([](ModulePin* p){ return p->get_direction() == PinDirection::input; });
                EXPECT_EQ(actual_input_ports, expected_input_ports);
            }
            {
                // Get all output port names. The old ports shouldn't be contained.
                ModulePin* pin_4 = m_2->get_pin_by_net(nl->get_net_by_id(MIN_NET_ID + 37));
                ASSERT_NE(pin_4, nullptr);
                std::vector<ModulePin*> expected_input_ports = {pin_4};
                std::vector<ModulePin*> actual_input_ports = m_2->get_pins([](ModulePin* p){ return p->get_direction() == PinDirection::output; });
                EXPECT_EQ(actual_input_ports, expected_input_ports);
            }

            // NEGATIVE
            {
                NO_COUT_TEST_BLOCK;
                ModulePin* pin_5 = m_2->get_pin_by_net(nl->get_net_by_id(MIN_NET_ID + 20));
                ASSERT_NE(pin_5, nullptr);
                EXPECT_FALSE(m_2->set_pin_name(pin_5, ""));                 // empty name
                EXPECT_EQ(m_2->get_pin_by_id(0), nullptr);                  // invalid ID
                EXPECT_FALSE(m_0->set_pin_name(pin_5, "test"));             // wrong module
                EXPECT_FALSE(m_2->set_pin_name(nullptr, "port_name"));      // nullptr net
            }
        }
        {
            // add module to netlist
            auto nl = test_utils::create_example_netlist();
            ASSERT_NE(nl, nullptr);
            Module* m_1 = nl->create_module("mod", nl->get_top_module(), {nl->get_gate_by_id(MIN_GATE_ID + 0), nl->get_gate_by_id(MIN_GATE_ID + 3), nl->get_gate_by_id(MIN_GATE_ID + 7)});
            ASSERT_NE(m_1, nullptr);
            {
                // get all pins
                ModulePin* in_pin_0 = m_1->get_pin_by_net(nl->get_net_by_id(MIN_NET_ID + 13));
                ASSERT_NE(in_pin_0, nullptr);
                ModulePin* in_pin_1 = m_1->get_pin_by_net(nl->get_net_by_id(MIN_NET_ID + 20));
                ASSERT_NE(in_pin_1, nullptr);                
                ModulePin* out_pin_0 = m_1->get_pin_by_net(nl->get_net_by_id(MIN_NET_ID + 045));
                ASSERT_NE(out_pin_0, nullptr);
                ModulePin* out_pin_1 = m_1->get_pin_by_net(nl->get_net_by_id(MIN_NET_ID + 78));
                ASSERT_NE(out_pin_1, nullptr);
                std::string old_in_pin_0_group_name = in_pin_0->get_group().first->get_name();
                std::string old_in_pin_1_group_name = in_pin_1->get_group().first->get_name();

                std::vector<ModulePin*> expected_pins = {in_pin_0, in_pin_1, out_pin_0, out_pin_1};
                EXPECT_TRUE(test_utils::vectors_have_same_content(expected_pins, m_1->get_pins()));

                // create input pin group
                auto res = m_1->create_pin_group("I", {in_pin_0, in_pin_1}, PinDirection::input, PinType::none, false, 2);
                ASSERT_TRUE(res.is_ok());
                // descending group start index 2
                //     in_pin_0    "I0"   index 2
                //     in_pin_1    "I1"   index 1
                PinGroup<ModulePin>* in_group = res.get();
                ASSERT_NE(in_group, nullptr);
                EXPECT_EQ(m_1->get_pin_groups().size(), 3);
                EXPECT_EQ(in_group->size(), 2);
                EXPECT_FALSE(in_group->empty());
                EXPECT_EQ(in_group->get_name(), "I");
                EXPECT_EQ(in_group->get_type(), PinType::none);
                EXPECT_EQ(in_group->is_ascending(), false);
                EXPECT_EQ(in_group->get_start_index(), 2);
                EXPECT_EQ(in_group->get_direction(), PinDirection::input);
                EXPECT_EQ(in_group->get_pins(), std::vector<ModulePin*>({in_pin_0, in_pin_1}));
                const auto in_pin_0_index_res = in_group->get_index(in_pin_0);
                ASSERT_TRUE(in_pin_0_index_res.is_ok());
                EXPECT_EQ(in_pin_0_index_res.get(), 2);
                const auto in_pin_1_index_res = in_group->get_index(in_pin_1);
                ASSERT_TRUE(in_pin_1_index_res.is_ok());
                EXPECT_EQ(in_pin_1_index_res.get(), 1);
                const auto in_pin_0_from_index_res = in_group->get_pin_at_index(2);
                ASSERT_TRUE(in_pin_0_from_index_res.is_ok());
                EXPECT_EQ(in_pin_0_from_index_res.get(), in_pin_0);
                const auto in_pin_1_from_index_res = in_group->get_pin_at_index(1);
                ASSERT_TRUE(in_pin_1_from_index_res.is_ok());
                EXPECT_EQ(in_pin_1_from_index_res.get(), in_pin_1);
                EXPECT_EQ(in_pin_0->get_group(), std::pair(in_group, i32(2)));
                EXPECT_EQ(in_pin_1->get_group(), std::pair(in_group, i32(1)));

                // move pins within group
                EXPECT_TRUE(m_1->move_pin_within_group(in_group, in_pin_1, 2));
                // descending group start index 2
                //     in_pin_1    "I1"   index 2
                //     in_pin_0    "I0"   index 1
                EXPECT_EQ(in_pin_0->get_group(), std::pair(in_group, i32(1)));
                EXPECT_EQ(in_group->get_index(in_pin_0).get(), 1);
                EXPECT_EQ(in_pin_1->get_group(), std::pair(in_group, i32(2)));
                EXPECT_EQ(in_group->get_index(in_pin_1).get(), 2);
                EXPECT_EQ(in_group->get_pin_at_index(1).get(), in_pin_0);
                EXPECT_EQ(in_group->get_pin_at_index(2).get(), in_pin_1);

                EXPECT_TRUE(m_1->move_pin_within_group(in_group, in_pin_0, 2));
                // descending group start index 2
                //     in_pin_0    "I0"   index 2
                //     in_pin_1    "I1"   index 1
                EXPECT_EQ(in_pin_0->get_group(), std::pair(in_group, i32(2)));
                EXPECT_EQ(in_group->get_index(in_pin_0).get(), 2);
                EXPECT_EQ(in_pin_1->get_group(), std::pair(in_group, i32(1)));
                EXPECT_EQ(in_group->get_index(in_pin_1).get(), 1);
                EXPECT_EQ(in_group->get_pin_at_index(2).get(), in_pin_0);
                EXPECT_EQ(in_group->get_pin_at_index(1).get(), in_pin_1);

                // remove pin from group
                EXPECT_TRUE(m_1->remove_pin_from_group(in_group, in_pin_0));
                // descending group start index 1
                //     in_pin_1    "I1"   index 1
                EXPECT_EQ(in_group->get_start_index(), 1);
                EXPECT_EQ(in_pin_1->get_group(), std::pair(in_group, i32(1)));
                EXPECT_EQ(in_group->get_index(in_pin_1).get(), 1);
                EXPECT_EQ(in_group->get_pin_at_index(1).get(), in_pin_1);
                EXPECT_EQ(in_pin_0->get_group().first->get_name(), in_pin_0->get_name());
                EXPECT_EQ(in_pin_0->get_group().second, 0);

                // assign pin to group
                EXPECT_TRUE(m_1->assign_pin_to_group(in_group, in_pin_0));
                // descending group start index 2
                //     in_pin_0    "I0"   index 2
                //     in_pin_1    "I1"   index 1
                EXPECT_EQ(in_group->size(), 2);

                // assign same pin twice should not do anything
                EXPECT_TRUE(m_1->assign_pin_to_group(in_group, in_pin_0));
                EXPECT_EQ(m_1->get_pin_groups().size(), 3);
                EXPECT_EQ(in_group->size(), 2);
                EXPECT_EQ(in_pin_0->get_group(), std::pair(in_group, i32(2)));
                EXPECT_EQ(in_group->get_index(in_pin_0).get(), 2);
                EXPECT_EQ(in_pin_1->get_group(), std::pair(in_group, i32(1)));
                EXPECT_EQ(in_group->get_index(in_pin_1).get(), 1);
                EXPECT_EQ(in_group->get_pin_at_index(2).get(), in_pin_0);
                EXPECT_EQ(in_group->get_pin_at_index(1).get(), in_pin_1);
            }
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
                test_mod->set_pin_name(test_mod->get_pin_by_net(test_gate->get_fan_in_net("I0")), "mod_in_0");
            };
            std::function<void(void)> trigger_output_port_name_changed = [=](){
                test_mod->set_pin_name(test_mod->get_pin_by_net(test_gate->get_fan_out_net("O")), "mod_out");
            };

            // The events that are tested
            std::vector<ModuleEvent::event> event_type = {
                ModuleEvent::event::name_changed, ModuleEvent::event::type_changed,
                ModuleEvent::event::parent_changed, ModuleEvent::event::submodule_added,
                ModuleEvent::event::submodule_removed, ModuleEvent::event::gate_assigned, ModuleEvent::event::pin_changed, ModuleEvent::event::pin_changed,
                ModuleEvent::event::gate_removed};

            // A list of the functions that will trigger its associated event exactly once
            std::vector<std::function<void(void)>> trigger_event = { trigger_name_changed, trigger_type_changed,
                 trigger_parent_changed, trigger_submodule_added, trigger_submodule_removed, trigger_gate_assigned, trigger_input_port_name_changed, trigger_output_port_name_changed, trigger_gate_removed};

            // The parameters of the events that are expected
            std::vector<std::tuple<ModuleEvent::event, Module*, u32>> expected_parameter = {
                std::make_tuple(ModuleEvent::event::name_changed, test_mod, NO_DATA),
                std::make_tuple(ModuleEvent::event::type_changed, test_mod, NO_DATA),
                std::make_tuple(ModuleEvent::event::parent_changed, test_mod, NO_DATA),
                std::make_tuple(ModuleEvent::event::submodule_added, test_mod, other_mod_sub->get_id()),
                std::make_tuple(ModuleEvent::event::submodule_removed, test_mod, other_mod_sub->get_id()),
                std::make_tuple(ModuleEvent::event::gate_assigned, test_mod, test_gate->get_id()),
                std::make_tuple(ModuleEvent::event::pin_changed, test_mod, PinChangedEvent(test_mod,PinEvent::PinRename,3).associated_data()),
                std::make_tuple(ModuleEvent::event::pin_changed, test_mod, PinChangedEvent(test_mod,PinEvent::PinRename,1).associated_data()),
                std::make_tuple(ModuleEvent::event::gate_removed, test_mod, test_gate->get_id())
            };

            // Check all events in a for-loop
            for(u32 event_idx = 0; event_idx < event_type.size(); event_idx++)
            {
                // Create the listener for the tested event
                test_utils::EventListener<void, ModuleEvent::event, Module*, u32> listener;
                std::function<void(ModuleEvent::event, Module*, u32)> cb = listener.get_conditional_callback(
                    [=](ModuleEvent::event ev, Module* m, u32 /*id*/){return ev == event_type[event_idx] && m == test_mod;}
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
                [=](ModuleEvent::event ev, Module* /*m*/, u32 /*id*/){return ev == ModuleEvent::event::created;}
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
                [=](ModuleEvent::event ev, Module* /*m*/, u32 /*id*/){return ev == ModuleEvent::event::removed;}
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
