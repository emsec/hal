#include "netlist_test_utils.h"
#include "netlist/netlist.h"
#include "gtest/gtest.h"
#include "netlist/hdl_parser/hdl_parser_dispatcher.h"
#include <sstream>
#include <experimental/filesystem>
#include <core/program_arguments.h>


using namespace test_utils;

class hdl_parser_dispatcher_test : public ::testing::Test
{
protected:
    hal::path tmp_dir;
    std::string valid_verilog_content = "module top (\n"
                                        "  net_global_in,\n"
                                        "  net_global_out\n"
                                        " ) ;\n"
                                        "  input net_global_in ;\n"
                                        "  output net_global_out ;\n"
                                        "INV gate_0 (\n"
                                        "  .\\I (net_global_in ),\n"
                                        "  .\\O (net_global_out)\n"
                                        " ) ;\n"
                                        "endmodule";

    std::string valid_vhdl_content    = "-- Device\t: device_name\n"
                                        "entity TEST_Comp is\n"
                                        "  port (\n"
                                        "    net_global_in : in STD_LOGIC := 'X';\n"
                                        "    net_global_out : out STD_LOGIC := 'X';\n"
                                        "  );\n"
                                        "end TEST_Comp;\n"
                                        "\n"
                                        "architecture STRUCTURE of TEST_Comp is\n"
                                        "begin\n"
                                        "  gate_0 : INV\n"
                                        "    port map (\n"
                                        "      I => net_global_in,\n"
                                        "      O => net_global_out\n"
                                        "    );\n"
                                        "end STRUCTURE;";

    // std::string invalid_input         = "(#+;$: INVALID INPUT :$;+#(";
    std::string invalid_input         = "(+;$: INVALID INPUT :$;+(";

    virtual void SetUp()
    {
        NO_COUT_BLOCK;
        gate_library_manager::load_all();
        tmp_dir = core_utils::get_base_directory() / "tests/sandbox_directory";
        fs::remove_all(tmp_dir);
    }

    virtual void TearDown()
    {
        fs::remove_all(tmp_dir);
    }

    // Creates a temporary file within a specific directory that is removed at the end of the test fixture
    hal::path create_tmp_file(std::string file_name, std::string content){
        fs::create_directory(tmp_dir);
        hal::path path_in_dir = file_name;
        hal::path full_path = tmp_dir / path_in_dir;
        std::ofstream tmp_file( full_path.string() );
        tmp_file << content;
        tmp_file.close();
        return full_path;
    }

};

/**
 * Testing the access on usage information: the cli-options and the gui-options
 *
 * Functions: get_cli_options, get_gui_option
 */
TEST_F(hdl_parser_dispatcher_test, check_cli_gui_options)
{
    TEST_START
        {
            // Access the gui-options and the cli-options (shouldn't  be empty)
            EXPECT_FALSE(hdl_parser_dispatcher::get_cli_options().get_options().empty());
            EXPECT_FALSE(hdl_parser_dispatcher::get_gui_option().empty());
        }
    TEST_END
}

/**
 * Testing the parse function that takes program_arguments as input.
 *
 * Functions: parse
 */
