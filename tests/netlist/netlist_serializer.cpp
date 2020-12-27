#include "hal_core/netlist/persistent/netlist_serializer.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_library_parser/gate_library_parser_manager.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "netlist_test_utils.h"

#include <experimental/filesystem>

namespace hal {
    using test_utils::MIN_GATE_ID;
    using test_utils::MIN_NET_ID;
    using test_utils::MIN_MODULE_ID;
    using test_utils::MIN_GROUPING_ID;

    class NetlistSerializerTest : public ::testing::Test {
    protected:
        std::filesystem::path m_g_lib_path;
        GateLibrary *m_gl;

        std::string m_min_gl_content = "library (MIN_TEST_GATE_LIBRARY_FOR_NETLIST_SERIALIZER_TESTS) {\n"
                                       "    define(cell);\n"
                                       "    cell(gate_1_to_1) {\n"
                                       "        pin(I) { direction: input; }\n"
                                       "        pin(O) { direction: output; } }\n"
                                       "    cell(gate_2_to_1) {\n"
                                       "        pin(I0) { direction: input; }\n"
                                       "        pin(I1) { direction: input; }\n"
                                       "        pin(O) { direction: output; } }\n"
                                       "    cell(gate_2_to_0) {\n"
                                       "        pin(I0) { direction: input; }\n"
                                       "        pin(I1) { direction: input; } }\n"
                                       "    cell(gnd) {\n"
                                       "        pin(O) { direction: output; function: \"0\"; } }\n"
                                       "    cell(vcc) {\n"
                                       "        pin(O) { direction: output; function: \"1\"; } }\n"
                                       "}";

        virtual void SetUp() {
            NO_COUT_BLOCK;
            test_utils::init_log_channels();
            plugin_manager::load_all_plugins();
            test_utils::create_sandbox_directory();
            m_g_lib_path = test_utils::create_sandbox_file("min_test_gate_lib_for_netlist_serializer_tests.lib", m_min_gl_content);
            m_gl = gate_library_parser_manager::load_file(m_g_lib_path, true);
        }

        virtual void TearDown() {
            NO_COUT_BLOCK;
            test_utils::remove_sandbox_directory();
            plugin_manager::unload_all_plugins();
        }

        // ===== Example Serializer Netlists =====

