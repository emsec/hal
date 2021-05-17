#include "gtest/gtest.h"
#include "gate_library_test_utils.h"
#include "netlist_test_utils.h"
#include "verilog_parser/verilog_parser.h"
#include "verilog_writer/verilog_writer.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"

namespace hal 
{
    class VerilogWriterTest : public ::testing::Test {
    protected:
        GateLibrary* m_gl;

        virtual void SetUp() 
        {
            NO_COUT_BLOCK;
            test_utils::init_log_channels();
            plugin_manager::load_all_plugins();
            test_utils::create_sandbox_directory();
            
            // gate library needs to be registered through gate_library_manager for serialization
            std::unique_ptr<GateLibrary> gl_tmp = test_utils::create_gate_library(test_utils::create_sandbox_path("testing_gate_library.hgl"));
            gate_library_manager::save(gl_tmp->get_path(), gl_tmp.get(), true);
            m_gl = gate_library_manager::load(gl_tmp->get_path());
        }

        virtual void TearDown() 
        {
            NO_COUT_BLOCK;
            plugin_manager::unload_all_plugins();
            // test_utils::remove_sandbox_directory();
        }
    };

    /**
     * Test writing a given netlist to file and subsequently parse it using the VerilogParser.
     *
     * Functions: write
     */
    TEST_F(VerilogWriterTest, check_main) {
        TEST_START
            {
                std::filesystem::path path_netlist = test_utils::create_sandbox_path("test.v");
                std::unique_ptr<Netlist> nl = std::make_unique<Netlist>(m_gl);
                
                {    
                    nl->set_design_name("top_module");
                    Module* top_module = nl->get_top_module();
                    top_module->set_type("top_module");

                    Gate* gate_0 = nl->create_gate(m_gl->get_gate_type_by_name("AND2"), "gate_0");
                    Gate* gate_1 = nl->create_gate(m_gl->get_gate_type_by_name("GND"), "gate_1");
                    Gate* gate_2 = nl->create_gate(m_gl->get_gate_type_by_name("VCC"), "gate_2");
                    Gate* gate_3 = nl->create_gate(m_gl->get_gate_type_by_name("INV"), "gate_3");
                    Gate* gate_4 = nl->create_gate(m_gl->get_gate_type_by_name("INV"), "gate_4");
                    Gate* gate_5 = nl->create_gate(m_gl->get_gate_type_by_name("AND2"), "gate_5");
                    Gate* gate_6 = nl->create_gate(m_gl->get_gate_type_by_name("AND2"), "gate_6");
                    Gate* gate_7 = nl->create_gate(m_gl->get_gate_type_by_name("AND2"), "gate_7");

                    gate_1->mark_gnd_gate();
                    gate_2->mark_vcc_gate();

                    Net* net_1_3 = test_utils::connect(nl.get(), gate_1, "O", gate_3, "I");
                    Net* net_3_0 = test_utils::connect(nl.get(), gate_3, "O", gate_0, "I0");
                    Net* net_2_0 = test_utils::connect(nl.get(), gate_2, "O", gate_0, "I1");
                    Net* net_0_4_5 = test_utils::connect(nl.get(), gate_0, "O", gate_4, "I");
                    test_utils::connect(nl.get(), gate_0, "O", gate_5, "I0");
                    Net* net_6_7 = test_utils::connect(nl.get(), gate_6, "O", gate_7, "I0");

                    Net* net_4_out = test_utils::connect_global_out(nl.get(), gate_4, "O", "net_4_out");
                    top_module->set_output_port_name(net_4_out, net_4_out->get_name());

                    Net* net_5_out = test_utils::connect_global_out(nl.get(), gate_5, "O", "net_5_out");
                    top_module->set_output_port_name(net_5_out, net_5_out->get_name());

                    Net* net_7_out = test_utils::connect_global_out(nl.get(), gate_7, "O", "net_7_out");
                    top_module->set_output_port_name(net_7_out, net_7_out->get_name());
                }

                VerilogWriter verilog_writer;
                ASSERT_TRUE(verilog_writer.write(nl.get(), path_netlist));

                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(path_netlist, m_gl);
                ASSERT_NE(parsed_nl, nullptr);

                // prepare comparison
                parsed_nl->set_device_name(nl->get_device_name());

                // compare netlists
                EXPECT_TRUE(test_utils::netlists_are_equal(nl.get(), parsed_nl.get(), true));
            }
        TEST_END
    }

