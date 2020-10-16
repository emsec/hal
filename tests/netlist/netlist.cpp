#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "netlist_test_utils.h"
#include "hal_core/netlist/event_system/netlist_event_handler.h"

namespace hal {
    using test_utils::MIN_NETLIST_ID;
    using test_utils::MIN_MODULE_ID;
    using test_utils::MIN_GATE_ID;
    using test_utils::MIN_NET_ID;

    class NetlistTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
            test_utils::init_log_channels();
        }

        virtual void TearDown() {
        }
    };

    /**
     * Testing the access on the id
     *
     * Functions: get_id, set_id
     */
    TEST_F(NetlistTest, check_id_access) {
        TEST_START
            // Create an empty netlist
            auto nl = test_utils::create_empty_netlist();
            // Set the id to another value
            nl->set_id(MIN_NETLIST_ID + 123);
            EXPECT_EQ(nl->get_id(), (u32) (MIN_NETLIST_ID + 123));
            // Set the id to the same value again
            nl->set_id(MIN_NETLIST_ID + 123);
            EXPECT_EQ(nl->get_id(), (u32) (MIN_NETLIST_ID + 123));
        TEST_END
    }

    /**
    * Testing the access on the input filename
    *
    * Functions: get_id, set_id
    */
    TEST_F(NetlistTest, check_input_filename_access) {
        TEST_START
            // Create an empty netlist
            auto nl = test_utils::create_empty_netlist();
            // The filename should be empty initially
            EXPECT_EQ(nl->get_input_filename(), std::filesystem::path(""));
            // Set a filename
            nl->set_input_filename("/this/is/a/filename");
            EXPECT_EQ(nl->get_input_filename(), std::filesystem::path("/this/is/a/filename"));
            // Set the same filename again
            nl->set_input_filename("/this/is/a/filename");
            EXPECT_EQ(nl->get_input_filename(), std::filesystem::path("/this/is/a/filename"));
        TEST_END
    }

    /**
    * Testing the access on the design name
    *
    * Functions: get_design_name, set_design_name
    */
    TEST_F(NetlistTest, check_design_access) {
        TEST_START
            // Create an empty netlist
            auto nl = test_utils::create_empty_netlist();
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
    TEST_F(NetlistTest, check_device_access) {
        TEST_START
            // Create an empty netlist
            auto nl = test_utils::create_empty_netlist();
            // Set a design name
            nl->set_device_name("device_name");
            EXPECT_EQ(nl->get_device_name(), "device_name");
            // Set the same design name again
            nl->set_device_name("device_name");
            EXPECT_EQ(nl->get_device_name(), "device_name");
        TEST_END
    }

    /**
    * Testing the access on the stored pointers of the NetlistInternalManager,
     * the module_manager and the module_manager
    *
    * Functions: get_gate_library
    */
    TEST_F(NetlistTest, check_pointer_access) {
        TEST_START
            // Create an empty netlist
            auto nl = test_utils::create_empty_netlist();
            EXPECT_NE(nl->get_gate_library(), nullptr);
        TEST_END
    }

    /**
     * Testing the get_unique_gate_id function by creating a netlist with gates of the id
     * 0,1,2,4. The new Gate id should be 3.
     *
     * Functions: get_unique_gate_id
     */
    TEST_F(NetlistTest, check_get_unique_gate_id) {
        TEST_START
            // Create an empty netlist with some gates
            auto nl = test_utils::create_empty_netlist();
            Gate*
                g_0 = nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0");
            Gate*
                g_1 = nl->create_gate(MIN_GATE_ID + 1, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_1");
            Gate*
                g_2 = nl->create_gate(MIN_GATE_ID + 3, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_2");
            std::set<u32> used_ids = {MIN_GATE_ID + 0, MIN_GATE_ID + 1, MIN_GATE_ID + 3};

            // Get a unique id
            u32 unique_id = nl->get_unique_gate_id();
            EXPECT_TRUE(used_ids.find(unique_id) == used_ids.end());
            EXPECT_NE(unique_id, test_utils::INVALID_GATE_ID);

            // Insert the unique id Gate and get a new unique id
            Gate*
                g_new = nl->create_gate(unique_id, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_2");
            used_ids.insert(unique_id);

            unique_id = nl->get_unique_gate_id();
            EXPECT_TRUE(used_ids.find(unique_id) == used_ids.end());
            EXPECT_NE(unique_id, test_utils::INVALID_GATE_ID);

            // Remove a Gate and get a new unique id
            nl->delete_gate(g_1);
            used_ids.erase(MIN_GATE_ID + 1);

            unique_id = nl->get_unique_gate_id();
            EXPECT_TRUE(used_ids.find(unique_id) == used_ids.end());
            EXPECT_NE(unique_id, test_utils::INVALID_GATE_ID);

        TEST_END
    }

    /**
     * Testing get_num_of_gates function
     *
     * Functions: get_num_of_gates
     */
    TEST_F(NetlistTest, check_get_num_of_gates) {
        TEST_START
            // Create an empty netlist with 4 gates
            auto nl = test_utils::create_empty_netlist();
            Gate* g_0 =
                nl->create_gate(nl->get_unique_gate_id(), test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0");
            Gate* g_1 =
                nl->create_gate(nl->get_unique_gate_id(), test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_1");
            Gate* g_2 =
                nl->create_gate(nl->get_unique_gate_id(), test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_2");
            Gate* g_3 =
                nl->create_gate(nl->get_unique_gate_id(), test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_4");

            EXPECT_EQ(nl->get_gates().size(), (size_t) 4);

        TEST_END
    }

    /**
     * Testing addition of gates (calls the function create_gate in NetlistInternalManager)
     *
     * Functions: create_gate
     */
    TEST_F(NetlistTest, check_create_gate) {
        TEST_START
            {// Add a Gate the normal way
                auto nl = test_utils::create_empty_netlist();
                auto g_0 = nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0");
                EXPECT_TRUE(nl->is_gate_in_netlist(g_0));
            }
            {
                // Add a Gate, remove it afterwards and add it again (used to test the free_ids logic)
                auto nl = test_utils::create_empty_netlist();
                auto g_0 = nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0");
                nl->delete_gate(g_0);
                auto g_new = nl->create_gate(test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0");
                EXPECT_TRUE(nl->is_gate_in_netlist(g_new));
            }
            // NEGATIVE
            {
                // Try to add the same Gate twice
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                auto g_0 = nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0");
                auto g_1 = nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0");
                EXPECT_NE(g_0, nullptr);
                EXPECT_EQ(g_1, nullptr);
            }
            {
                // Try to add two gates with the same id
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                auto g_0 = nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0");
                Gate* g_0_other =
                    nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0_other");
                EXPECT_EQ(g_0_other, nullptr);
            }
            {
                // Try to add a Gate with an invalid id
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Gate* g_invalid = nl->create_gate(test_utils::INVALID_GATE_ID,
                                                                  test_utils::get_gate_type_by_name("gate_1_to_1"),
                                                                  "gate_0");
                EXPECT_EQ(g_invalid, nullptr);
            }
            {
                // Try to add a Gate with an invalid name (empty string)
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                auto g_0 = nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "");
                EXPECT_EQ(g_0, nullptr);
            }
            {
                // Try to add a Gate with a nullptr gate type
                NO_COUT_TEST_BLOCK;
                std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
                auto g_0 = nl->create_gate(MIN_GATE_ID + 0, nullptr, "");
                EXPECT_EQ(g_0, nullptr);
            }
            {
                // Try to add a Gate with an invalid gate type
                NO_COUT_TEST_BLOCK;
                std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
                // Create a GateType of another gate library
                std::unique_ptr<GateLibrary> gl = std::make_unique<GateLibrary>("imaginary_path", "OtherLibrary");
                std::unique_ptr<GateType> not_in_gl_gate_type = std::make_unique<GateType>("not_in_gl_gate");
                not_in_gl_gate_type->add_output_pins({"O"});
                gl->add_gate_type(std::move(not_in_gl_gate_type));

                auto g_0 = nl->create_gate(MIN_GATE_ID + 0, gl->get_gate_types()["not_in_gl_gate"], "");
                EXPECT_EQ(g_0, nullptr);
            }
        TEST_END
    }

    /**
     * Testing deletion of gates (calls the function delete_gate in NetlistInternalManager)
     *
     * Functions: delete_gate
     */
    TEST_F(NetlistTest, check_delete_gate) {
        TEST_START
            // POSITIVE
            {// Add and delete an unconnected Gate in a normal way
                auto nl = test_utils::create_empty_netlist();
                auto g_0 = nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0");
                bool suc = nl->delete_gate(g_0);
                EXPECT_TRUE(suc);
            }
            {
                // Delete a Gate, which is connected to some in and output nets.
                // The nets source and destination should be updated
                auto nl = test_utils::create_example_netlist();
                Gate* gate_0 = nl->get_gate_by_id(MIN_GATE_ID + 0);
                bool suc = nl->delete_gate(gate_0);
                EXPECT_TRUE(suc);
                EXPECT_FALSE(nl->is_gate_in_netlist(gate_0));
                EXPECT_TRUE(nl->get_net_by_id(MIN_NET_ID + 30)->get_destinations([gate_0](auto ep){return ep->get_gate() == gate_0;}).empty());
                EXPECT_TRUE(nl->get_net_by_id(MIN_NET_ID + 20)->get_destinations([gate_0](auto ep){return ep->get_gate() == gate_0;}).empty());
                EXPECT_EQ(nl->get_net_by_id(MIN_NET_ID + 045)->get_sources().size(), 0);
            }
            {
                // Add and delete global_gnd Gate
                auto nl = test_utils::create_empty_netlist();
                auto g_0 = nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gnd"), "gate_0");
                nl->mark_gnd_gate(g_0);
                bool suc = nl->delete_gate(g_0);
                EXPECT_TRUE(suc);
                EXPECT_FALSE(nl->is_gate_in_netlist(g_0));
                EXPECT_TRUE(nl->get_gnd_gates().empty());
            }
            {
                // Add and delete global_vcc Gate
                auto nl = test_utils::create_empty_netlist();
                auto g_0 = nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("vcc"), "gate_0");
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
                auto nl = test_utils::create_empty_netlist();
                bool suc = nl->delete_gate(nullptr);
                EXPECT_FALSE(suc);
            }
            {
                // Try to delete a Gate which is not part of the netlist
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                auto g_0 = nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0");
                bool suc = nl->delete_gate(g_0);
                EXPECT_TRUE(suc);
            }

        TEST_END
    }

    /**
     * Testing the function is_gate_in_netlist
     *
     * Functions: is_gate_in_netlist
     */
    TEST_F(NetlistTest, check_is_gate_in_netlist) {
        TEST_START
            {// Gate is part of the netlist
                auto nl = test_utils::create_empty_netlist();
                auto g_0 = nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0");
                EXPECT_TRUE(nl->is_gate_in_netlist(g_0));
            }
            {
                // Gate is not part of the netlist
                auto nl = test_utils::create_empty_netlist();
                auto g_0 = nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0");
                nl->delete_gate(g_0);
                // Gate isn't added
                EXPECT_FALSE(nl->is_gate_in_netlist(g_0));
            }
            {
                // Gate is a nullptr
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                EXPECT_FALSE(nl->is_gate_in_netlist(nullptr));
            }
        TEST_END
    }

    /**
     * Testing the function get_gate_by_id
     *
     * Functions: get_gate_by_id
     */
    TEST_F(NetlistTest, check_get_gate_by_id) {
        TEST_START
            {// Get (existing) Gate with id 3
                auto nl = test_utils::create_empty_netlist();
                auto g_0 = nl->create_gate(MIN_GATE_ID + 3, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0");
                EXPECT_EQ(nl->get_gate_by_id(MIN_GATE_ID + 3), g_0);
            }
            {
                // Get not existing Gate
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                // Gate isn't added
                EXPECT_EQ(nl->get_gate_by_id(MIN_GATE_ID + 3), nullptr);
            }
        TEST_END
    }

    /**
     * Testing the function get_gates by using a namefilter
     *
     * Functions: get_gates
     */
    TEST_F(NetlistTest, check_get_gates_by_name) {
        TEST_START
            {// Get an existing Gate of the example netlist by its name
                auto nl = test_utils::create_example_netlist();
                Gate* g_0 = nl->get_gate_by_id(MIN_GATE_ID + 0);
                Gate* g_5 = nl->get_gate_by_id(MIN_GATE_ID + 5);
                EXPECT_EQ(nl->get_gates(test_utils::gate_name_filter("gate_0")),
                          std::vector<Gate*>({g_0}));
                EXPECT_EQ(nl->get_gates(test_utils::gate_name_filter("gate_5")),
                          std::vector<Gate*>({g_5}));
            }
            {
                // Call with an non existing Gate name
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                EXPECT_EQ(nl->get_gates(test_utils::gate_name_filter("not_existing_gate")),
                          std::vector<Gate*>());
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
    TEST_F(NetlistTest, check_get_gates) {
        TEST_START
            {// Get all gates of the example netlist
                auto nl = test_utils::create_example_netlist();
                // The expected result
                std::vector<Gate*> ex_gates;
                for (int id = 0; id <= 8; id++) {
                    ex_gates.push_back(nl->get_gate_by_id(MIN_GATE_ID + id));
                }

                EXPECT_TRUE(test_utils::vectors_have_same_content(nl->get_gates(), ex_gates));
            }
            {
                // Get all INV gates of the example netlist
                auto nl = test_utils::create_example_netlist();
                // The expected result
                std::vector<Gate*>
                    ex_gates = {nl->get_gate_by_id(MIN_GATE_ID + 3), nl->get_gate_by_id(MIN_GATE_ID + 4)};

                EXPECT_TRUE(test_utils::vectors_have_same_content(nl->get_gates(test_utils::gate_type_filter("gate_1_to_1")), ex_gates));
                EXPECT_TRUE(test_utils::vectors_have_same_content(nl->get_gates(test_utils::gate_type_filter("gate_1_to_1")), ex_gates));
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
    TEST_F(NetlistTest, check_mark_vcc_gate) {
        TEST_START
            {// Add a global vcc Gate which wasn't added to the netlist before and unmark it after
                auto nl = test_utils::create_empty_netlist();

                auto vcc_gate = nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("vcc"), "gate_vcc");
                bool suc_mark = nl->mark_vcc_gate(vcc_gate);
                EXPECT_TRUE(suc_mark);
                EXPECT_TRUE(nl->is_vcc_gate(vcc_gate));
                EXPECT_EQ(nl->get_vcc_gates(), std::vector<Gate*>({vcc_gate}));

                bool suc_unmark = nl->unmark_vcc_gate(vcc_gate);
                EXPECT_TRUE(suc_unmark);
                EXPECT_FALSE(nl->is_vcc_gate(vcc_gate));
            }

            {
                // Add a global gnd Gate which which wasn't added to the netlist before and unmark it after
                auto nl = test_utils::create_empty_netlist();

                auto gnd_gate = nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gnd"), "gate_gnd");
                bool suc_mark = nl->mark_gnd_gate(gnd_gate);
                EXPECT_TRUE(suc_mark);
                EXPECT_TRUE(nl->is_gnd_gate(gnd_gate));
                EXPECT_EQ(nl->get_gnd_gates(), std::vector<Gate*>({gnd_gate}));

                bool suc_unmark = nl->unmark_gnd_gate(gnd_gate);
                EXPECT_TRUE(suc_unmark);
                EXPECT_FALSE(nl->is_gnd_gate(gnd_gate));
            }
            {
                // Add the same global vcc Gate twice
                auto nl = test_utils::create_empty_netlist();

                auto vcc_gate = nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("vcc"), "gate_vcc");
                nl->mark_vcc_gate(vcc_gate);
                bool suc = nl->mark_vcc_gate(vcc_gate);
                EXPECT_TRUE(suc);
                EXPECT_TRUE(nl->is_vcc_gate(vcc_gate));
                EXPECT_EQ(nl->get_vcc_gates(), std::vector<Gate*>({vcc_gate}));
            }
            {
                // Add the same global gnd Gate twice
                auto nl = test_utils::create_empty_netlist();

                auto gnd_gate = nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gnd"), "gate_gnd");
                nl->mark_gnd_gate(gnd_gate);
                bool suc = nl->mark_gnd_gate(gnd_gate);
                EXPECT_TRUE(suc);
                EXPECT_TRUE(nl->is_gnd_gate(gnd_gate));
                EXPECT_EQ(nl->get_gnd_gates(), std::vector<Gate*>({gnd_gate}));
            }

            // NEGATIVE
            {
                // Mark a global_gnd Gate, which is not part of the netlist (part of another netlist)
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                auto nl_other = test_utils::create_empty_netlist();
                Gate* gnd_gate = nl_other->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gnd"), "gate_0");

                bool suc = nl->mark_gnd_gate(gnd_gate);

                EXPECT_FALSE(suc);
            }
            {
                // Mark a global_vcc Gate, which is not part of the netlist (part of another netlist)
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                auto nl_other = test_utils::create_empty_netlist();
                Gate* vcc_gate = nl_other->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("vcc"), "gate_0");

                bool suc = nl->mark_vcc_gate(vcc_gate);

                EXPECT_FALSE(suc);
            }
            {
                // Unmark a global_gnd Gate, which is not part of the netlist (part of another netlist)
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                auto nl_other = test_utils::create_empty_netlist();
                Gate* gnd_gate = nl_other->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gnd"), "gate_0");
                nl_other->mark_gnd_gate(gnd_gate);

                nl->unmark_gnd_gate(gnd_gate);

                EXPECT_TRUE(nl_other->is_gnd_gate(gnd_gate));
            }
            {
                // Unmark a global_vcc Gate, which is not part of the netlist (part of another netlist)
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                auto nl_other = test_utils::create_empty_netlist();
                Gate* vcc_gate = nl_other->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("vcc"), "gate_0");
                nl_other->mark_vcc_gate(vcc_gate);

                nl->unmark_vcc_gate(vcc_gate);

                EXPECT_TRUE(nl_other->is_vcc_gate(vcc_gate));
            }

        TEST_END
    }

    /**
     * Testing the get_unique_net_id function by creating a netlist with nets of the id
     * 0,1,2,4. The new Net id shouldn't be one of them.
     *
     * Functions: get_unique_net_id
     */
    TEST_F(NetlistTest, check_get_unique_net_id) {
        TEST_START
            {// Create an empty netlist with some nets and get a unique id
                auto nl = test_utils::create_empty_netlist();
                Net* n_0 = nl->create_net(MIN_NET_ID + 0, "net_0");
                Net* n_1 = nl->create_net(MIN_NET_ID + 1, "net_1");
                Net* n_3 = nl->create_net(MIN_NET_ID + 3, "net_3");
                std::set<u32> used_ids = {MIN_NET_ID + 0, MIN_NET_ID + 1, MIN_NET_ID + 3};

                u32 new_net_id = nl->get_unique_net_id();
                EXPECT_TRUE(used_ids.find(new_net_id) == used_ids.end());
            }
            {
                // Create an empty netlist with some nets, delete a Net, and get the id
                // (used to test the free_net ids logic)
                auto nl = test_utils::create_empty_netlist();
                Net* n_0 = nl->create_net(MIN_NET_ID + 0, "net_0");
                Net* n_1 = nl->create_net(MIN_NET_ID + 1, "net_1");
                Net* n_2 = nl->create_net(MIN_NET_ID + 2, "net_2");

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
    TEST_F(NetlistTest, check_get_num_of_nets) {
        TEST_START
            // Create the example netlist (has 5 nets)
            auto nl = test_utils::create_example_netlist();
            EXPECT_EQ(nl->get_nets().size(), (size_t) 5);
        TEST_END
    }

    /**
     * Testing addition of nets (calls the function create_net in NetlistInternalManager).
     * To validate success the function is_net_in_netlist is used.
     *
     * Functions: create_net
     */
    TEST_F(NetlistTest, check_add_net) {
        TEST_START
            {// Add a Net the normal way (unrouted)
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(MIN_NET_ID + 0, "net_0");
                EXPECT_NE(net_0, nullptr);
                EXPECT_TRUE(nl->is_net_in_netlist(net_0));
            }
            {
                // Add a Net the normal way by passing no id
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net("net_0");
                EXPECT_NE(net_0, nullptr);
                EXPECT_TRUE(nl->is_net_in_netlist(net_0));
            }
            {
                // Add a Net, remove it, and add a Net with the same id (used to test the free_net_ids logic)
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(MIN_NET_ID + 0, "net_0");
                nl->delete_net(net_0);
                Net* net_0_other = nl->create_net(MIN_NET_ID + 0, "net_0_other");
                EXPECT_NE(net_0_other, nullptr);
                EXPECT_TRUE(nl->is_net_in_netlist(net_0_other));
            }
            // NEGATIVE
            {
                // Create a Net with an invalid id
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Net* net_invalid = nl->create_net(test_utils::INVALID_NET_ID, "net_invalid");
                EXPECT_TRUE(net_invalid == nullptr);
            }
            {
                // Create a Net with an already used id
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(MIN_NET_ID + 0, "net_0");
                Net* net_0_other = nl->create_net(MIN_NET_ID + 0, "net_0_other");
                EXPECT_TRUE(net_0_other == nullptr);
            }
            {
                // Create a Net with an invalid name (empty string)
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(MIN_NET_ID + 0, "");
                EXPECT_TRUE(net_0 == nullptr);
            }
        TEST_END
    }

    /**
     * Testing deletion of nets (calls the function delete_net in NetlistInternalManager)
     *
     * Functions: delete_net
     */
    TEST_F(NetlistTest, check_delete_net) {
        TEST_START
            // POSITIVE
            {// Add and delete an unrouted Net in a normal way
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(MIN_NET_ID + 0, "net_0");
                bool suc = nl->delete_net(net_0);
                EXPECT_TRUE(suc);
                EXPECT_FALSE(nl->is_net_in_netlist(net_0));
            }
            {
                // Delete a Net, which isn't unrouted by using the example netlist
                auto nl = test_utils::create_example_netlist();
                Net* net_045 = nl->get_net_by_id(MIN_NET_ID + 045);
                bool suc = nl->delete_net(net_045);

                EXPECT_TRUE(suc);
                EXPECT_FALSE(nl->is_net_in_netlist(net_045));
                // Check if the netlist was updated correctly
                EXPECT_TRUE(nl->get_gate_by_id(MIN_GATE_ID + 0)->get_successors().empty());
                EXPECT_TRUE(nl->get_gate_by_id(MIN_GATE_ID + 4)->get_predecessors().empty());
                EXPECT_TRUE(nl->get_gate_by_id(MIN_GATE_ID + 5)->get_predecessors().empty());
            }
            {
                // Delete a global input Net
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(MIN_NET_ID + 0, "net_0");
                nl->mark_global_input_net(net_0);
                bool suc = nl->delete_net(net_0);

                EXPECT_TRUE(suc);
                EXPECT_FALSE(nl->is_net_in_netlist(net_0));
                EXPECT_FALSE(nl->is_global_input_net(net_0));
            }
            {
                // Delete a global output Net
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(MIN_NET_ID + 0, "net_0");
                nl->mark_global_output_net(net_0);
                bool suc = nl->delete_net(net_0);

                EXPECT_TRUE(suc);
                EXPECT_FALSE(nl->is_net_in_netlist(net_0));
                EXPECT_FALSE(nl->is_global_output_net(net_0));
            }
            // NEGATIVE
            {
                // Try to delete a nullptr
                // NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                bool suc = nl->delete_net(nullptr);

                EXPECT_FALSE(suc);
                EXPECT_FALSE(nl->is_net_in_netlist(nullptr));
            }
            {
                // Try to delete a Net which is not part of the netlist
                // NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(MIN_NET_ID + 0, "net_0");
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
    TEST_F(NetlistTest, check_is_net_in_netlist) {
        TEST_START
            {// Net is part of the netlist
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(MIN_NET_ID + 1, "net_0");

                EXPECT_TRUE(nl->is_net_in_netlist(net_0));
            }

            {
                // Net is a nullptr
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                EXPECT_FALSE(nl->is_net_in_netlist(nullptr));
            }
        TEST_END
    }

    /**
     * Testing the get_nets function using the example netlist (see above).
     *
     * Functions: get_nets
     */
    TEST_F(NetlistTest, check_get_nets) {
        TEST_START
            {// Get all nets of the example netlist
                auto nl = test_utils::create_example_netlist();
                // The expected result
                std::vector<Net*> ex_nets;
                for (int id : {(int) MIN_NET_ID + 13, (int) MIN_NET_ID + 30, (int) MIN_NET_ID + 20,
                                             (int) MIN_NET_ID + 045, (int) MIN_NET_ID + 78}) {
                    ex_nets.push_back(nl->get_net_by_id(id));
                }

                EXPECT_TRUE(test_utils::vectors_have_same_content(nl->get_nets(), ex_nets));
            }
        TEST_END
    }

    /**
     * Testing the function get_net_by_id
     *
     * Functions: get_net_by_id
     */
    TEST_F(NetlistTest, check_get_net_by_id) {
        TEST_START
            {// Net exists (ID: 123)
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(MIN_NET_ID + 123, "net_0");

                EXPECT_EQ(nl->get_net_by_id(MIN_NET_ID + 123), net_0);
            }
            {
                // Net doesn't exists (ID: 123)
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                EXPECT_EQ(nl->get_net_by_id(MIN_NET_ID + 123), nullptr);
            }
        TEST_END
    }

    /**
     * Testing the function get_nets_by_name
     *
     * Functions: get_nets_by_name
     */
    TEST_F(NetlistTest, check_get_nets_by_name) {
        TEST_START
            {// Get an existing Net by its name
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(MIN_NET_ID + 0, "net_name");
                Net* net_1 = nl->create_net(MIN_NET_ID + 1, "other_net_name");

                EXPECT_TRUE(test_utils::vectors_have_same_content(nl->get_nets(), std::vector<Net*>({net_0, net_1})));
                EXPECT_EQ(nl->get_nets(test_utils::net_name_filter("net_name")), std::vector<Net*>({net_0}));
            }
            {
                // Call with an non existing Net name
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                EXPECT_EQ(nl->get_nets(test_utils::net_name_filter("not_existing_net")), std::vector<Net*>());
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
    TEST_F(NetlistTest, check_add_global_net) {
        TEST_START
            // Mark functions (normal usage)
            {// Add a global input Net which isn't part of the netlist yet
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(MIN_NET_ID + 0, "net_0");
                bool suc = nl->mark_global_input_net(net_0);

                EXPECT_TRUE(suc);
                EXPECT_TRUE(nl->is_global_input_net(net_0));
            }
            {
                // Add a global output Net which isn't part of the netlist yet
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(MIN_NET_ID + 0, "net_0");
                bool suc = nl->mark_global_output_net(net_0);

                EXPECT_TRUE(suc);
                EXPECT_TRUE(nl->is_global_output_net(net_0));
            }
            // Unmark functions (normal usage)
            {
                // Mark and unmark a global input Net
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(MIN_NET_ID + 0, "net_0");
                bool suc_mark = nl->mark_global_input_net(net_0);

                ASSERT_TRUE(suc_mark);
                ASSERT_TRUE(nl->is_global_input_net(net_0));

                bool suc_unmark = nl->unmark_global_input_net(net_0);
                ASSERT_TRUE(suc_unmark);
                ASSERT_FALSE(nl->is_global_input_net(net_0));
            }
            {
                // Mark and unmark a global output Net
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(MIN_NET_ID + 0, "net_0");
                bool suc_mark = nl->mark_global_output_net(net_0);

                ASSERT_TRUE(suc_mark);
                ASSERT_TRUE(nl->is_global_output_net(net_0));

                bool suc_unmark = nl->unmark_global_output_net(net_0);
                ASSERT_TRUE(suc_unmark);
                ASSERT_FALSE(nl->is_global_output_net(net_0));
            }
            {
                // Add a global input Net which was already added (as normal Net)
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(MIN_NET_ID + 0, "net_0");
                bool suc = nl->mark_global_input_net(net_0);

                EXPECT_TRUE(suc);
                EXPECT_TRUE(nl->is_global_input_net(net_0));
            }
            {
                // Add a global output Net which was already added (as normal Net)
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(MIN_NET_ID + 0, "net_0");
                bool suc = nl->mark_global_output_net(net_0);

                EXPECT_TRUE(suc);
                EXPECT_TRUE(nl->is_global_output_net(net_0));
            }
            {
                // Add the same global input Net twice
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(MIN_NET_ID + 0, "net_0");
                nl->mark_global_input_net(net_0);
                bool suc = nl->mark_global_input_net(net_0);

                EXPECT_TRUE(suc);
                EXPECT_TRUE(nl->is_global_input_net(net_0));
            }
            {
                // Add the same global output Net twice
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(MIN_NET_ID + 0, "net_0");
                nl->mark_global_output_net(net_0);
                bool suc = nl->mark_global_output_net(net_0);

                EXPECT_TRUE(suc);
                EXPECT_TRUE(nl->is_global_output_net(net_0));
            }
            // NEGATIVE
            // Mark functions
            {
                // Mark an input-Net, which is not part of the netlist (part of another netlist)
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                auto nl_other = test_utils::create_empty_netlist();
                Net* net_0 = nl_other->create_net(MIN_NET_ID + 0, "net_0");

                bool suc = nl->mark_global_input_net(net_0);

                EXPECT_FALSE(suc);
            }
            {
                // Mark an output-Net, which is not part of the netlist (part of another netlist)
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                auto nl_other = test_utils::create_empty_netlist();
                Net* net_0 = nl_other->create_net(MIN_NET_ID + 0, "net_0");

                bool suc = nl->mark_global_output_net(net_0);

                EXPECT_FALSE(suc);
                EXPECT_FALSE(nl->is_global_output_net(net_0));
            }
            // Unmark functions
            {
                // Unmark a global input-Net, which isn't marked as such
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(MIN_NET_ID + 0, "net_0");

                bool suc = nl->unmark_global_input_net(net_0);

                EXPECT_FALSE(suc);
            }
            {
                // Unmark a global output-Net, which isn't marked as such
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(MIN_NET_ID + 0, "net_0");

                bool suc = nl->unmark_global_output_net(net_0);

                EXPECT_FALSE(suc);
            }
            {
                // Unmark a global input-Net, which is part of another netlist
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                auto nl_other = test_utils::create_empty_netlist();
                Net* net_0 = nl_other->create_net(MIN_NET_ID + 0, "net_0");
                nl_other->mark_global_input_net(net_0);

                bool suc = nl->unmark_global_input_net(net_0);

                EXPECT_FALSE(suc);
                EXPECT_TRUE(nl_other->is_global_input_net(net_0));
            }
            {
                // Unmark a global output-Net, which is part of another netlist
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                auto nl_other = test_utils::create_empty_netlist();
                Net* net_0 = nl_other->create_net(MIN_NET_ID + 0, "net_0");
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
    TEST_F(NetlistTest, check_is_global_net) {
        TEST_START
            {// The Net is a global input Net
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(MIN_NET_ID + 0, "net_0");
                nl->mark_global_input_net(net_0);

                EXPECT_TRUE(nl->is_global_input_net(net_0));
                EXPECT_EQ(nl->get_global_input_nets(), std::vector<Net*>({net_0}));
            }
            {
                // The Net is a global output Net
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(MIN_NET_ID + 0, "net_0");
                nl->mark_global_output_net(net_0);

                EXPECT_TRUE(nl->is_global_output_net(net_0));
                EXPECT_EQ(nl->get_global_output_nets(), std::vector<Net*>({net_0}));
            }
            {
                // The Net isn't a global input Net
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(MIN_NET_ID + 0, "net_0");

                EXPECT_FALSE(nl->is_global_input_net(net_0));
                EXPECT_TRUE(nl->get_global_input_nets().empty());
            }
            {
                // The Net isn't a global output Net
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(MIN_NET_ID + 0, "net_0");

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
    TEST_F(NetlistTest, check_get_top_module) {
        TEST_START
            {// Testing the access on the topmodule
                auto nl = test_utils::create_empty_netlist();
                Module* tm = nl->get_top_module();
                ASSERT_NE(tm, nullptr);
                EXPECT_EQ(tm->get_parent_module(), nullptr);
                // If only this test fails you may update the TOP_MODULE_ID constant
                EXPECT_EQ(tm->get_id(), test_utils::TOP_MODULE_ID);
            }
        TEST_END
    }

    /**
    * Testing the creation of a module
    *
    * Functions: create_module
    */
    TEST_F(NetlistTest, check_create_module) {
        TEST_START
            // Positive
            {// Create a module
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module(MIN_MODULE_ID + 0, "module_0", nl->get_top_module());
                EXPECT_TRUE(nl->is_module_in_netlist(m_0));
            }
            {
                // Create a module without passing an id
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module("module_0", nl->get_top_module());
                EXPECT_TRUE(nl->is_module_in_netlist(m_0));
            }
            {
                // Add a module, remove it and add a module with the same id (to test the free_module_id logic)
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module(MIN_MODULE_ID + 0, "module_0", nl->get_top_module());
                nl->delete_module(m_0);
                Module*
                    m_0_other = nl->create_module(MIN_MODULE_ID + 0, "module_0_other", nl->get_top_module());
                EXPECT_TRUE(nl->is_module_in_netlist(m_0_other));
            }

            // Negative
            {
                // Create a module with an invalid id
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Module*
                    m_0 = nl->create_module(test_utils::INVALID_MODULE_ID, "module_0", nl->get_top_module());
                EXPECT_EQ(m_0, nullptr);
            }
            {
                // Create a module with an id, which is already used
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module(MIN_MODULE_ID + 0, "module_0", nl->get_top_module());
                Module*
                    m_0_other = nl->create_module(MIN_MODULE_ID + 0, "module_0_other", nl->get_top_module());
                EXPECT_EQ(m_0_other, nullptr);
            }
            {
                // Create a module with the id, used of the top module
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Module*
                    m_0 = nl->create_module(test_utils::TOP_MODULE_ID, "module_0", nl->get_top_module());
                EXPECT_EQ(m_0, nullptr);
                EXPECT_EQ(nl->get_module_by_id(test_utils::TOP_MODULE_ID), nl->get_top_module());
            }
            {
                // Create a module with an invalid name (empty string)
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module(MIN_MODULE_ID + 0, "", nl->get_top_module());
                EXPECT_EQ(m_0, nullptr);
            }
            {
                // Create a module with no parent-module
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module(MIN_MODULE_ID + 0, "module_0", nullptr);
                EXPECT_EQ(m_0, nullptr);
            }
            {
                // Create a module where the parrent module is part of ANOTHER netlist
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                auto nl_other = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module(MIN_MODULE_ID+0,"module_0", nl_other->get_top_module());
                EXPECT_EQ(m_0, nullptr);
            }

        TEST_END
    }

    /**
     * Testing the get_module_by_id function
     *
     * Functions: get_module_by_id
     */
    TEST_F(NetlistTest, check_get_module_by_id) {
        TEST_START
            // Positive
            {// Testing the access on a module by its name
                auto nl = test_utils::create_empty_netlist();
                Module*
                    m_123 = nl->create_module(MIN_MODULE_ID + 123, "module_123", nl->get_top_module());
                if (m_123 != nullptr) {
                    Module* m_123_by_id = nl->get_module_by_id(MIN_MODULE_ID + 123);
                    EXPECT_EQ(m_123_by_id, m_123);
                }
            }
            {
                // Testing the access on the topmodule by its id
                auto nl = test_utils::create_empty_netlist();
                Module* tm = nl->get_top_module();
                if (tm != nullptr) {
                    Module* tm_by_id = nl->get_module_by_id(tm->get_id());
                    EXPECT_EQ(tm, tm_by_id);
                }
            }
            // Negative
            {
                // The passed id is not taken
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module(MIN_MODULE_ID + 0, "module_0", nl->get_top_module());

                EXPECT_EQ(nl->get_module_by_id(MIN_MODULE_ID + 123), nullptr);
            }
        TEST_END
    }

    /**
    * Testing the deletion of modules from the netlist. Verification of success by is_module_in_netlist
    *
    * Functions: delete_module, is_module_in_netlist
    */
    TEST_F(NetlistTest, check_delete_module) {
        TEST_START
            // Positive
            {// Add a module and delete it after
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module(MIN_MODULE_ID + 0, "module_0", nl->get_top_module());
                nl->delete_module(m_0);
                EXPECT_FALSE(nl->is_module_in_netlist(m_0));
            }
            {
                // Remove modules which own submodules
                auto nl = test_utils::create_empty_netlist();
                Module* parent = nl->create_module(MIN_MODULE_ID + 0, "module_0", nl->get_top_module());
                Module* test_module = nl->create_module(MIN_MODULE_ID + 1, "module_1", parent);
                Module* child = nl->create_module(MIN_MODULE_ID + 2, "module_2", test_module);

                // Add a Net and a Gate to the test_module
                auto gate_0 = nl->create_gate(test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0");
                Net* net_0 = nl->create_net("net_0");
                test_module->assign_gate(gate_0);
                //test_module->assign_net(net_0);

                nl->delete_module(test_module);

                EXPECT_FALSE(nl->is_module_in_netlist(test_module));
                EXPECT_TRUE(parent->contains_gate(gate_0));
                //EXPECT_TRUE(parent->contains_net(net_0));
                auto sms = parent->get_submodules();
                EXPECT_TRUE(std::find(sms.begin(), sms.end(), child) != parent->get_submodules().end());
            }

            // NEGATIVE
            {
                // Try to delete the top module
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Module* tm = nl->get_top_module();
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
    TEST_F(NetlistTest, check_is_module_in_netlist) {
        TEST_START
            // Positive
            {
                // Add a module and check if it is in the netlist
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module(MIN_MODULE_ID + 0, "module_0", nl->get_top_module());
                EXPECT_TRUE(nl->is_module_in_netlist(m_0));
            }
            {
                // Create a module, delete it and create a new module with the same id and check if the !old_one! is in the netlist
                auto nl = test_utils::create_empty_netlist();
                Module* m_0_old = nl->create_module(MIN_MODULE_ID+0, "module_0_old", nl->get_top_module());
                nl->delete_module(m_0_old); // Adress of m_0_old is now freed
                Module* m_0_other = nl->create_module(MIN_MODULE_ID+0, "module_0_other", nl->get_top_module());
                EXPECT_TRUE(m_0_old == m_0_other || !nl->is_module_in_netlist(m_0_old));
            }
            // Negative
            {
                // Pass a nullptr
                auto nl = test_utils::create_empty_netlist();
                EXPECT_FALSE(nl->is_module_in_netlist(nullptr));
            }
        TEST_END
    }

    /**
    * Testing the function get_unique_module_id
    *
    * Functions: get_unique_module_id
    */
    TEST_F(NetlistTest, check_get_unique_module_id) {
        TEST_START
            // Positive
            {
                // Create some modules and get unique module ids
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module(MIN_MODULE_ID + 0, "module_0", nl->get_top_module());
                Module* m_1 = nl->create_module(MIN_MODULE_ID + 1, "module_1", nl->get_top_module());
                Module* m_3 = nl->create_module(MIN_MODULE_ID + 3, "module_3", nl->get_top_module());
                std::set<u32>
                    used_ids = {test_utils::TOP_MODULE_ID, MIN_MODULE_ID + 0, MIN_MODULE_ID + 1, MIN_MODULE_ID + 3};

                u32 new_module_id_1 = nl->get_unique_module_id();
                ASSERT_TRUE(used_ids.find(new_module_id_1) == used_ids.end());

                Module* m_new = nl->create_module(new_module_id_1, "module_new", nl->get_top_module());
                used_ids.insert(new_module_id_1);

                u32 new_module_id_2 = nl->get_unique_module_id();
                EXPECT_TRUE(used_ids.find(new_module_id_2) == used_ids.end());
            }
            {
                // Create some modules, delete some and get a unique module id (for testing the free_module_ids logic)
                auto nl = test_utils::create_empty_netlist();
                Module* m_0 = nl->create_module(MIN_MODULE_ID + 0, "module_0", nl->get_top_module());
                Module* m_1 = nl->create_module(MIN_MODULE_ID + 1, "module_1", nl->get_top_module());
                Module* m_2 = nl->create_module(MIN_MODULE_ID + 2, "module_2", nl->get_top_module());
                Module* m_3 = nl->create_module(MIN_MODULE_ID + 3, "module_3", nl->get_top_module());

                nl->delete_module(m_0);
                nl->delete_module(m_2);
                std::set<u32> used_ids = {test_utils::TOP_MODULE_ID, MIN_MODULE_ID + 1, MIN_MODULE_ID + 3};

                u32 new_module_id_1 = nl->get_unique_module_id();
                ASSERT_TRUE(used_ids.find(new_module_id_1) == used_ids.end());

                Module* m_new = nl->create_module(new_module_id_1, "module_new", nl->get_top_module());
                used_ids.insert(new_module_id_1);

                u32 new_module_id_2 = nl->get_unique_module_id();
                EXPECT_TRUE(used_ids.find(new_module_id_2) == used_ids.end());
            }
        TEST_END
    }

    /*************************************
     * Event System
     *************************************/

    /**
     * Testing the triggering of events.
     */
    TEST_F(NetlistTest, check_events) {
        TEST_START
            const u32 NO_DATA = 0xFFFFFFFF;
            u32 nl_old_id = MIN_NETLIST_ID + 123;

            std::unique_ptr<Netlist> test_nl = test_utils::create_example_netlist(nl_old_id);
            Gate* gnd_gate = test_nl->get_gate_by_id(MIN_GATE_ID + 1);
            Gate* vcc_gate = test_nl->get_gate_by_id(MIN_GATE_ID + 2);
            Net* test_net = test_nl->get_net_by_id(MIN_NET_ID + 13);

            // Small functions that should trigger certain events exactly once (these operations are executed in this order)
            std::function<void(Netlist*)> trigger_id_changed = [](Netlist* nl){nl->set_id(456);};
            std::function<void(Netlist*)> trigger_input_filename_changed = [](Netlist* nl){nl->set_input_filename("other_filename");};
            std::function<void(Netlist*)> trigger_design_name_changed = [](Netlist* nl){nl->set_design_name("new_design_name");};
            std::function<void(Netlist*)> trigger_device_name_changed = [](Netlist* nl){nl->set_device_name("new_device_name");};
            std::function<void(Netlist*)> trigger_marked_global_vcc = [=](Netlist* nl){nl->mark_vcc_gate(vcc_gate);};
            std::function<void(Netlist*)> trigger_marked_global_gnd = [=](Netlist* nl){nl->mark_gnd_gate(gnd_gate);};
            std::function<void(Netlist*)> trigger_unmarked_global_vcc = [=](Netlist* nl){nl->unmark_vcc_gate(vcc_gate);};
            std::function<void(Netlist*)> trigger_unmarked_global_gnd = [=](Netlist* nl){nl->unmark_gnd_gate(gnd_gate);};
            std::function<void(Netlist*)> trigger_marked_global_input = [=](Netlist* nl){nl->mark_global_input_net(test_net);};
            std::function<void(Netlist*)> trigger_marked_global_output = [=](Netlist* nl){nl->mark_global_output_net(test_net);};
            std::function<void(Netlist*)> trigger_marked_global_inout = [=](Netlist* nl){return;}; // ISSUE: legacy?
            std::function<void(Netlist*)> trigger_unmarked_global_input = [=](Netlist* nl){nl->unmark_global_input_net(test_net);};
            std::function<void(Netlist*)> trigger_unmarked_global_output = [=](Netlist* nl){nl->unmark_global_output_net(test_net);};
            std::function<void(Netlist*)> trigger_unmarked_global_inout = [=](Netlist* nl){return;}; // ISSUE: legacy?

            // The events that are tested
            std::vector<netlist_event_handler::event> event_type = {
                netlist_event_handler::event::id_changed, netlist_event_handler::event::input_filename_changed,
                netlist_event_handler::event::design_name_changed ,netlist_event_handler::event::device_name_changed,
                netlist_event_handler::event::marked_global_vcc, netlist_event_handler::event::marked_global_gnd,
                netlist_event_handler::event::unmarked_global_vcc, netlist_event_handler::event::unmarked_global_gnd,
                netlist_event_handler::event::marked_global_input, netlist_event_handler::event::marked_global_output,
                /*netlist_event_handler::event::marked_global_inout,*/ netlist_event_handler::event::unmarked_global_input,
                netlist_event_handler::event::unmarked_global_output/*, netlist_event_handler::event::unmarked_global_inout*/};

            // A list of the functions that will trigger its associated event exactly once
            std::vector<std::function<void(Netlist*)>> trigger_event = {trigger_id_changed, trigger_input_filename_changed,
                trigger_design_name_changed, trigger_device_name_changed, trigger_marked_global_vcc, trigger_marked_global_gnd,
                trigger_unmarked_global_vcc, trigger_unmarked_global_gnd, trigger_marked_global_input, trigger_marked_global_output,
                /*trigger_marked_global_inout,*/trigger_unmarked_global_input, trigger_unmarked_global_output/*, trigger_unmarked_global_inout*/ };

            // The parameters of the events that are expected
            std::vector<std::tuple<netlist_event_handler::event, Netlist*, u32>> expected_parameter = {
                std::make_tuple(netlist_event_handler::event::id_changed, test_nl.get(), nl_old_id),
                std::make_tuple(netlist_event_handler::input_filename_changed, test_nl.get(), NO_DATA),
                std::make_tuple(netlist_event_handler::design_name_changed, test_nl.get(), NO_DATA),
                std::make_tuple(netlist_event_handler::device_name_changed, test_nl.get(), NO_DATA),
                std::make_tuple(netlist_event_handler::marked_global_vcc, test_nl.get(), vcc_gate->get_id()),
                std::make_tuple(netlist_event_handler::marked_global_gnd, test_nl.get(), gnd_gate->get_id()),
                std::make_tuple(netlist_event_handler::unmarked_global_vcc, test_nl.get(), vcc_gate->get_id()),
                std::make_tuple(netlist_event_handler::unmarked_global_gnd, test_nl.get(), gnd_gate->get_id()),
                std::make_tuple(netlist_event_handler::marked_global_input, test_nl.get(), test_net->get_id()),
                std::make_tuple(netlist_event_handler::marked_global_output, test_nl.get(), test_net->get_id()),
                //std::make_tuple(netlist_event_handler::marked_global_inout, test_nl.get(), test_net->get_id()),
                std::make_tuple(netlist_event_handler::unmarked_global_input, test_nl.get(), test_net->get_id()),
                std::make_tuple(netlist_event_handler::unmarked_global_output, test_nl.get(), test_net->get_id()),
                //std::make_tuple(netlist_event_handler::unmarked_global_inout, test_nl.get(), test_net->get_id())
            };

            // Check all events in a for-loop
            for(u32 event_idx = 0; event_idx < event_type.size(); event_idx++)
            {
                // Create the listener for the tested event
                test_utils::EventListener<void, netlist_event_handler::event, Netlist*, u32> listener;
                std::function<void(netlist_event_handler::event, Netlist*, u32)> cb = listener.get_conditional_callback(
                    [=](netlist_event_handler::event ev, Netlist* nl, u32 id){return ev == event_type[event_idx];}
                );
                std::string cb_name = "nl_event_callback_" + std::to_string((u32)event_type[event_idx]);
                // Register a callback of the listener
                netlist_event_handler::register_callback(cb_name, cb);

                // Trigger the event
                trigger_event[event_idx](test_nl.get());

                EXPECT_EQ(listener.get_event_count(), 1);
                EXPECT_EQ(listener.get_last_parameters(), expected_parameter[event_idx]);

                // Unregister the callback
                netlist_event_handler::unregister_callback(cb_name);
            }
        TEST_END
    }

} //namespace hal