TEST_F(hdl_parser_dispatcher_test, check_parse_by_program_args)
{
    TEST_START
        hal::path vhdl_file_with_extension = create_tmp_file("tmp_vhdl_file.vhdl", valid_vhdl_content);
        hal::path verilog_file_with_extension = create_tmp_file("tmp_verilog_file.v", valid_verilog_content);
        hal::path vhdl_file_without_extension = create_tmp_file("tmp_vhdl_file", valid_vhdl_content);
        hal::path verilog_file_without_extension = create_tmp_file("tmp_verilog_file", valid_verilog_content);
        hal::path invalid_file = create_tmp_file("invalid_file", invalid_input);

        {
            // Parse a vhdl and a verilog file by passing the parser name and the gate_library name in the program arguments.
            program_arguments p_args_vhdl;
            p_args_vhdl.set_option("--parser", std::vector<std::string>({"vhdl"}));
            p_args_vhdl.set_option("--gate-library", std::vector<std::string>({"EXAMPLE_GATE_LIBRARY"}));

            program_arguments p_args_verilog;
            p_args_verilog.set_option("--parser", std::vector<std::string>({"verilog"}));
            p_args_verilog.set_option("--gate-library", std::vector<std::string>({"EXAMPLE_GATE_LIBRARY"}));

            std::shared_ptr<netlist> nl_vhdl = hdl_parser_dispatcher::parse(vhdl_file_without_extension, p_args_vhdl);
            std::shared_ptr<netlist> nl_verilog = hdl_parser_dispatcher::parse(verilog_file_without_extension, p_args_verilog);

            EXPECT_NE(nl_vhdl, nullptr);
            EXPECT_NE(nl_verilog, nullptr);
        }
        {
            // Parse a vhdl and a verilog file by passing no relevant program arguments. The parser should be determined
            // from the file extension ('.vhdl'/'.v') while the gate_library should be guessed.
            program_arguments p_args_empty;

            std::shared_ptr<netlist> nl_vhdl = hdl_parser_dispatcher::parse(vhdl_file_with_extension, p_args_empty);
            std::shared_ptr<netlist> nl_verilog = hdl_parser_dispatcher::parse(verilog_file_with_extension, p_args_empty);

            EXPECT_NE(nl_vhdl, nullptr);
            EXPECT_NE(nl_verilog, nullptr);
        }
        // NEGATIVE
        {
            // Pass an unknown parser
            program_arguments p_args_vhdl;
            p_args_vhdl.set_option("--parser", std::vector<std::string>({"verihdl"})); // <- unknown
            p_args_vhdl.set_option("--gate-library", std::vector<std::string>({"EXAMPLE_GATE_LIBRARY"}));

            std::shared_ptr<netlist> nl = hdl_parser_dispatcher::parse(vhdl_file_without_extension, p_args_vhdl);

            EXPECT_EQ(nl, nullptr);
        }
        {
            // Pass an unknown file path
            program_arguments p_args_vhdl;
            p_args_vhdl.set_option("--parser", std::vector<std::string>({"verilog"})); // <- unknown
            p_args_vhdl.set_option("--gate-library", std::vector<std::string>({"EXAMPLE_GATE_LIBRARY"}));

            std::shared_ptr<netlist> nl = hdl_parser_dispatcher::parse(hal::path("this/path/does/not/exist.v"), p_args_vhdl);

            EXPECT_EQ(nl, nullptr);
        }
        /*{ // ISSUE: Some issue in the verilog tokenizer (also vhdl?).
            // (l.189: if(... && parsed_tokens.back() == '#'), parsed_tokens can be empty)
            // Pass an invalid file with no gate-library hint
            program_arguments p_args;
            p_args.set_option("--parser", std::vector<std::string>({"verilog"}));

            std::shared_ptr<netlist> nl = hdl_parser_dispatcher::parse(invalid_file, p_args);

            EXPECT_EQ(nl, nullptr);
        }*/

    TEST_END
}


/**
 * Testing the parse function that takes a parser name and a gate library explicitly
 *
 * Functions: parse
 */
TEST_F(hdl_parser_dispatcher_test, check_parse_by_parser_name)
{
    TEST_START
        hal::path vhdl_file_without_extension = create_tmp_file("tmp_vhdl_file", valid_vhdl_content);
        hal::path verilog_file_without_extension = create_tmp_file("tmp_verilog_file", valid_verilog_content);

        {
            // Parse a vhdl and a verilog file by passing the parser name and the gate_library name. The file is passed by the hal::path.
            std::shared_ptr<netlist> nl_vhdl = hdl_parser_dispatcher::parse("EXAMPLE_GATE_LIBRARY", "vhdl", vhdl_file_without_extension);
            std::shared_ptr<netlist> nl_verilog = hdl_parser_dispatcher::parse("EXAMPLE_GATE_LIBRARY", "verilog", verilog_file_without_extension);

            EXPECT_NE(nl_vhdl, nullptr);
            EXPECT_NE(nl_verilog, nullptr);
        }
        {
            // Parse a vhdl and a verilog file by passing the parser name and the gate_library name. The file is passed by a string.
            std::shared_ptr<netlist> nl_vhdl = hdl_parser_dispatcher::parse("EXAMPLE_GATE_LIBRARY", "vhdl", vhdl_file_without_extension.string());
            std::shared_ptr<netlist> nl_verilog = hdl_parser_dispatcher::parse("EXAMPLE_GATE_LIBRARY", "verilog", verilog_file_without_extension.string());

            EXPECT_NE(nl_vhdl, nullptr);
            EXPECT_NE(nl_verilog, nullptr);
        }
        // NEGATIVE
        {
            // Pass an unknown file path
            std::shared_ptr<netlist> nl = hdl_parser_dispatcher::parse("EXAMPLE_GATE_LIBRARY", "verilog", hal::path("this/path/does/not/exist.v"));

            EXPECT_EQ(nl, nullptr);
        }
    TEST_END
}
