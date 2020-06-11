#include "netlist/persistent/netlist_serializer.h"
#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/netlist.h"
#include "netlist/netlist_factory.h"
#include "netlist_test_utils.h"
#include "gtest/gtest.h"
#include <experimental/filesystem>
#include <core/log.h>
#include <core/utils.h>
#include <iostream>
#include <netlist/gate.h>
#include <netlist/net.h>
#include <netlist/module.h>

#include <fstream>
#include <streambuf>
#include <string>

using namespace test_utils;

class netlist_serializer_test : public ::testing::Test
{
protected:
    hal::path m_g_lib_path;
    std::shared_ptr<gate_library> m_gl;

    std::string m_min_gl_content =   "library (MIN_TEST_GATE_LIBRARY) {\n"
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

    virtual void SetUp()
    {
        NO_COUT_BLOCK;
        test_utils::init_log_channels();
        create_sandbox_directory();
        m_g_lib_path = create_sandbox_file("min_test_gate_lib.lib", m_min_gl_content);
        m_gl = gate_library_manager::load_file(m_g_lib_path);
    }

    virtual void TearDown()
    {
        remove_sandbox_directory();
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
    *     =                       =                 =----------=
    *       gate_2_to_0 (6)         gate_2_to_1 (7)            ...  gate_2_to_1 (8) =
    *     =                       =                 =          =
    */
    std::shared_ptr<netlist> create_example_serializer_netlist()
    {
        std::shared_ptr<netlist> nl = std::make_shared<netlist>(m_gl);
        nl->set_id(123);
        nl->set_input_filename("esnl_input_filename");
        nl->set_device_name("esnl_device_name");
        nl->get_top_module()->set_type("top_mod_type");

        // Create the gates
        std::shared_ptr<gate> gate_0 = nl->create_gate(MIN_GATE_ID + 0, m_gl->get_gate_types().at("gate_2_to_1"), "gate_0");
        std::shared_ptr<gate> gate_1 = nl->create_gate(MIN_GATE_ID + 1, m_gl->get_gate_types().at("gnd"), "gate_1");
        std::shared_ptr<gate> gate_2 = nl->create_gate(MIN_GATE_ID + 2, m_gl->get_gate_types().at("vcc"), "gate_2");
        std::shared_ptr<gate> gate_3 = nl->create_gate(MIN_GATE_ID + 3, m_gl->get_gate_types().at("gate_1_to_1"), "gate_3");
        std::shared_ptr<gate> gate_4 = nl->create_gate(MIN_GATE_ID + 4, m_gl->get_gate_types().at("gate_1_to_1"), "gate_4");
        std::shared_ptr<gate> gate_5 = nl->create_gate(MIN_GATE_ID + 5, m_gl->get_gate_types().at("gate_2_to_1"), "gate_5");
        std::shared_ptr<gate> gate_6 = nl->create_gate(MIN_GATE_ID + 6, m_gl->get_gate_types().at("gate_2_to_0"), "gate_6");
        std::shared_ptr<gate> gate_7 = nl->create_gate(MIN_GATE_ID + 7, m_gl->get_gate_types().at("gate_2_to_1"), "gate_7");
        std::shared_ptr<gate> gate_8 = nl->create_gate(MIN_GATE_ID + 8, m_gl->get_gate_types().at("gate_2_to_1"), "gate_8");

        // Add the nets (net_x_y1_y2... := net between the gate with id x and the gates y1,y2,...)
        std::shared_ptr<net> net_1_3 = nl->create_net(MIN_NET_ID + 13, "net_1_3");
        net_1_3->add_source(gate_1, "O");
        net_1_3->add_destination(gate_3, "I");

        std::shared_ptr<net> net_3_0 = nl->create_net(MIN_NET_ID + 30, "net_3_0");
        net_3_0->add_source(gate_3, "O");
        net_3_0->add_destination(gate_0, "I0");

        std::shared_ptr<net> net_2_0 = nl->create_net(MIN_NET_ID + 20, "net_2_0");
        net_2_0->add_source(gate_2, "O");
        net_2_0->add_destination(gate_0, "I1");

        std::shared_ptr<net> net_0_4_5 = nl->create_net(MIN_NET_ID + 045, "net_0_4_5");
        net_0_4_5->add_source(gate_0, "O");
        net_0_4_5->add_destination(gate_4, "I");
        net_0_4_5->add_destination(gate_5, "I0");

        std::shared_ptr<net> net_7_8 = nl->create_net(MIN_NET_ID + 78, "net_7_8");
        net_7_8->add_source(gate_7, "O");
        net_7_8->add_destination(gate_8, "I0");

        // Mark some gates as gnd/vcc gates
        nl->mark_gnd_gate(nl->get_gate_by_id(MIN_GATE_ID+1));
        nl->mark_vcc_gate(nl->get_gate_by_id(MIN_GATE_ID+2));

        // Mark some nets as global nets
        nl->mark_global_input_net(nl->get_net_by_id(MIN_NET_ID+13));
        nl->mark_global_output_net(nl->get_net_by_id(MIN_NET_ID+30));

        // Create the modules
        std::shared_ptr<module> test_m_0 = nl->create_module(MIN_MODULE_ID+0, "test_mod_0", nl->get_top_module());
        test_m_0->set_type("test_mod_type_0");
        test_m_0->assign_gate(nl->get_gate_by_id(MIN_GATE_ID+0));
        test_m_0->assign_gate(nl->get_gate_by_id(MIN_GATE_ID+3));

        std::shared_ptr<module> test_m_1 = nl->create_module(MIN_MODULE_ID+1, "test_mod_1", test_m_0);
        test_m_1->set_type("test_mod_type_1");
        test_m_1->assign_gate(nl->get_gate_by_id(MIN_GATE_ID+0));


        // Store some data in a gate, net and module
        nl->get_gate_by_id(MIN_GATE_ID+1)->set_data("category_0", "key_0", "data_type", "test_value");
        nl->get_gate_by_id(MIN_GATE_ID+1)->set_data("category_1", "key_1", "data_type", "test_value_1");
        nl->get_gate_by_id(MIN_GATE_ID+1)->set_data("category_1", "key_0", "data_type", "test_value_2");
        nl->get_net_by_id(MIN_NET_ID+13)->set_data("category", "key_2", "data_type", "test_value");
        test_m_0->set_data("category", "key_3", "data_type", "test_value");

        // Set some input/output port names of module 0

        test_m_0->set_input_port_name(net_1_3, "test_m_0_net_1_3_in");
        test_m_0->set_input_port_name(net_2_0, "test_m_0_net_2_0_in");
        test_m_0->set_output_port_name(net_0_4_5, "test_m_0_net_0_4_5_out");

        test_m_1->set_output_port_name(net_0_4_5, "test_m_1_net_0_4_5_out");

        return nl;
    }

};

/**
 * Testing the serialization and a followed deserialization of the example
 * netlist.
 *
 * Functions: serialize_netlist, deserialize_netlist
 */
TEST_F(netlist_serializer_test, check_serialize_and_deserialize){
    TEST_START
        {
            // Serialize and deserialize the example netlist (with some additions) and compare the result with the original netlist
            std::shared_ptr<netlist> nl = create_example_serializer_netlist();

            // Serialize and deserialize the netlist now
            hal::path test_hal_file_path = create_sandbox_path("test_hal_file.hal");
            bool suc                        = netlist_serializer::serialize_to_file(nl, test_hal_file_path);

            std::shared_ptr<netlist> des_nl = netlist_serializer::deserialize_from_file(test_hal_file_path);


            EXPECT_TRUE(suc);

            ASSERT_NE(des_nl, nullptr);

            // Check if the original netlist and the deserialized one (from file) are equal

            // -- Check if the id and the gate library is the same
            EXPECT_EQ(nl->get_id(), des_nl->get_id());
            EXPECT_EQ(nl->get_gate_library()->get_name(), des_nl->get_gate_library()->get_name());

            // -- Check if gates and nets are the same
            EXPECT_EQ(nl->get_gates().size(), des_nl->get_gates().size());
            for (auto g_0 : nl->get_gates())
            {
                EXPECT_TRUE(gates_are_equal(g_0, des_nl->get_gate_by_id(g_0->get_id())));
            }


            EXPECT_EQ(nl->get_nets().size(), des_nl->get_nets().size());

            for (auto n_0 : nl->get_nets())
            {
                EXPECT_TRUE(nets_are_equal(n_0, des_nl->get_net_by_id(n_0->get_id())));
            }

            // -- Check if global gates are the same
            EXPECT_EQ(nl->get_gnd_gates().size(), des_nl->get_gnd_gates().size());
            for (auto gl_gnd_0 : nl->get_gnd_gates())
            {
                EXPECT_TRUE(des_nl->is_gnd_gate(des_nl->get_gate_by_id(gl_gnd_0->get_id())));
            }

            EXPECT_EQ(nl->get_vcc_gates().size(), des_nl->get_vcc_gates().size());
            for (auto gl_vcc_0 : nl->get_vcc_gates())
            {
                EXPECT_TRUE(des_nl->is_vcc_gate(des_nl->get_gate_by_id(gl_vcc_0->get_id())));
            }

            // -- Check if global nets are the same
            EXPECT_EQ(nl->get_global_input_nets().size(), des_nl->get_global_input_nets().size());
            for (auto gl_in_net : nl->get_global_input_nets())
            {
                EXPECT_TRUE(des_nl->is_global_input_net(des_nl->get_net_by_id(gl_in_net->get_id())));
            }

            EXPECT_EQ(nl->get_global_output_nets().size(), des_nl->get_global_output_nets().size());
            for (auto gl_out_net : nl->get_global_output_nets())
            {
                EXPECT_TRUE(des_nl->is_global_output_net(des_nl->get_net_by_id(gl_out_net->get_id())));
            }

            // -- Check if the modules are the same
            EXPECT_EQ(nl->get_modules().size(), des_nl->get_modules().size());
            std::set<std::shared_ptr<module>> mods_1 = des_nl->get_modules();
            for(auto m_0 : nl->get_modules()){
                EXPECT_TRUE(modules_are_equal(m_0, des_nl->get_module_by_id(m_0->get_id())));
            }

            // -- Check the netlists as a whole
            EXPECT_TRUE(netlists_are_equal(nl, des_nl));

        }
        {
            // Test the example netlist against its deserialized version, but flip the module ids
            std::shared_ptr<netlist> nl = create_example_serializer_netlist();
            // -- Remove the modules
            nl->delete_module(nl->get_module_by_id(MIN_MODULE_ID+0));
            nl->delete_module(nl->get_module_by_id(MIN_MODULE_ID+1));
            // -- Add them again with flipped ids
            std::shared_ptr<module> test_m_0_flipped = nl->create_module(MIN_MODULE_ID+1, "test_mod_0_flipped", nl->get_top_module());
            test_m_0_flipped->set_type("test_mod_type_0_flipped");
            test_m_0_flipped->assign_gate(nl->get_gate_by_id(MIN_GATE_ID+0));
            test_m_0_flipped->assign_gate(nl->get_gate_by_id(MIN_GATE_ID+3));

            std::shared_ptr<module> test_m_1_flipped = nl->create_module(MIN_MODULE_ID+0, "test_mod_1_flipped", test_m_0_flipped);
            test_m_1_flipped->set_type("test_mod_type_1_flipped");
            test_m_1_flipped->assign_gate(nl->get_gate_by_id(MIN_GATE_ID+0));

            test_m_0_flipped->set_data("category", "key_3", "data_type", "test_value");

            // Set some input/output port names of module 0
            test_m_0_flipped->set_input_port_name(nl->get_net_by_id(MIN_NET_ID+13), "test_m_0_flipped_net_1_3_in");
            test_m_0_flipped->set_input_port_name(nl->get_net_by_id(MIN_NET_ID+20), "test_m_0_flipped_net_2_0_in");
            test_m_0_flipped->set_output_port_name(nl->get_net_by_id(MIN_NET_ID+045), "test_m_0_flipped_net_0_4_5_out");

            test_m_1_flipped->set_output_port_name(nl->get_net_by_id(MIN_NET_ID+045), "test_m_1_flipped_net_0_4_5_out");

            // Serialize and deserialize the netlist now
            hal::path test_hal_file_path = create_sandbox_path("test_hal_file.hal");
            bool suc                        = netlist_serializer::serialize_to_file(nl, test_hal_file_path);

            std::shared_ptr<netlist> des_nl = netlist_serializer::deserialize_from_file(test_hal_file_path);

            // -- Check the netlists as a whole
            EXPECT_TRUE(netlists_are_equal(nl, des_nl));

        }
        {
            // Serialize and deserialize an empty netlist and compare the result with the original netlist
            std::shared_ptr<netlist> nl = std::make_shared<netlist>(m_gl);

            hal::path test_hal_file_path = create_sandbox_path("test_hal_file.hal");
            bool suc                        = netlist_serializer::serialize_to_file(nl, test_hal_file_path);
            std::shared_ptr<netlist> des_nl = netlist_serializer::deserialize_from_file(test_hal_file_path);

            EXPECT_TRUE(suc);
            EXPECT_TRUE(netlists_are_equal(nl, des_nl));
        }
    TEST_END
}

/**
 * Testing the serialization and deserialization of a netlist with invalid input
 *
 * Functions: serialize_netlist, deserialize_netlist
 */
TEST_F(netlist_serializer_test, check_serialize_and_deserialize_negative)
{
    TEST_START
        {
            // Serialize a netlist which is a nullptr
            hal::path test_hal_file_path = create_sandbox_path("test_hal_file.hal");
            bool suc = netlist_serializer::serialize_to_file(nullptr, test_hal_file_path);
            EXPECT_FALSE(suc);
        }
        {
            // Serialize a netlist to an invalid path
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_example_serializer_netlist();
            bool suc                    = netlist_serializer::serialize_to_file(nl, hal::path(""));
            EXPECT_FALSE(suc);
        }
        {
            // Deserialize a netlist from a non existing path
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> des_nl = netlist_serializer::deserialize_from_file(hal::path("/using/this/file/is/let.hal"));
            EXPECT_EQ(des_nl, nullptr);
        }
        {
            // Deserialize invalid input
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl     = create_example_netlist(0);
            hal::path test_hal_file_path = create_sandbox_path("test_hal_file.hal");
            std::ofstream myfile;
            myfile.open (test_hal_file_path.string());
            myfile << "I h4ve no JSON f0rmat!!!\n(Temporary file for testing. Should be already deleted...)";
            myfile.close();
            std::shared_ptr<netlist> des_nl = netlist_serializer::deserialize_from_file(test_hal_file_path);
            EXPECT_EQ(des_nl, nullptr);
        }
    TEST_END
}
