#include "netlist/hdl_parser/hdl_parser_manager.h"

#include "netlist/netlist.h"
#include "netlist_test_utils.h"
#include "core/plugin_manager.h"

#include "gtest/gtest.h"
#include "core/program_arguments.h"
// #include <experimental/filesystem>
#include <sstream>

namespace hal {
    class hdl_parser_managerTest : public ::testing::Test {
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
        std::string m_valid_verilog_content = "module top (\n"
                                              "  net_global_in,\n"
                                              "  net_global_out\n"
                                              " ) ;\n"
                                              "  input net_global_in ;\n"
                                              "  output net_global_out ;\n"
                                              "gate_1_to_1 gate_0 (\n"
                                              "  .\\I (net_global_in ),\n"
                                              "  .\\O (net_global_out)\n"
                                              " ) ;\n"
                                              "endmodule";

        std::string m_valid_vhdl_content = "-- Device\t: device_name\n"
                                           "entity TEST_Comp is\n"
                                           "  port (\n"
                                           "    net_global_in : in STD_LOGIC := 'X';\n"
                                           "    net_global_out : out STD_LOGIC := 'X';\n"
                                           "  );\n"
                                           "end TEST_Comp;\n"
                                           "\n"
                                           "architecture STRUCTURE of TEST_Comp is\n"
                                           "begin\n"
                                           "  gate_0 : gate_1_to_1\n"
                                           "    port map (\n"
                                           "      I => net_global_in,\n"
                                           "      O => net_global_out\n"
                                           "    );\n"
                                           "end STRUCTURE;";

        // std::string m_invalid_input         = "(#+;$: INVALID INPUT :$;+#(";
        std::string m_invalid_input = "(+;$: INVALID INPUT :$;+(";
        //GateLibrary* m_gl;
        std::filesystem::path m_g_lib_path;

        virtual void SetUp() {
            NO_COUT_BLOCK;
            test_utils::init_log_channels();
            test_utils::create_sandbox_directory();
            PluginManager::load_all_plugins();
            //m_gl = test_utils::get_testing_gate_library();
            m_g_lib_path = test_utils::create_sandbox_file("min_test_gate_lib.lib", m_min_gl_content);

        }

        virtual void TearDown() {
            NO_COUT_BLOCK;
            PluginManager::unload_all_plugins();
            test_utils::remove_sandbox_directory();
        }
    };

    /**
     * Testing the access on usage information: the cli-options
     *
     * Functions: get_cli_options
     */
    TEST_F(hdl_parser_managerTest, check_cli_options) {
        TEST_START
            {// Access the cli-options (should be empty)
                EXPECT_TRUE(hdl_parser_manager::get_cli_options().get_options().empty());
            }
        TEST_END
    }

