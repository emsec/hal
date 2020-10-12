#include "hal_core/netlist/hdl_writer/hdl_writer_manager.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/hdl_parser/hdl_parser_manager.h"
#include "hal_core/netlist/netlist.h"
#include "netlist_test_utils.h"
#include "hal_core/plugin_system/plugin_manager.h"

#include "gtest/gtest.h"
#include "hal_core/utilities/program_arguments.h"
#include <experimental/filesystem>

namespace hal {

    class hdl_writer_managerTest : public ::testing::Test {
    protected:
        const std::string m_min_gl_content = "library (MIN_TEST_GATE_LIBRARY_FOR_WRITER_MANAGER_TESTS) {\n"
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
        GateLibrary* m_gl;
        virtual void SetUp() {
            NO_COUT_BLOCK;
            test_utils::init_log_channels();
            test_utils::create_sandbox_directory();
            plugin_manager::load_all_plugins();
            m_g_lib_path = test_utils::create_sandbox_file("min_test_gate_lib_for_writer_manager_tests.lib", m_min_gl_content);
            m_gl = gate_library_manager::load_file(m_g_lib_path);
        }

        virtual void TearDown() {
            NO_COUT_BLOCK;
            test_utils::remove_sandbox_directory();
            // std::filesystem::remove_all(m_tmp_dir);
            plugin_manager::unload_all_plugins();
        }

