#include "hal_core/netlist/module.h"
#include "hal_core/netlist/event_system/gate_event_handler.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "netlist_test_utils.h"
#include "hal_core/netlist/event_system/module_event_handler.h"

namespace hal {

    using test_utils::MIN_MODULE_ID;
    using test_utils::MIN_GATE_ID;
    using test_utils::MIN_NET_ID;

    class ModuleTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
            test_utils::init_log_channels();
        }

        virtual void TearDown() {
        }
    };

    /**
     * Testing the access on the id, the type and the stored netlist after calling the constructor
     *
     * Functions: constructor, get_id, get_name
     */
    TEST_F(ModuleTest, check_constructor) {
        TEST_START
            {
                // Creating a Module of id 123 and type "test Module"
                auto nl = test_utils::create_empty_netlist();
                Module*
                    test_module = nl->create_module(MIN_MODULE_ID + 123, "test Module", nl->get_top_module());

                EXPECT_EQ(test_module->get_id(), (u32) (MIN_MODULE_ID + 123));
                EXPECT_EQ(test_module->get_name(), "test Module");
                EXPECT_EQ(test_module->get_netlist(), nl.get());
            }
        TEST_END
    }

    /**
     * Testing the set_name function of Module
     *
     * Functions: set_name
     */
    TEST_F(ModuleTest, check_set_id) {
        TEST_START
            {
                // Set a new name for Module
                auto nl = test_utils::create_empty_netlist();
                Module*
                    test_module = nl->create_module(MIN_MODULE_ID + 0, "test_module", nl->get_top_module());

                test_module->set_name("new_name");
                EXPECT_EQ(test_module->get_name(), "new_name");
            }
            {
                // Set an already set name
                auto nl = test_utils::create_empty_netlist();
                Module*
                    test_module = nl->create_module(MIN_MODULE_ID + 0, "test_module", nl->get_top_module());

                test_module->set_name("test_module");
                EXPECT_EQ(test_module->get_name(), "test_module");
            }
            {
                // Set an empty name
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Module*
                    test_module = nl->create_module(MIN_MODULE_ID + 0, "test_module", nl->get_top_module());

                test_module->set_name("");
                EXPECT_EQ(test_module->get_name(), "test_module");
            }
        TEST_END
    }

    /**
     * Testing the access on the modules type
     *
     * Functions: set_type, get_type
     */
    TEST_F(ModuleTest, check_module_type) {
        TEST_START
            {
                // Set a new type for Module
                auto nl = test_utils::create_empty_netlist();
                Module*
                    test_module = nl->create_module(MIN_MODULE_ID + 0, "test_module", nl->get_top_module());

                test_module->set_type("new_type");
                EXPECT_EQ(test_module->get_type(), "new_type");
            }
            {
                // Set an already set type
                auto nl = test_utils::create_empty_netlist();
                Module*
                    test_module = nl->create_module(MIN_MODULE_ID + 0, "test_module", nl->get_top_module());

                test_module->set_type("new_type");
                test_module->set_type("new_type");
                EXPECT_EQ(test_module->get_type(), "new_type");
            }
        TEST_END
    }

    /**
     * Testing the set_parent_module function
     *
     * Functions: set_parent_module
     */
    TEST_F(ModuleTest, check_set_parent_module) {
        TEST_START
            // POSITIVE
            {
                /*  Consider the Module scheme below. We set the parent_module of m_0 from the top_module to m_1
                 *
                 *                  .--> m_2
                 *        .--> m_0 -|
                 *   top -|         '--> m_3     ==>      top-.                 .--> m_2
                 *        '--> m_1                            '-> m_1 --> m_0 --|
                 *                                                              '--> m_3
                 *
                 */
                auto nl = test_utils::create_empty_netlist();
                Module*
                    m_0 = nl->create_module(MIN_MODULE_ID + 0, "test_module_0", nl->get_top_module());
                Module*
                    m_1 = nl->create_module(MIN_MODULE_ID + 1, "test_module_1", nl->get_top_module());
                Module* m_2 = nl->create_module(MIN_MODULE_ID + 2, "test_module_2", m_0);
                Module* m_3 = nl->create_module(MIN_MODULE_ID + 3, "test_module_3", m_0);

                m_0->set_parent_module(m_1);
                EXPECT_EQ(m_0->get_parent_module(), m_1);
                EXPECT_FALSE(m_1->get_submodules(test_utils::module_name_filter("test_module_0"), false).empty());
                EXPECT_FALSE(m_1->get_submodules(test_utils::module_name_filter("test_module_2"), true).empty());
                EXPECT_FALSE(m_1->get_submodules(test_utils::module_name_filter("test_module_3"), true).empty());
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
                auto nl = test_utils::create_empty_netlist();
                Module*
                    m_0 = nl->create_module(MIN_MODULE_ID + 0, "test_module_0", nl->get_top_module());
                Module* m_1 = nl->create_module(MIN_MODULE_ID + 1, "test_module_1", m_0);
                Module* m_2 = nl->create_module(MIN_MODULE_ID + 2, "test_module_2", m_0);

                m_0->set_parent_module(m_1);
                EXPECT_EQ(m_1->get_parent_module(), nl->get_top_module());
                EXPECT_EQ(m_0->get_parent_module(), m_1);
                EXPECT_EQ(m_2->get_parent_module(), m_0);
            }
            // NEGATIVE
            {
                // Hang a Module to itself
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module(MIN_MODULE_ID+0, "test_module_0", nl->get_top_module());

                m_0->set_parent_module(m_0);
                EXPECT_EQ(m_0->get_parent_module(), nl->get_top_module());
            }
            {
                // Try to change the parent of the top_module
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Module*
                    m_0 = nl->create_module(MIN_MODULE_ID + 0, "test_module_0", nl->get_top_module());

                nl->get_top_module()->set_parent_module(m_0);
                EXPECT_EQ(m_0->get_parent_module(), nl->get_top_module());
                EXPECT_EQ(nl->get_top_module()->get_parent_module(), nullptr);
            }
            {
                // new_parent is a nullptr
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module(MIN_MODULE_ID+0, "test_module_0", nl->get_top_module());

                m_0->set_parent_module(nullptr);
                nl->get_top_module()->set_parent_module(m_0);
                EXPECT_EQ(m_0->get_parent_module(), nl->get_top_module());
            }
            {
                // new_parent not part of the netlist (anymore)
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                auto nl_other = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module(MIN_MODULE_ID+0, "test_module_0", nl->get_top_module());
                Module* m_1 = nl->create_module(MIN_MODULE_ID+1, "test_module_1", nl->get_top_module());
                nl->delete_module(m_0); // m_0 is removed from the netlist

                m_1->set_parent_module(m_0);
                EXPECT_EQ(m_1->get_parent_module(), nl->get_top_module());
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
                Module* m_0 = nl->create_module(MIN_MODULE_ID + 0, "test_module", nl->get_top_module());
                Gate*
                    gate_0 =
                    nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0");
                m_0->assign_gate(gate_0);

                EXPECT_TRUE(m_0->contains_gate(gate_0));
            }
            {
                // Check a Gate, that isn't part of the Module (not recursive)
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module(MIN_MODULE_ID + 0, "test_module", nl->get_top_module());
                Gate*
                    gate_0 =
                    nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0");

                EXPECT_FALSE(m_0->contains_gate(gate_0));
            }
            {
                // Check a Gate, that isn't part of the Module, but of a submodule (not recursive)
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module(MIN_MODULE_ID + 0, "test_module", nl->get_top_module());
                Module* submodule = nl->create_module(MIN_MODULE_ID + 1, "test_module", m_0);
                ASSERT_NE(submodule, nullptr);
                Gate*
                    gate_0 =
                    nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0");
                submodule->assign_gate(gate_0);

                EXPECT_FALSE(m_0->contains_gate(gate_0));
            }
            {
                // Check a Gate, that isn't part of the Module, but of a submodule (recursive)
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module(MIN_MODULE_ID + 0, "test_module", nl->get_top_module());
                Module* submodule = nl->create_module(MIN_MODULE_ID + 1, "test_module", m_0);
                ASSERT_NE(submodule, nullptr);
                Gate*
                    gate_0 =
                    nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0");
                submodule->assign_gate(gate_0);

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
                Gate*
                    gate_0 =
                    nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0");
                Gate*
                    gate_1 =
                    nl->create_gate(MIN_GATE_ID + 1, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_1");
                // this Gate is not part of the Module
                Gate* gate_not_in_m =
                    nl->create_gate(MIN_GATE_ID + 2, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_not_in_m");

                // Add gate_0 and gate_1 to a Module
                Module*
                    test_module = nl->create_module(MIN_MODULE_ID + 0, "test Module", nl->get_top_module());
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
                Gate*
                    gate_0 =
                    nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0");

                // Add gate_0 twice
                Module*
                    test_module = nl->create_module(MIN_MODULE_ID + 0, "test Module", nl->get_top_module());
                test_module->assign_gate(gate_0);
                test_module->assign_gate(gate_0);

                std::vector<Gate*> expRes = {
                    gate_0,
                };

                EXPECT_EQ(test_module->get_gates(), expRes);
                EXPECT_TRUE(test_module->contains_gate(gate_0));
            }
            {
                // Insert a Gate owned by a submodule
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Gate*
                    gate_0 =
                    nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0");

                Module*
                    test_module = nl->create_module(MIN_MODULE_ID + 0, "test Module", nl->get_top_module());
                Module* submodule = nl->create_module(MIN_MODULE_ID + 1, "submodule", test_module);
                submodule->assign_gate(gate_0);
                ASSERT_TRUE(submodule->contains_gate(gate_0));
                ASSERT_FALSE(test_module->contains_gate(gate_0));

                test_module->assign_gate(gate_0);

                std::vector<Gate*> expRes = {
                    gate_0
                };

                EXPECT_EQ(test_module->get_gates(), expRes);
                EXPECT_FALSE(submodule->contains_gate(gate_0));
            }

            // NEGATIVE
            {
                // Assigned Gate is a nullptr
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Module*
                    test_module = nl->create_module(MIN_MODULE_ID + 0, "test Module", nl->get_top_module());
                test_module->assign_gate(nullptr);
                EXPECT_TRUE(test_module->get_gates().empty());
            }
            {
                // Call contains_gate with a nullptr
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Module*
                    test_module = nl->create_module(MIN_MODULE_ID + 0, "test Module", nl->get_top_module());
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
                Module* m_0 = nl->create_module(MIN_MODULE_ID + 0, "test_module", nl->get_top_module());
                Gate*
                    gate_0 =
                    nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0");
                m_0->assign_gate(gate_0);

                ASSERT_TRUE(m_0->contains_gate(gate_0));
                m_0->remove_gate(gate_0);
                EXPECT_FALSE(m_0->contains_gate(gate_0));
            }
            {
                // Try to delete a Gate from a Module (Gate owned by another Module)
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module(MIN_MODULE_ID + 0, "test_module", nl->get_top_module());
                Module*
                    m_other = nl->create_module(MIN_MODULE_ID + 1, "other_test_module", nl->get_top_module());
                Gate*
                    gate_0 =
                    nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0");
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
                Gate*
                    gate_0 =
                    nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0");
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
                Module* m_0 = nl->create_module(MIN_MODULE_ID + 0, "test_module", nl->get_top_module());
                Gate*
                    gate_123 =
                    nl->create_gate(MIN_GATE_ID + 123, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_123");
                m_0->assign_gate(gate_123);

                ASSERT_TRUE(m_0->contains_gate(gate_123));
                EXPECT_EQ(m_0->get_gate_by_id(MIN_GATE_ID + 123), gate_123);
            }
            {
                // get a Gate by its id (not owned by a submodule)(not recursive)
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module(MIN_MODULE_ID + 0, "test_module", nl->get_top_module());
                Module* submodule = nl->create_module(MIN_MODULE_ID + 1, "other_module", m_0);
                Gate*
                    gate_123 =
                    nl->create_gate(MIN_GATE_ID + 123, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_123");
                submodule->assign_gate(gate_123);

                EXPECT_EQ(m_0->get_gate_by_id(MIN_GATE_ID + 123), nullptr);
            }
            {
                // get a Gate by its id (not owned by a submodule)(recursive)
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module(MIN_MODULE_ID + 0, "test_module", nl->get_top_module());
                Module* submodule = nl->create_module(MIN_MODULE_ID + 1, "other_module", m_0);
                Gate*
                    gate_123 =
                    nl->create_gate(MIN_GATE_ID + 123, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_123");
                submodule->assign_gate(gate_123);

                EXPECT_EQ(m_0->get_gate_by_id(MIN_GATE_ID + 123, true), gate_123);
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
            Module* m_0 = nl->create_module(MIN_MODULE_ID + 0, "even_module", tm);
            ASSERT_NE(m_0, nullptr);
            Module* m_1 = nl->create_module(MIN_MODULE_ID + 1, "odd_module", tm);
            ASSERT_NE(m_1, nullptr);
            Module* m_2 = nl->create_module(MIN_MODULE_ID + 2, "even_module", m_1);
            ASSERT_NE(m_2, nullptr);
            Module* m_3 = nl->create_module(MIN_MODULE_ID + 3, "odd_module", m_1);
            ASSERT_NE(m_3, nullptr);
            {
                // Testing the access on submodules (no module_name_filter, not recursive)
                {
                    // Submodules of TOP_MODULE;
                    std::vector<Module*> exp_result = {m_0, m_1};
                    EXPECT_TRUE(test_utils::vectors_have_same_content(tm->get_submodules(nullptr, false), exp_result));
                    EXPECT_TRUE(tm->contains_module(m_0, false));
                    EXPECT_TRUE(tm->contains_module(m_1, false));
                    EXPECT_FALSE(tm->contains_module(m_2, false));
                    EXPECT_FALSE(tm->contains_module(m_3, false));
                }
                {
                    // Submodules of MODULE_1;
                    std::vector<Module*> exp_result = {m_2, m_3};
                    EXPECT_TRUE(test_utils::vectors_have_same_content(m_1->get_submodules(nullptr, false), exp_result));
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
                    EXPECT_EQ(tm->get_submodules(test_utils::module_name_filter("even_module"), false), exp_result);
                }
                {
                    // Submodules of MODULE_1;
                    std::vector<Module*> exp_result = {m_2};
                    EXPECT_EQ(m_1->get_submodules(test_utils::module_name_filter("even_module"), false), exp_result);
                }
                {
                    // Submodules of TOP_MODULE (name does not exists);
                    std::vector<Module*> exp_result = {};
                    EXPECT_EQ(tm->get_submodules(test_utils::module_name_filter("non_existing_name"), false),
                              exp_result);
                }
            }
            {
                // Testing the access on submodules (recursive)
                {
                    // Submodules of TOP_MODULE;
                    std::vector<Module*> exp_result = {m_0, m_1, m_2, m_3};
                    EXPECT_TRUE(test_utils::vectors_have_same_content(tm->get_submodules(nullptr, true), exp_result));
                    EXPECT_TRUE(tm->contains_module(m_0, true));
                    EXPECT_TRUE(tm->contains_module(m_1, true));
                    EXPECT_TRUE(tm->contains_module(m_2, true));
                    EXPECT_TRUE(tm->contains_module(m_3, true));
                }
                {
                    // Submodules of TOP_MODULE (with module_name_filter);
                    std::vector<Module*> exp_result = {m_0, m_2};
                    EXPECT_TRUE(test_utils::vectors_have_same_content(tm->get_submodules(test_utils::module_name_filter("even_module"), true), exp_result));
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
    TEST_F(ModuleTest, check_get_input_nets) {
        TEST_START
            // +++ Create the example netlist (see above)

            auto nl = test_utils::create_empty_netlist();

            // Add the gates
            Gate*
                gate_0 = nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0");
            Gate*
                gate_1 = nl->create_gate(MIN_GATE_ID + 1, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_1");
            Gate*
                gate_2 = nl->create_gate(MIN_GATE_ID + 2, test_utils::get_gate_type_by_name("gate_2_to_1"), "gate_2");
            Gate*
                gate_3 = nl->create_gate(MIN_GATE_ID + 3, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_3");
            Gate*
                gate_4 = nl->create_gate(MIN_GATE_ID + 4, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_4");
            Gate*
                gate_5 = nl->create_gate(MIN_GATE_ID + 5, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_5");

            // Add the nets (net_x_y1_y2_... is Net from x to y1,y2,... (g = global input/output))
            Net* net_g_0 = nl->create_net(MIN_NET_ID + 0, "name_0");
            Net* net_0_g = nl->create_net(MIN_NET_ID + 1, "name_0");
            Net* net_1_2 = nl->create_net(MIN_NET_ID + 3, "name_0");
            Net* net_4_1_2 = nl->create_net(MIN_NET_ID + 4, "name_1");
            Net* net_2_3_5 = nl->create_net(MIN_NET_ID + 5, "name_1");

            // Connect the nets
            net_g_0->add_destination(gate_0, "I");

            net_0_g->add_source(gate_0, "O");

            net_4_1_2->add_source(gate_4, "O");
            net_4_1_2->add_destination(gate_1, "I");
            net_4_1_2->add_destination(gate_2, "I1");

            net_1_2->add_source(gate_1, "O");
            net_1_2->add_destination(gate_2, "I0");

            net_2_3_5->add_source(gate_2, "O");
            net_2_3_5->add_destination(gate_3, "I");
            net_2_3_5->add_destination(gate_5, "I");

            // Mark global nets
            nl->mark_global_input_net(net_g_0);
            nl->mark_global_output_net(net_0_g);

            // Create the Module
            auto test_module = nl->create_module(MIN_MODULE_ID + 0, "test_module", nl->get_top_module());
            for (auto g : std::set<Gate*>({gate_0, gate_1, gate_2, gate_3})) {
                test_module->assign_gate(g);
            }
            {
                // Get input nets of the test Module
                std::vector<Net*> exp_result = {net_g_0, net_4_1_2};
                EXPECT_TRUE(test_utils::vectors_have_same_content(test_module->get_input_nets(), exp_result));
            }
            {
                // Get output nets of the test Module
                std::vector<Net*> exp_result = {net_0_g, net_2_3_5};
                EXPECT_TRUE(test_utils::vectors_have_same_content(test_module->get_output_nets(), exp_result));
            }
            {
                // Get internal nets of the test Module
                std::vector<Net*> exp_result = {net_1_2, net_2_3_5};
                EXPECT_TRUE(test_utils::vectors_have_same_content(test_module->get_internal_nets(), exp_result));
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
            Module* m_0 = nl->create_module("mod_0",
                                                            nl->get_top_module(),
                                                            {nl->get_gate_by_id(MIN_GATE_ID + 0),
                                                             nl->get_gate_by_id(MIN_GATE_ID + 3)});
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
            Module* m_1 = nl->create_module("mod_1",
                                                            nl->get_top_module(),
                                                            {nl->get_gate_by_id(MIN_GATE_ID + 0),
                                                             nl->get_gate_by_id(MIN_GATE_ID + 3),
                                                             nl->get_gate_by_id(MIN_GATE_ID + 7)});
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
            std::vector<module_event_handler::event> event_type = {
                module_event_handler::event::name_changed, module_event_handler::event::type_changed,
                module_event_handler::event::parent_changed, module_event_handler::event::submodule_added,
                module_event_handler::event::submodule_removed, module_event_handler::event::gate_assigned,
                module_event_handler::event::gate_removed, module_event_handler::event::input_port_name_changed,
                module_event_handler::event::output_port_name_changed};

            // A list of the functions that will trigger its associated event exactly once
            std::vector<std::function<void(void)>> trigger_event = { trigger_name_changed, trigger_type_changed,
                 trigger_parent_changed, trigger_submodule_added, trigger_submodule_removed, trigger_gate_assigned,
                 trigger_gate_removed, trigger_input_port_name_changed, trigger_output_port_name_changed};

            // The parameters of the events that are expected
            std::vector<std::tuple<module_event_handler::event, Module*, u32>> expected_parameter = {
                std::make_tuple(module_event_handler::event::name_changed, test_mod, NO_DATA),
                std::make_tuple(module_event_handler::event::type_changed, test_mod, NO_DATA),
                std::make_tuple(module_event_handler::event::parent_changed, test_mod, NO_DATA),
                std::make_tuple(module_event_handler::event::submodule_added, test_mod, other_mod_sub->get_id()),
                std::make_tuple(module_event_handler::event::submodule_removed, test_mod, other_mod_sub->get_id()),
                std::make_tuple(module_event_handler::event::gate_assigned, test_mod, test_gate->get_id()),
                std::make_tuple(module_event_handler::event::gate_removed, test_mod, test_gate->get_id()),
                std::make_tuple(module_event_handler::event::input_port_name_changed, test_mod, test_gate->get_fan_in_net("I0")->get_id()),
                std::make_tuple(module_event_handler::event::output_port_name_changed, test_mod, test_gate->get_fan_out_net("O")->get_id())
            };

            // Check all events in a for-loop
            for(u32 event_idx = 0; event_idx < event_type.size(); event_idx++)
            {
                // Create the listener for the tested event
                test_utils::EventListener<void, module_event_handler::event, Module*, u32> listener;
                std::function<void(module_event_handler::event, Module*, u32)> cb = listener.get_conditional_callback(
                    [=](module_event_handler::event ev, Module* m, u32 id){return ev == event_type[event_idx] && m == test_mod;}
                );
                std::string cb_name = "mod_event_callback_" + std::to_string((u32)event_type[event_idx]);
                // Register a callback of the listener
                module_event_handler::register_callback(cb_name, cb);

                // Trigger the event
                trigger_event[event_idx]();

                EXPECT_EQ(listener.get_event_count(), 1);
                EXPECT_EQ(listener.get_last_parameters(), expected_parameter[event_idx]);

                // Unregister the callback
                module_event_handler::unregister_callback(cb_name);
            }

            // Test the events 'created' and 'removed'
            // -- 'created' event
            test_utils::EventListener<void, module_event_handler::event, Module*, u32> listener_created;
            std::function<void(module_event_handler::event, Module*, u32)> cb_created = listener_created.get_conditional_callback(
                [=](module_event_handler::event ev, Module* m, u32 id){return ev == module_event_handler::created;}
            );
            std::string cb_name_created = "mod_event_callback_created";
            module_event_handler::register_callback(cb_name_created, cb_created);

            // Create a new mod
            Module* new_mod = test_nl->create_module("new_mod", test_nl->get_top_module());
            EXPECT_EQ(listener_created.get_event_count(), 1);
            EXPECT_EQ(listener_created.get_last_parameters(), std::make_tuple(module_event_handler::event::created, new_mod, NO_DATA));

            module_event_handler::unregister_callback(cb_name_created);

            // -- 'removed' event
            test_utils::EventListener<void, module_event_handler::event, Module*, u32> listener_removed;
            std::function<void(module_event_handler::event, Module*, u32)> cb_removed = listener_removed.get_conditional_callback(
                [=](module_event_handler::event ev, Module* m, u32 id){return ev == module_event_handler::removed;}
            );
            std::string cb_name_removed = "mod_event_callback_removed";
            module_event_handler::register_callback(cb_name_removed, cb_removed);

            // Delete the module which was created in the previous part
            test_nl->delete_module(new_mod);
            EXPECT_EQ(listener_removed.get_event_count(), 1);
            EXPECT_EQ(listener_removed.get_last_parameters(), std::make_tuple(module_event_handler::event::removed, new_mod, NO_DATA));

            module_event_handler::unregister_callback(cb_name_removed);

        TEST_END
    }

} //namespace hal