    /**
     * Testing the parse function that takes ProgramArguments as input.
     *
     * Functions: parse
     */
    TEST_F(hdl_parser_managerTest, check_parse_by_program_args) {
        TEST_START
            // Create the files to parse
            auto vhdl_file_with_extension = test_utils::create_sandbox_file("tmp_vhdl_file.vhdl", m_valid_vhdl_content);
            auto verilog_file_with_extension = test_utils::create_sandbox_file("tmp_verilog_file.v", m_valid_verilog_content);
            auto file_with_unknown_extension = test_utils::create_sandbox_file("unknown_ext.seuifsiuef", m_invalid_input);
            auto file_without_extension = test_utils::create_sandbox_file("no_ext", m_invalid_input);
            // Load a gate library
            {
                NO_COUT_BLOCK;
                gate_library_manager::load_file(m_g_lib_path);
            }
            {
                // Parse a vhdl and a verilog file by passing no relevant program arguments. The parser should be determined
                // from the file extension ('.vhdl'/'.v') while the GateLibrary should be guessed.
                ProgramArguments p_args_empty;
                p_args_empty.set_option("--gate-library", std::vector<std::string>({m_g_lib_path}));

                std::unique_ptr<Netlist> nl_vhdl;
                std::unique_ptr<Netlist> nl_verilog;
                {
                    NO_COUT_BLOCK;
                    nl_vhdl = hdl_parser_manager::parse(vhdl_file_with_extension, p_args_empty);
                    nl_verilog = hdl_parser_manager::parse(verilog_file_with_extension, p_args_empty);
                }

                EXPECT_NE(nl_vhdl, nullptr);
                EXPECT_NE(nl_verilog, nullptr);
            }
            // NEGATIVE
            {
                // Pass a non-existing netlist
                ProgramArguments p_args;
                p_args.set_option("--gate-library", m_g_lib_path);
                std::unique_ptr<Netlist> nl;
                {
                    NO_COUT_BLOCK;
                    nl = hdl_parser_manager::parse("this/path/does/not/exist.v", p_args);
                }
                EXPECT_EQ(nl, nullptr);
            }
            {
                // Pass a non-existing gate library
                ProgramArguments p_args;
                p_args.set_option("--gate-library", "this/path/does/not/exist.v");
                std::unique_ptr<Netlist> nl;
                {
                    NO_COUT_BLOCK;
                    nl = hdl_parser_manager::parse(vhdl_file_with_extension, p_args);
                }
                EXPECT_EQ(nl, nullptr);
            }
            {
                // Pass an invalid file extension
                ProgramArguments p_args;
                p_args.set_option("--gate-library", m_g_lib_path);
                std::unique_ptr<Netlist> nl;
                {
                    NO_COUT_BLOCK;
                    nl = hdl_parser_manager::parse(file_with_unknown_extension, p_args);
                }
                EXPECT_EQ(nl, nullptr);
            }
            {
                // Pass file without extension
                ProgramArguments p_args;
                p_args.set_option("--gate-library", m_g_lib_path);
                std::unique_ptr<Netlist> nl;
                {
                    NO_COUT_BLOCK;
                    nl = hdl_parser_manager::parse(file_without_extension, p_args);
                }
                EXPECT_EQ(nl, nullptr);
            }

        TEST_END
    }

    /**
     * Testing the parse function that takes a parser name and a Gate library explicitly
     *
     * Functions: parse
     */
    TEST_F(hdl_parser_managerTest, check_parse_by_parser_name) {
        TEST_START
            // Create the files to parse
            auto vhdl_file = test_utils::create_sandbox_file("tmp_vhdl_file.vhd", m_valid_vhdl_content);
            auto verilog_file = test_utils::create_sandbox_file("tmp_verilog_file.v", m_valid_verilog_content);
            auto file_with_unknown_extension = test_utils::create_sandbox_file("unknown_ext.seuifsiuef", m_invalid_input);
            auto file_without_extension = test_utils::create_sandbox_file("no_ext", m_invalid_input);
            // Load a gate library
            GateLibrary* min_gl;
            {
                NO_COUT_BLOCK;
                min_gl = gate_library_manager::load_file(m_g_lib_path);
            }
            {
                // Parse a vhdl and a verilog file by GateLibrary
                std::unique_ptr<Netlist> nl_vhdl;
                std::unique_ptr<Netlist> nl_verilog;
                {
                    NO_COUT_BLOCK;
                    nl_vhdl = hdl_parser_manager::parse(vhdl_file, min_gl);
                    nl_verilog = hdl_parser_manager::parse(verilog_file, min_gl);
                }

                EXPECT_NE(nl_vhdl, nullptr);
                EXPECT_NE(nl_verilog, nullptr);
            }
            // NEGATIVE
            {
                // Pass an unknown file path
                std::unique_ptr<Netlist> nl;
                {
                    NO_COUT_BLOCK;
                    nl = hdl_parser_manager::parse("this/path/does/not/exist.v", min_gl);
                }

                EXPECT_EQ(nl, nullptr);
            }
            {
                // Pass an unknown file extension
                std::unique_ptr<Netlist> nl;
                {
                    NO_COUT_BLOCK;
                    nl = hdl_parser_manager::parse(file_with_unknown_extension, min_gl);
                }

                EXPECT_EQ(nl, nullptr);
            }
            {
                // Pass a file without extension
                std::unique_ptr<Netlist> nl;
                {
                    NO_COUT_BLOCK;
                    nl = hdl_parser_manager::parse(file_without_extension, min_gl);
                }

                EXPECT_EQ(nl, nullptr);
            }
        TEST_END
    }
} //namespace hal