    /**
     * Test writing global input and output nets.
     *
     * Functions: write
     */
    TEST_F(VerilogWriterTest, check_global_nets) {
        TEST_START
            {
                std::filesystem::path path_netlist = test_utils::create_sandbox_path("test.v");
                std::unique_ptr<Netlist> nl = std::make_unique<Netlist>(m_gl);
                
                {   
                    Module* top_module = nl->get_top_module();

                    Gate* gate_0 = nl->create_gate(m_gl->get_gate_type_by_name("BUF"), "gate_0");
                    Net* global_in_0 = test_utils::connect_global_in(nl.get(), gate_0, "I", "global_in_0");
                    top_module->set_input_port_name(global_in_0, global_in_0->get_name());

                    Gate* gate_1 = nl->create_gate(m_gl->get_gate_type_by_name("BUF"), "gate_1");
                    Net* global_in_1 = test_utils::connect_global_in(nl.get(), gate_1, "I", "global_in_1");
                    top_module->set_input_port_name(global_in_1, global_in_1->get_name());

                    Gate* gate_2 = nl->create_gate(m_gl->get_gate_type_by_name("BUF"), "gate_2");
                    Net* global_out_0 = test_utils::connect_global_out(nl.get(), gate_2, "O", "global_out_0");
                    top_module->set_output_port_name(global_out_0, global_out_0->get_name());

                    Gate* gate_3 = nl->create_gate(m_gl->get_gate_type_by_name("BUF"), "gate_3");
                    Net* global_out_1 = test_utils::connect_global_out(nl.get(), gate_3, "O", "global_out_1");
                    top_module->set_output_port_name(global_out_1, global_out_1->get_name());
                }


                VerilogWriter verilog_writer;
                ASSERT_TRUE(verilog_writer.write(nl.get(), path_netlist));

                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(path_netlist, m_gl);
                ASSERT_NE(parsed_nl, nullptr);

                EXPECT_EQ(parsed_nl->get_global_input_nets().size(), 2);
                EXPECT_EQ(parsed_nl->get_global_output_nets().size(), 2);

                std::vector<Net*> nets = parsed_nl->get_nets();
                ASSERT_EQ(nets.size(), 4);

                auto global_in_0_it = std::find_if(nets.begin(), nets.end(), [](const Net* net){ return net->get_name() == "global_in_0"; });
                ASSERT_NE(global_in_0_it, nets.end());
                EXPECT_TRUE((*global_in_0_it)->is_global_input_net());

                auto global_in_1_it = std::find_if(nets.begin(), nets.end(), [](const Net* net){ return net->get_name() == "global_in_1"; });
                ASSERT_NE(global_in_1_it, nets.end());
                EXPECT_TRUE((*global_in_1_it)->is_global_input_net());

                auto global_out_0_it = std::find_if(nets.begin(), nets.end(), [](const Net* net){ return net->get_name() == "global_out_0"; });
                ASSERT_NE(global_out_0_it, nets.end());
                EXPECT_TRUE((*global_out_0_it)->is_global_output_net());

                auto global_out_1_it = std::find_if(nets.begin(), nets.end(), [](const Net* net){ return net->get_name() == "global_out_1"; });
                ASSERT_NE(global_out_1_it, nets.end());
                EXPECT_TRUE((*global_out_1_it)->is_global_output_net());
            }
        TEST_END
    }

