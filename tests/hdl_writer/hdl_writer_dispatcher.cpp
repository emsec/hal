#include "netlist/hdl_writer/hdl_writer_dispatcher.h"

#include "netlist/gate.h"
#include "netlist/hdl_parser/hdl_parser_manager.h"
#include "netlist/netlist.h"
#include "netlist_test_utils.h"
#include "core/plugin_manager.h"

#include "gtest/gtest.h"
#include <core/program_arguments.h>
#include <experimental/filesystem>

namespace hal {

    class HDLWriterDispatcherTest : public ::testing::Test {
    protected:
        const std::string m_min_gl_content = "library (MIN_TEST_GATE_LIBRARY) {\n"
                                             "    define(cell);\n"
                                             "    cell(gate_1_to_1) {\n"
                                             "        pin(I) { direction: input; }\n"
                                             "        pin(O) { direction: output; } }\n"
                                             "    cell(gnd) {\n"
                                             "        pin(O) { direction: output; function: \"0\"; } }\n"
                                             "    cell(vcc) {\n"
                                             "        pin(O) { direction: output; function: \"1\"; } }\n"
                                             "}";
        std::filesystem::path m_g_lib_path;
        std::shared_ptr<GateLibrary> m_gl;
        virtual void SetUp() {
            NO_COUT_BLOCK;
            test_utils::init_log_channels();
            test_utils::create_sandbox_directory();
            PluginManager::load_all_plugins();
            m_g_lib_path = test_utils::create_sandbox_file("min_test_gate_lib.lib", m_min_gl_content);
            m_gl = gate_library_manager::load_file(m_g_lib_path);
        }

        virtual void TearDown() {
            test_utils::remove_sandbox_directory();
            // std::filesystem::remove_all(m_tmp_dir);
            PluginManager::unload_all_plugins();
        }

        // Creates the following netlist:     global_in ---= INV =--- global_out
        std::shared_ptr<Netlist> create_simple_netlist() {
            std::shared_ptr<Netlist> nl = std::make_shared<Netlist>(m_gl);
            nl->set_design_name("top_module");
            nl->get_top_module()->set_name("top_module");
            nl->get_top_module()->set_type("top_module_type");
            std::shared_ptr<Gate> inv_gate = nl->create_gate(test_utils::MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "inv_gate_inst");
            std::shared_ptr<Net> g_in_net = nl->create_net("global_in");
            std::shared_ptr<Net> g_out_net = nl->create_net("global_out");
            nl->mark_global_input_net(g_in_net);
            nl->mark_global_output_net(g_out_net);
            g_in_net->add_destination(inv_gate, "I");
            g_out_net->add_source(inv_gate, "O");

            return nl;
        }
    };

    /**
     * Testing the access on usage information: the cli-options
     *
     * Functions: get_cli_options
     */
    TEST_F(HDLWriterDispatcherTest, check_cli_options) {
        TEST_START
            {// Access the gui-options and the cli-options (shouldn't  be empty)
                EXPECT_FALSE(HDLWriterDispatcher::get_cli_options().get_options().empty());
            }
        TEST_END
    }

    /**
     * Testing writing of a netlist using program arguments
     *
     * Functions: get_cli_options
     */
    TEST_F(HDLWriterDispatcherTest, check_write_by_program_args) {
        TEST_START
            {// Write a netlist to two files, by passing the file path in the arguments
                std::string out_path_vhdl = test_utils::create_sandbox_path("writer_out_vhdl.vhd").string();
                std::string out_path_verilog = test_utils::create_sandbox_path("writer_out_verilog.v").string();
                ProgramArguments p_args_vhdl;
                p_args_vhdl.set_option("--write-vhdl", std::vector<std::string>({out_path_vhdl}));
                p_args_vhdl.set_option("--write-verilog", std::vector<std::string>({out_path_verilog}));

                std::shared_ptr<Netlist> simple_nl = create_simple_netlist();
                // Write the two files
                bool suc = HDLWriterDispatcher::write(simple_nl, p_args_vhdl);
                EXPECT_TRUE(suc);

                // Verify the correctness of the output by parsing it
                std::shared_ptr<Netlist> parsed_nl_vhdl = hdl_parser_manager::parse(out_path_vhdl, m_gl);
                std::shared_ptr<Netlist>
                    parsed_nl_verilog = hdl_parser_manager::parse(out_path_verilog, m_gl);

                parsed_nl_vhdl->get_top_module()->set_type("top_module_type");
                parsed_nl_verilog->get_top_module()->set_type("top_module_type");
                EXPECT_TRUE(test_utils::netlists_are_equal(parsed_nl_vhdl, simple_nl, true));
                EXPECT_TRUE(test_utils::netlists_are_equal(parsed_nl_verilog, simple_nl, true));
            }
        TEST_END
    }

    /**
     * Testing writing of a netlist by passing a format and a file name
     *
     * Functions: get_cli_options
     */
    TEST_F(HDLWriterDispatcherTest, check_write_by_format_and_filename) {
        TEST_START
            {
                // Write a netlist to two files, by passing the file path in the arguments
                std::filesystem::path out_path_vhdl = test_utils::create_sandbox_path("writer_out_vhdl.vhd");
                std::filesystem::path out_path_verilog = test_utils::create_sandbox_path("writer_out_verilog.v");

                std::shared_ptr<Netlist> simple_nl = create_simple_netlist();
                // Write the two files
                bool suc_vhdl = HDLWriterDispatcher::write(simple_nl, "vhdl", out_path_vhdl);
                bool suc_verilog = HDLWriterDispatcher::write(simple_nl, "verilog", out_path_verilog);
                EXPECT_TRUE(suc_vhdl);
                EXPECT_TRUE(suc_verilog);

                // Verify the correctness of the output by parsing it
                std::shared_ptr<Netlist> parsed_nl_vhdl = hdl_parser_manager::parse(out_path_vhdl, m_gl);
                std::shared_ptr<Netlist>
                    parsed_nl_verilog = hdl_parser_manager::parse(out_path_verilog, m_gl);

                parsed_nl_vhdl->get_top_module()->set_type("top_module_type");
                parsed_nl_verilog->get_top_module()->set_type("top_module_type");
                EXPECT_TRUE(test_utils::netlists_are_equal(parsed_nl_vhdl, simple_nl, true));
                EXPECT_TRUE(test_utils::netlists_are_equal(parsed_nl_verilog, simple_nl, true));
            }
            // NEAGTIVE
            {
                // The format is unknown
                std::filesystem::path out_path = test_utils::create_sandbox_path("unknown_format.txt");

                std::shared_ptr<Netlist> simple_nl = create_simple_netlist();
                // Write the two files
                EXPECT_FALSE(HDLWriterDispatcher::write(simple_nl, "<unknown_format>", out_path));
            }
        TEST_END
    }
} //namespace hal
