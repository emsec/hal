#include "netlist/hdl_parser/hdl_parser_dispatcher.h"

#include "netlist/netlist.h"
#include "netlist_test_utils.h"

#include "gtest/gtest.h"
#include <core/program_arguments.h>
#include <experimental/filesystem>
#include <sstream>

namespace hal {
    class HDLParserDispatcherTest : public ::testing::Test {
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
        //std::shared_ptr<GateLibrary> m_gl;
        std::filesystem::path m_g_lib_path;

        virtual void SetUp() {
            NO_COUT_BLOCK;
            test_utils::init_log_channels();
            test_utils::create_sandbox_directory();
            //m_gl = test_utils::get_testing_gate_library();
            m_g_lib_path = test_utils::create_sandbox_file("min_test_gate_lib.lib", m_min_gl_content);

        }

        virtual void TearDown() {
            test_utils::remove_sandbox_directory();
        }
    };

    /**
     * Testing the access on usage information: the cli-options and the gui-options
     *
     * Functions: get_cli_options, get_gui_option
     */
    TEST_F(HDLParserDispatcherTest, check_cli_gui_options) {
        TEST_START
            {// Access the gui-options and the cli-options (shouldn't  be empty)
                EXPECT_FALSE(HDLParserDispatcher::get_cli_options().get_options().empty());
                EXPECT_FALSE(HDLParserDispatcher::get_gui_option().empty());
            }
        TEST_END
    }

    /**
     * Testing the parse function that takes ProgramArguments as input.
     *
     * Functions: parse
     */
    TEST_F(HDLParserDispatcherTest, check_parse_by_program_args) {
        TEST_START
            // Create the files to parse
            std::filesystem::path
                vhdl_file_with_extension = test_utils::create_sandbox_file("tmp_vhdl_file.vhdl", m_valid_vhdl_content);
            std::filesystem::path
                verilog_file_with_extension = test_utils::create_sandbox_file("tmp_verilog_file.v", m_valid_verilog_content);
            std::filesystem::path vhdl_file_without_extension = test_utils::create_sandbox_file("tmp_vhdl_file", m_valid_vhdl_content);
            std::filesystem::path
                verilog_file_without_extension = test_utils::create_sandbox_file("tmp_verilog_file", m_valid_verilog_content);
            std::filesystem::path invalid_file = test_utils::create_sandbox_file("invalid_file", m_invalid_input);
            // Load a gate library
            gate_library_manager::load_file(m_g_lib_path);
            {
                // Parse a vhdl and a verilog file by passing the parser name and the GateLibrary name in the program arguments.
                ProgramArguments p_args_vhdl;
                p_args_vhdl.set_option("--parser", std::vector<std::string>({"vhdl"}));
                p_args_vhdl.set_option("--gate-library", std::vector<std::string>({m_g_lib_path}));

                ProgramArguments p_args_verilog;
                p_args_verilog.set_option("--parser", std::vector<std::string>({"verilog"}));
                p_args_verilog.set_option("--gate-library", std::vector<std::string>({m_g_lib_path}));

                std::shared_ptr<Netlist> nl_vhdl = HDLParserDispatcher::parse(vhdl_file_without_extension, p_args_vhdl);
                std::shared_ptr<Netlist>
                    nl_verilog = HDLParserDispatcher::parse(verilog_file_without_extension, p_args_verilog);

                EXPECT_NE(nl_vhdl, nullptr);
                EXPECT_NE(nl_verilog, nullptr);
            }
            {
                // Parse a vhdl and a verilog file by passing no relevant program arguments. The parser should be determined
                // from the file extension ('.vhdl'/'.v') while the GateLibrary should be guessed.
                ProgramArguments p_args_empty;
                p_args_empty.set_option("--gate-library", std::vector<std::string>({m_g_lib_path}));

                std::shared_ptr<Netlist> nl_vhdl = HDLParserDispatcher::parse(vhdl_file_with_extension, p_args_empty);
                std::shared_ptr<Netlist>
                    nl_verilog = HDLParserDispatcher::parse(verilog_file_with_extension, p_args_empty);

                EXPECT_NE(nl_vhdl, nullptr);
                EXPECT_NE(nl_verilog, nullptr);
            }
            // NEGATIVE
            {
                // Pass an unknown parser
                ProgramArguments p_args_vhdl;
                p_args_vhdl.set_option("--parser", std::vector<std::string>({"verihdl"}));    // <- unknown
                p_args_vhdl.set_option("--gate-library", std::vector<std::string>({m_g_lib_path}));

                std::shared_ptr<Netlist> nl = HDLParserDispatcher::parse(vhdl_file_without_extension, p_args_vhdl);

                EXPECT_EQ(nl, nullptr);
            }
            {
                // Pass an unknown file path
                ProgramArguments p_args_vhdl;
                p_args_vhdl.set_option("--parser", std::vector<std::string>({"verilog"}));    // <- unknown
                p_args_vhdl.set_option("--gate-library", std::vector<std::string>({m_g_lib_path}));

                std::shared_ptr<Netlist>
                    nl = HDLParserDispatcher::parse(std::filesystem::path("this/path/does/not/exist.v"), p_args_vhdl);

                EXPECT_EQ(nl, nullptr);
            }
            {
                    // Pass an invalid file with no Gate-library hint
                    ProgramArguments p_args;
                    p_args.set_option("--parser", std::vector<std::string>({"verilog"}));
                    p_args.set_option("--gate-library", std::vector<std::string>({m_g_lib_path}));

                    std::shared_ptr<Netlist> nl = HDLParserDispatcher::parse(invalid_file, p_args);

                    EXPECT_EQ(nl, nullptr);
            }

        TEST_END
    }