        // Creates the following netlist:     global_in ---= INV =--- global_out
        std::unique_ptr<Netlist> create_simple_netlist() {
            auto nl = std::make_unique<Netlist>(m_gl);
            nl->set_design_name("top_module");
            nl->get_top_module()->set_name("top_module");
            nl->get_top_module()->set_type("top_module_type");
            Gate* inv_gate = nl->create_gate(test_utils::MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "inv_gate_inst");
            Net* g_in_net = nl->create_net("global_in");
            Net* g_out_net = nl->create_net("global_out");
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
    TEST_F(hdl_writer_managerTest, check_cli_options) {
        TEST_START
            {// Access the gui-options and the cli-options (shouldn't  be empty)
                EXPECT_FALSE(hdl_writer_manager::get_cli_options().get_options().empty());
            }
        TEST_END
    }

    /**
     * Testing writing of a netlist using program arguments
     *
     * Functions: write
     */
    TEST_F(hdl_writer_managerTest, check_write_by_program_args) {
        TEST_START
            {// Write a netlist to two files, by passing the file path in the arguments
                std::string out_path_vhdl = test_utils::create_sandbox_path("writer_out_vhdl.vhd").string();
                std::string out_path_verilog = test_utils::create_sandbox_path("writer_out_verilog.v").string();
                ProgramArguments p_args_vhdl;
                ProgramArguments p_args_verilog;
                p_args_vhdl.set_option("--write-hdl", std::vector<std::string>({out_path_vhdl}));
                p_args_verilog.set_option("--write-hdl", std::vector<std::string>({out_path_verilog}));

                auto simple_nl = create_simple_netlist();
                // Write the two files
                bool suc = hdl_writer_manager::write(simple_nl.get(), p_args_vhdl);
                EXPECT_TRUE(suc);
                suc = hdl_writer_manager::write(simple_nl.get(), p_args_verilog);
                EXPECT_TRUE(suc);

                // Verify the correctness of the output by parsing it
                auto parsed_nl_vhdl = hdl_parser_manager::parse(out_path_vhdl, m_gl);
                auto parsed_nl_verilog = hdl_parser_manager::parse(out_path_verilog, m_gl);

                parsed_nl_vhdl->get_top_module()->set_type("top_module_type");
                parsed_nl_verilog->get_top_module()->set_type("top_module_type");
                EXPECT_TRUE(test_utils::netlists_are_equal(parsed_nl_vhdl.get(), simple_nl.get(), true));
                EXPECT_TRUE(test_utils::netlists_are_equal(parsed_nl_verilog.get(), simple_nl.get(), true));
            }
        TEST_END
    }

    /**
     * Testing the writing of a netlist by passing a format and a file name
     *
     * Functions: write
     */
    TEST_F(hdl_writer_managerTest, check_write_by_format_and_filename) {
        TEST_START
            {
                // Write a netlist to two files, by passing the file path in the arguments
                std::filesystem::path out_path_vhdl = test_utils::create_sandbox_path("writer_out_vhdl.vhd");
                std::filesystem::path out_path_verilog = test_utils::create_sandbox_path("writer_out_verilog.v");

                auto simple_nl = create_simple_netlist();
                // Write the two files
                bool suc_vhdl = hdl_writer_manager::write(simple_nl.get(), out_path_vhdl);
                bool suc_verilog = hdl_writer_manager::write(simple_nl.get(), out_path_verilog);
                EXPECT_TRUE(suc_vhdl);
                EXPECT_TRUE(suc_verilog);

                // Verify the correctness of the output by parsing it
                auto parsed_nl_vhdl = hdl_parser_manager::parse(out_path_vhdl, m_gl);
                auto parsed_nl_verilog = hdl_parser_manager::parse(out_path_verilog, m_gl);

                parsed_nl_vhdl->get_top_module()->set_type("top_module_type");
                parsed_nl_verilog->get_top_module()->set_type("top_module_type");
                EXPECT_TRUE(test_utils::netlists_are_equal(parsed_nl_vhdl.get(), simple_nl.get(), true));
                EXPECT_TRUE(test_utils::netlists_are_equal(parsed_nl_verilog.get(), simple_nl.get(), true));
            }
            // NEAGTIVE
            {
                // The format is unknown
                std::filesystem::path out_path = test_utils::create_sandbox_path("unknown_format.sdfwefsdfs");

                auto simple_nl = create_simple_netlist();
                // Write the two files
                EXPECT_FALSE(hdl_writer_manager::write(simple_nl.get(), out_path));
            }
        TEST_END
    }

    /**
    * Testing the writing of a netlist to a stringstream
    *
    * Functions: write
    */
    TEST_F(hdl_writer_managerTest, check_write_to_stringstream) {
        TEST_START
            {
                auto simple_nl = create_simple_netlist();
                // Write the two files
                std::stringstream out_ss_vhdl;
                std::stringstream out_ss_verilog;
                bool suc_vhdl = hdl_writer_manager::write(simple_nl.get(), ".vhdl", out_ss_vhdl);
                bool suc_verilog = hdl_writer_manager::write(simple_nl.get(), ".v", out_ss_verilog);

                EXPECT_TRUE(suc_vhdl);
                EXPECT_TRUE(suc_verilog);

                std::filesystem::path out_path_vhdl = test_utils::create_sandbox_file("writer_out_vhdl.vhd", out_ss_vhdl.str());
                std::filesystem::path out_path_verilog = test_utils::create_sandbox_file("writer_out_verilog.v", out_ss_verilog.str());

                // Verify the correctness of the output by parsing it
                auto parsed_nl_vhdl = hdl_parser_manager::parse(out_path_vhdl, m_gl);
                auto parsed_nl_verilog = hdl_parser_manager::parse(out_path_verilog, m_gl);

                parsed_nl_vhdl->get_top_module()->set_type("top_module_type");
                parsed_nl_verilog->get_top_module()->set_type("top_module_type");
                EXPECT_TRUE(test_utils::netlists_are_equal(parsed_nl_vhdl.get(), simple_nl.get(), true));
                EXPECT_TRUE(test_utils::netlists_are_equal(parsed_nl_verilog.get(), simple_nl.get(), true));
            }
            // NEAGTIVE
            {
                // The type extension is unknown
                NO_COUT_TEST_BLOCK;
                auto simple_nl = create_simple_netlist();
                std::stringstream dummy_ss;
                EXPECT_FALSE(hdl_writer_manager::write(simple_nl.get(), ".unknown_extension",dummy_ss));
            }
        TEST_END
    }
} //namespace hal