    /**
     * Test writing a modularized netlist.
     *
     * Functions: write
     */
    TEST_F(VerilogWriterTest, check_modules) {
        TEST_START
            {
                std::filesystem::path path_netlist = test_utils::create_sandbox_path("test.v");
                std::unique_ptr<Netlist> nl = std::make_unique<Netlist>(m_gl);
                
                {
                    Gate* buf_0 = nl->create_gate(m_gl->get_gate_type_by_name("BUF"), "buf_0");
                    Gate* buf_1 = nl->create_gate(m_gl->get_gate_type_by_name("BUF"), "buf_1");
                    Gate* buf_2 = nl->create_gate(m_gl->get_gate_type_by_name("BUF"), "buf_2");
                    Gate* buf_3 = nl->create_gate(m_gl->get_gate_type_by_name("BUF"), "buf_3");
                    Gate* inv_0 = nl->create_gate(m_gl->get_gate_type_by_name("INV"), "inv_0");
                    Gate* and2_0 = nl->create_gate(m_gl->get_gate_type_by_name("AND2"), "and2_0");
                    Gate* and2_1 = nl->create_gate(m_gl->get_gate_type_by_name("AND2"), "and2_1");
                    Gate* or2_0 = nl->create_gate(m_gl->get_gate_type_by_name("OR2"), "or2_0");
                    
                    Net* net_0 = test_utils::connect(nl.get(), buf_0, "O", and2_0, "I0", "net_0");
                    Net* net_1 = test_utils::connect(nl.get(), buf_1, "O", and2_0, "I1", "net_1");
                    Net* net_2 = test_utils::connect(nl.get(), and2_0, "O", or2_0, "I0", "net_2");
                    test_utils::connect(nl.get(), and2_0, "O", and2_1, "I0", "net_3");
                    Net* net_3 = test_utils::connect(nl.get(), inv_0, "O", or2_0, "I1", "net_3");
                    test_utils::connect(nl.get(), inv_0, "O", and2_1, "I1", "net_5");
                    Net* net_4 = test_utils::connect(nl.get(), or2_0, "O", buf_2, "I", "net_4");
                    Net* net_5 = test_utils::connect(nl.get(), and2_1, "O", buf_3, "I", "net_5");

                    Net* net_6 = test_utils::connect_global_in(nl.get(), buf_0, "I", "net_6");
                    Net* net_7 = test_utils::connect_global_in(nl.get(), buf_1, "I", "net_7");
                    Net* net_8 = test_utils::connect_global_in(nl.get(), inv_0, "I", "net_8");

                    Net* net_9 = test_utils::connect_global_out(nl.get(), buf_2, "O", "net_9");
                    Net* net_10 = test_utils::connect_global_out(nl.get(), buf_3, "O", "net_10");

                    Module* mod_0 = nl->create_module("mod_0", nl->get_top_module(), {buf_0, buf_1, and2_0});
                    Module* mod_1 = nl->create_module("mod_1", nl->get_top_module());
                    Module* mod_2 = nl->create_module("mod_2", mod_1, {or2_0, buf_2});
                    Module* mod_3 = nl->create_module("mod_3", mod_1, {and2_1, buf_3});
                }

                VerilogWriter verilog_writer;
                ASSERT_TRUE(verilog_writer.write(nl.get(), path_netlist));

                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(path_netlist, m_gl);
                ASSERT_NE(parsed_nl, nullptr);

                EXPECT_EQ(parsed_nl->get_nets().size(), 11);
                EXPECT_EQ(parsed_nl->get_global_input_nets().size(), 3);
                EXPECT_EQ(parsed_nl->get_global_output_nets().size(), 2);
                EXPECT_EQ(parsed_nl->get_gates().size(), 8);

                {
                    std::vector<Module*> modules = parsed_nl->get_modules();
                    EXPECT_EQ(modules.size(), 5);

                    const auto top_mod_it = std::find_if(modules.begin(), modules.end(), [](const Module* module){ return module->get_name() == "top_module"; });
                    ASSERT_NE(top_mod_it, modules.end());
                    EXPECT_EQ((*top_mod_it)->get_gates().size(), 1);
                    EXPECT_EQ((*top_mod_it)->get_input_nets().size(), 3);
                    EXPECT_EQ((*top_mod_it)->get_output_nets().size(), 2);
                    EXPECT_TRUE((*top_mod_it)->is_top_module());

                    const auto mod_0_it = std::find_if(modules.begin(), modules.end(), [](const Module* module){ return module->get_name() == "mod_0"; });
                    ASSERT_NE(mod_0_it, modules.end());
                    EXPECT_EQ((*mod_0_it)->get_gates().size(), 3);
                    EXPECT_EQ((*mod_0_it)->get_input_nets().size(), 2);
                    EXPECT_EQ((*mod_0_it)->get_output_nets().size(), 1);
                    EXPECT_EQ((*mod_0_it)->get_parent_module(), parsed_nl->get_top_module());

                    const auto mod_1_it = std::find_if(modules.begin(), modules.end(), [](const Module* module){ return module->get_name() == "mod_1"; });
                    ASSERT_NE(mod_1_it, modules.end());
                    EXPECT_EQ((*mod_1_it)->get_gates().size(), 0);
                    EXPECT_EQ((*mod_1_it)->get_input_nets().size(), 2);
                    EXPECT_EQ((*mod_1_it)->get_output_nets().size(), 2);
                    EXPECT_EQ((*mod_1_it)->get_parent_module(), parsed_nl->get_top_module());

                    const auto mod_2_it = std::find_if(modules.begin(), modules.end(), [](const Module* module){ return module->get_name() == "mod_2"; });
                    ASSERT_NE(mod_2_it, modules.end());
                    EXPECT_EQ((*mod_2_it)->get_gates().size(), 2);
                    EXPECT_EQ((*mod_2_it)->get_input_nets().size(), 2);
                    EXPECT_EQ((*mod_2_it)->get_output_nets().size(), 1);
                    EXPECT_EQ((*mod_2_it)->get_parent_module(), (*mod_1_it));

                    const auto mod_3_it = std::find_if(modules.begin(), modules.end(), [](const Module* module){ return module->get_name() == "mod_3"; });
                    ASSERT_NE(mod_3_it, modules.end());
                    EXPECT_EQ((*mod_3_it)->get_gates().size(), 2);
                    EXPECT_EQ((*mod_3_it)->get_input_nets().size(), 2);
                    EXPECT_EQ((*mod_3_it)->get_output_nets().size(), 1);
                    EXPECT_EQ((*mod_3_it)->get_parent_module(), (*mod_1_it));
                }
            }
        TEST_END
    } 