    /**
     * Testing the parse function that takes a parser name and a Gate library explicitly
     *
     * Functions: parse
     */
    TEST_F(HDLParserDispatcherTest, check_parse_by_parser_name) {
        TEST_START
            // Create the files to parse
            std::filesystem::path vhdl_file_without_extension = test_utils::create_sandbox_file("tmp_vhdl_file", m_valid_vhdl_content);
            std::filesystem::path
                verilog_file_without_extension = test_utils::create_sandbox_file("tmp_verilog_file", m_valid_verilog_content);
            // Load a gate library
            std::shared_ptr<GateLibrary> min_gl = gate_library_manager::load_file(m_g_lib_path);
            {
                // Parse a vhdl and a verilog file by passing the parser name and the GateLibrary name. The file is passed by the std::filesystem::path.
                std::shared_ptr<Netlist>
                    nl_vhdl = HDLParserDispatcher::parse(min_gl, "vhdl", vhdl_file_without_extension);
                std::shared_ptr<Netlist> nl_verilog =
                    HDLParserDispatcher::parse(min_gl, "verilog", verilog_file_without_extension);

                EXPECT_NE(nl_vhdl, nullptr);
                EXPECT_NE(nl_verilog, nullptr);
            }
            {
                // Parse a vhdl and a verilog file by passing the parser name and the GateLibrary name. The file is passed by a string.
                std::shared_ptr<Netlist> nl_vhdl =
                    HDLParserDispatcher::parse(min_gl, "vhdl", vhdl_file_without_extension.string());
                std::shared_ptr<Netlist> nl_verilog = HDLParserDispatcher::parse(min_gl,
                                                                                 "verilog",
                                                                                 verilog_file_without_extension
                                                                                     .string());

                EXPECT_NE(nl_vhdl, nullptr);
                EXPECT_NE(nl_verilog, nullptr);
            }
            // NEGATIVE
            {
                // Pass an unknown file path
                std::shared_ptr<Netlist> nl = HDLParserDispatcher::parse(min_gl,
                                                                         "verilog",
                                                                         std::filesystem::path(
                                                                             "this/path/does/not/exist.v"));

                EXPECT_EQ(nl, nullptr);
            }
        TEST_END
    }
} //namespace hal