        /*
        *
        *                 __________________________________________
        *                | test_mod_0           ____________________|
        *                |                     | test_mod_1         |
        *      gnd (1) =-=-= gate_1_to_1 (3) =-=-=                  | .------= gate_1_to_1 (4) =
        *                |                     |  gate_2_to_1 (0) =-=-+
        *      vcc (2) =-=---------------------=-=                  | '------=
        *                |_____________________|____________________|          gate_2_to_1 (5) =
        *                                                                    =
        *
        *     =                       =                    .-------=
        *       gate_2_to_0 (6)         gate_2_to_1 (7) =--'       ...  gate_2_to_1 (8) =
        *     =                       =                            =
        */
        std::unique_ptr<Netlist> create_example_serializer_netlist() {
            auto nl = std::make_unique<Netlist>(m_gl);
            nl->set_id(123);
            nl->set_input_filename("esnl_input_filename");
            nl->set_device_name("esnl_device_name");
            nl->get_top_module()->set_type("top_mod_type");

            // Create the gates

            Gate* gate_0 = nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_2_to_1", m_gl), "gate_0");
            Gate* gate_1 = nl->create_gate(MIN_GATE_ID + 1, test_utils::get_gate_type_by_name("gnd", m_gl), "gate_1");
            Gate* gate_2 = nl->create_gate(MIN_GATE_ID + 2, test_utils::get_gate_type_by_name("vcc", m_gl), "gate_2");
            Gate* gate_3 = nl->create_gate(MIN_GATE_ID + 3, test_utils::get_gate_type_by_name("gate_1_to_1", m_gl), "gate_3");
            Gate* gate_4 = nl->create_gate(MIN_GATE_ID + 4, test_utils::get_gate_type_by_name("gate_1_to_1", m_gl), "gate_4");
            Gate* gate_5 = nl->create_gate(MIN_GATE_ID + 5, test_utils::get_gate_type_by_name("gate_2_to_1", m_gl), "gate_5");
            Gate* gate_6 = nl->create_gate(MIN_GATE_ID + 6, test_utils::get_gate_type_by_name("gate_2_to_0", m_gl), "gate_6");
            Gate* gate_7 = nl->create_gate(MIN_GATE_ID + 7, test_utils::get_gate_type_by_name("gate_2_to_1", m_gl), "gate_7");
            Gate* gate_8 = nl->create_gate(MIN_GATE_ID + 8, test_utils::get_gate_type_by_name("gate_2_to_1", m_gl), "gate_8");

            // Add the nets (net_x_y1_y2... := Net between the Gate with id x and the gates y1,y2,...)
            Net* net_1_3 = nl->create_net(MIN_NET_ID + 13, "net_1_3");
            net_1_3->add_source(gate_1, "O");
            net_1_3->add_destination(gate_3, "I");

            Net* net_3_0 = nl->create_net(MIN_NET_ID + 30, "net_3_0");
            net_3_0->add_source(gate_3, "O");
            net_3_0->add_destination(gate_0, "I0");

            Net* net_2_0 = nl->create_net(MIN_NET_ID + 20, "net_2_0");
            net_2_0->add_source(gate_2, "O");
            net_2_0->add_destination(gate_0, "I1");

            Net* net_0_4_5 = nl->create_net(MIN_NET_ID + 045, "net_0_4_5");
            net_0_4_5->add_source(gate_0, "O");
            net_0_4_5->add_destination(gate_4, "I");
            net_0_4_5->add_destination(gate_5, "I0");

            Net* net_7_8 = nl->create_net(MIN_NET_ID + 78, "net_7_8");
            net_7_8->add_source(gate_7, "O");
            net_7_8->add_destination(gate_8, "I0");

            // Mark some gates as gnd/vcc gates
            nl->mark_gnd_gate(nl->get_gate_by_id(MIN_GATE_ID + 1));
            nl->mark_vcc_gate(nl->get_gate_by_id(MIN_GATE_ID + 2));

            // Mark some nets as global nets
            nl->mark_global_input_net(nl->get_net_by_id(MIN_NET_ID + 13));
            nl->mark_global_output_net(nl->get_net_by_id(MIN_NET_ID + 30));

            // Create the modules
            Module* test_m_0 = nl->create_module(MIN_MODULE_ID + 0, "test_mod_0", nl->get_top_module());
            test_m_0->set_type("test_mod_type_0");
            test_m_0->assign_gate(nl->get_gate_by_id(MIN_GATE_ID + 0));
            test_m_0->assign_gate(nl->get_gate_by_id(MIN_GATE_ID + 3));

            Module* test_m_1 = nl->create_module(MIN_MODULE_ID + 1, "test_mod_1", test_m_0);
            test_m_1->set_type("test_mod_type_1");
            test_m_1->assign_gate(nl->get_gate_by_id(MIN_GATE_ID + 0));

            // Store some data in a Gate, Net and module
            nl->get_gate_by_id(MIN_GATE_ID + 1)->set_data("category_0", "key_0", "data_type", "test_value");
            nl->get_gate_by_id(MIN_GATE_ID + 1)->set_data("category_1", "key_1", "data_type", "test_value_1");
            nl->get_gate_by_id(MIN_GATE_ID + 1)->set_data("category_1", "key_0", "data_type", "test_value_2");
            nl->get_net_by_id(MIN_NET_ID + 13)->set_data("category", "key_2", "data_type", "test_value");
            test_m_0->set_data("category", "key_3", "data_type", "test_value");

            // Set some input/output port names of module 0
            test_m_0->set_input_port_name(net_1_3, "test_m_0_net_1_3_in");
            test_m_0->set_input_port_name(net_2_0, "test_m_0_net_2_0_in");
            test_m_0->set_output_port_name(net_0_4_5, "test_m_0_net_0_4_5_out");

            test_m_1->set_output_port_name(net_0_4_5, "test_m_1_net_0_4_5_out");

            // Store some boolean functions in some gates
            nl->get_gate_by_id(MIN_GATE_ID + 0)->add_boolean_function("O_and",
                BooleanFunction::from_string("I0 & I1", std::vector<std::string>({"I0","I1"})));
            nl->get_gate_by_id(MIN_GATE_ID + 0)->add_boolean_function("O_or",
                                                                      BooleanFunction::from_string("I0 & I1", std::vector<std::string>({"I0","I1"})));
            nl->get_gate_by_id(MIN_GATE_ID + 4)->add_boolean_function("O_not",
                                                                      BooleanFunction::from_string("!I", std::vector<std::string>({"I"})));

            // create the groupings
            Grouping* grouping_0 = nl->create_grouping(MIN_GROUPING_ID + 0, "grouping_0");
            grouping_0->assign_gate(gate_0);
            grouping_0->assign_gate(gate_1);
            grouping_0->assign_gate(gate_2);
            grouping_0->assign_net(net_1_3);
            grouping_0->assign_net(net_2_0);
            grouping_0->assign_module(test_m_0);
            grouping_0->assign_module(test_m_1);

            Grouping* grouping_1 = nl->create_grouping(MIN_GROUPING_ID + 1, "grouping_1");
            grouping_1->assign_gate(gate_3);
            grouping_1->assign_gate(gate_4);
            grouping_1->assign_gate(gate_5);
            grouping_1->assign_net(net_7_8);

            return nl;
        }
    };