    /**
     * Test writing generic data.
     *
     * Functions: write
     */
    TEST_F(VerilogWriterTest, check_data) {
        TEST_START
            {
                std::filesystem::path path_netlist = test_utils::create_sandbox_path("test.v");
                std::unique_ptr<Netlist> nl = std::make_unique<Netlist>(m_gl);

                Gate* gate = nl->create_gate(m_gl->get_gate_type_by_name("BUF"), "gate_0");
                test_utils::connect_global_in(nl.get(), gate, "I");
                test_utils::connect_global_out(nl.get(), gate, "O");

                Module* mod = nl->create_module("mod", nl->get_top_module(), {gate});

                gate->set_data("generic", "test_bit_vector", "bit_vector", "123ABC");
                gate->set_data("generic", "test_string", "string", "one_two_three");
                gate->set_data("generic", "test_integer", "integer", "123");
                gate->set_data("generic", "test_float", "floating_point", "1.001");
                gate->set_data("generic", "test_bit_value", "bit_value", "1");

                // below data should be ignored when writing
                gate->set_data("generic", "test_invalid", "invalid", "ignore_me");
                gate->set_data("attribute", "test_attr_string", "string", "one_two_three");
                gate->set_data("random", "test_rand_string", "string", "one_two_three");

                mod->set_data("generic", "test_bit_vector", "bit_vector", "123ABC");
                mod->set_data("generic", "test_string", "string", "one_two_three");
                mod->set_data("generic", "test_integer", "integer", "123");
                mod->set_data("generic", "test_float", "floating_point", "1.001");
                mod->set_data("generic", "test_bit_value", "bit_value", "1");

                // below data should be ignored when writing
                mod->set_data("generic", "test_invalid", "invalid", "ignore_me");
                mod->set_data("attribute", "test_attr_string", "string", "one_two_three");
                mod->set_data("random", "test_rand_string", "string", "one_two_three");
                
                VerilogWriter verilog_writer;
                ASSERT_TRUE(verilog_writer.write(nl.get(), path_netlist));

                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(path_netlist, m_gl);
                ASSERT_NE(parsed_nl, nullptr);

                std::vector<Gate*> gates = parsed_nl->get_gates();
                ASSERT_EQ(gates.size(), 1);
                const Gate* parsed_gate = gates.front();
                ASSERT_NE(parsed_gate, nullptr);

                EXPECT_EQ(parsed_gate->get_data_map().size(), 5);
                EXPECT_EQ(parsed_gate->get_data("generic", "test_bit_vector"), std::make_tuple(std::string("bit_vector"), std::string("123ABC")));
                EXPECT_EQ(parsed_gate->get_data("generic", "test_string"), std::make_tuple(std::string("string"), std::string("one_two_three")));
                EXPECT_EQ(parsed_gate->get_data("generic", "test_integer"), std::make_tuple(std::string("integer"), std::string("123")));
                EXPECT_EQ(parsed_gate->get_data("generic", "test_float"), std::make_tuple(std::string("floating_point"), std::string("1.001")));
                EXPECT_EQ(parsed_gate->get_data("generic", "test_bit_value"), std::make_tuple(std::string("bit_value"), std::string("1")));

                std::vector<Module*> modules = parsed_nl->get_modules();
                ASSERT_EQ(modules.size(), 2);
                auto mod_it = std::find_if(modules.begin(), modules.end(), [](const Module* m){ return !m->is_top_module(); });
                ASSERT_NE(mod_it, modules.end());
                const Module* parsed_module = *mod_it;
                ASSERT_NE(parsed_module, nullptr);

                EXPECT_EQ(parsed_module->get_data_map().size(), 5);
                EXPECT_EQ(parsed_module->get_data("generic", "test_bit_vector"), std::make_tuple(std::string("bit_vector"), std::string("123ABC")));
                EXPECT_EQ(parsed_module->get_data("generic", "test_string"), std::make_tuple(std::string("string"), std::string("one_two_three")));
                EXPECT_EQ(parsed_module->get_data("generic", "test_integer"), std::make_tuple(std::string("integer"), std::string("123")));
                EXPECT_EQ(parsed_module->get_data("generic", "test_float"), std::make_tuple(std::string("floating_point"), std::string("1.001")));
                EXPECT_EQ(parsed_module->get_data("generic", "test_bit_value"), std::make_tuple(std::string("bit_value"), std::string("1")));
            }
        TEST_END
    }

    /**
     * Test writing gates with multi-bit pins.
     *
     * Functions: write
     */
    TEST_F(VerilogWriterTest, check_multi_bit_pins) {
        TEST_START
            {
                std::filesystem::path path_netlist = test_utils::create_sandbox_path("test.v");
                std::unique_ptr<Netlist> nl = std::make_unique<Netlist>(m_gl);

                Gate* gate = nl->create_gate(m_gl->get_gate_type_by_name("RAM"), "ram");

                Module* top_module = nl->get_top_module();

                for (u32 i = 0; i < 4; i++)
                {
                    Net* n = test_utils::connect_global_in(nl.get(), gate, "DATA_IN(" + std::to_string(i) + ")", "DATA_IN(" + std::to_string(i) + ")");
                    top_module->set_input_port_name(n, "DATA_IN(" + std::to_string(i) + ")");

                    n = test_utils::connect_global_out(nl.get(), gate, "DATA_OUT(" + std::to_string(i) + ")", "DATA_OUT(" + std::to_string(i) + ")");
                    top_module->set_output_port_name(n, "DATA_OUT(" + std::to_string(i) + ")");

                    n = test_utils::connect_global_in(nl.get(), gate, "ADDR(" + std::to_string(i) + ")", "ADDR(" + std::to_string(i) + ")");
                    top_module->set_input_port_name(n, "ADDR(" + std::to_string(i) + ")");
                }

                std::vector<Endpoint*> fan_in = gate->get_fan_in_endpoints();
                std::vector<Endpoint*> fan_out = gate->get_fan_out_endpoints();

                VerilogWriter verilog_writer;
                ASSERT_TRUE(verilog_writer.write(nl.get(), path_netlist));

                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(path_netlist, m_gl);
                ASSERT_NE(parsed_nl, nullptr);

                std::vector<Gate*> gates = parsed_nl->get_gates();
                ASSERT_EQ(gates.size(), 1);
                const Gate* parsed_gate = gates.front();
                ASSERT_NE(parsed_gate, nullptr);

                EXPECT_EQ(parsed_gate->get_fan_in_nets().size(), 8);
                EXPECT_EQ(parsed_gate->get_fan_out_nets().size(), 4);

                for (const Endpoint* ep : fan_in) 
                {
                    EXPECT_EQ(parsed_gate->get_fan_in_net(ep->get_pin())->get_name(), ep->get_net()->get_name());
                }

                for (const Endpoint* ep : fan_out) 
                {
                    EXPECT_EQ(parsed_gate->get_fan_out_net(ep->get_pin())->get_name(), ep->get_net()->get_name());
                }
            }
        TEST_END
    }
} //namespace hal