    /**
     * Testing the serialization and a followed deserialization of the example
     * netlist.
     *
     * Functions: serialize_netlist, deserialize_netlist
     */
    TEST_F(NetlistSerializerTest, check_serialize_and_deserialize) {
        TEST_START
            {// Serialize and deserialize the example netlist (with some additions) and compare the result with the original netlist
                auto nl = create_example_serializer_netlist();

                // Serialize and deserialize the netlist now
                std::filesystem::path test_hal_file_path = test_utils::create_sandbox_path("test_hal_file.hal");
                bool suc = netlist_serializer::serialize_to_file(nl.get(), test_hal_file_path);

                auto des_nl = netlist_serializer::deserialize_from_file(test_hal_file_path);

                EXPECT_TRUE(suc);

                ASSERT_NE(des_nl, nullptr);

                // -- Check the netlists as a whole
                EXPECT_TRUE(test_utils::netlists_are_equal(nl.get(), des_nl.get()));
            }
            {
                // Test the example netlist against its deserialized version, but flip the module ids
                auto nl = create_example_serializer_netlist();
                // -- Remove the modules
                nl->delete_module(nl->get_module_by_id(MIN_MODULE_ID + 0));
                nl->delete_module(nl->get_module_by_id(MIN_MODULE_ID + 1));
                // -- Add them again with flipped ids
                Module *test_m_0_flipped =
                    nl->create_module(MIN_MODULE_ID + 1, "test_mod_0_flipped", nl->get_top_module());
                test_m_0_flipped->set_type("test_mod_type_0_flipped");
                test_m_0_flipped->assign_gate(nl->get_gate_by_id(MIN_GATE_ID + 0));
                test_m_0_flipped->assign_gate(nl->get_gate_by_id(MIN_GATE_ID + 3));

                Module *test_m_1_flipped = nl->create_module(MIN_MODULE_ID + 0, "test_mod_1_flipped", test_m_0_flipped);
                test_m_1_flipped->set_type("test_mod_type_1_flipped");
                test_m_1_flipped->assign_gate(nl->get_gate_by_id(MIN_GATE_ID + 0));

                test_m_0_flipped->set_data("category", "key_3", "data_type", "test_value");

                // Set some input/output port names of module 0
                test_m_0_flipped
                    ->set_input_port_name(nl->get_net_by_id(MIN_NET_ID + 13), "test_m_0_flipped_net_1_3_in");
                test_m_0_flipped
                    ->set_input_port_name(nl->get_net_by_id(MIN_NET_ID + 20), "test_m_0_flipped_net_2_0_in");
                test_m_0_flipped
                    ->set_output_port_name(nl->get_net_by_id(MIN_NET_ID + 045), "test_m_0_flipped_net_0_4_5_out");

                test_m_1_flipped
                    ->set_output_port_name(nl->get_net_by_id(MIN_NET_ID + 045), "test_m_1_flipped_net_0_4_5_out");

                // Serialize and deserialize the netlist now
                std::filesystem::path test_hal_file_path = test_utils::create_sandbox_path("test_hal_file.hal");
                bool suc = netlist_serializer::serialize_to_file(nl.get(), test_hal_file_path);

                auto des_nl = netlist_serializer::deserialize_from_file(test_hal_file_path);

                // -- Check the netlists as a whole
                EXPECT_TRUE(test_utils::netlists_are_equal(nl.get(), des_nl.get()));
            }
            {
                // Serialize and deserialize an empty netlist and compare the result with the original netlist
                auto nl = std::make_unique<Netlist>(m_gl);

                std::filesystem::path test_hal_file_path = test_utils::create_sandbox_path("test_hal_file.hal");
                bool suc = netlist_serializer::serialize_to_file(nl.get(), test_hal_file_path);
                auto des_nl = netlist_serializer::deserialize_from_file(test_hal_file_path);

                EXPECT_TRUE(suc);
                EXPECT_TRUE(test_utils::netlists_are_equal(nl.get(), des_nl.get()));
            }


        TEST_END
    }

    /**
     * Testing the serialization and deserialization of a netlist with invalid input
     *
     * Functions: serialize_netlist, deserialize_netlist
     */
    TEST_F(NetlistSerializerTest, check_serialize_and_deserialize_negative) {
        TEST_START
            {
                // Serialize a netlist which is a nullptr
                std::filesystem::path test_hal_file_path = test_utils::create_sandbox_path("test_hal_file.hal");
                bool suc = netlist_serializer::serialize_to_file(nullptr, test_hal_file_path);
                EXPECT_FALSE(suc);
            }
            {
                // Serialize a netlist to an invalid path
                NO_COUT_TEST_BLOCK;
                auto nl = create_example_serializer_netlist();
                bool suc = netlist_serializer::serialize_to_file(nl.get(), std::filesystem::path(""));
                EXPECT_FALSE(suc);
            }
            {
                // Deserialize a netlist from a non existing path
                NO_COUT_TEST_BLOCK;
                auto des_nl =
                    netlist_serializer::deserialize_from_file(std::filesystem::path("/using/this/file/is/let.hal"));
                EXPECT_EQ(des_nl, nullptr);
            }
            {
                // Deserialize invalid input
                NO_COUT_TEST_BLOCK;
                std::filesystem::path test_hal_file_path = test_utils::create_sandbox_path("test_hal_file.hal");
                std::ofstream myfile;
                myfile.open(test_hal_file_path.string());
                myfile << "I h4ve no JSON f0rmat!!!\n(Temporary file for testing. Should be already deleted...)";
                myfile.close();
                auto des_nl = netlist_serializer::deserialize_from_file(test_hal_file_path);
                EXPECT_EQ(des_nl, nullptr);
            }
        TEST_END
    }
}    //namespace hal
