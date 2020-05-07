#include "netlist_test_utils.h"
#include "netlist/gate.h"
#include "netlist/netlist.h"
#include "gtest/gtest.h"
#include "netlist/hdl_writer/hdl_writer_dispatcher.h"
#include "netlist/hdl_parser/hdl_parser_dispatcher.h"
#include <experimental/filesystem>
#include <core/program_arguments.h>


using namespace test_utils;

class hdl_writer_dispatcher_test : public ::testing::Test
{
protected:
    hal::path tmp_dir;

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

    // Creates a unique file path in the temporary directory with a custom extension
    hal::path get_tmp_file_path(std::string extension = ""){
        static unsigned int file_id = 0;
        fs::create_directory(tmp_dir);
        return (tmp_dir / hal::path("tmp_file_" + std::to_string(file_id++) + extension));
    }

    // Creates the following netlist:     global_in ---= INV =--- global_out
    std::shared_ptr<netlist> create_simple_netlist() {
        std::shared_ptr<netlist> nl = create_empty_netlist();
        nl->set_design_name("top_module");
        nl->get_top_module()->set_name("top_module");
        std::shared_ptr<gate> inv_gate = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("INV"), "inv_gate");
        std::shared_ptr<net> g_in_net =  nl->create_net("global_in");
        std::shared_ptr<net> g_out_net = nl->create_net("global_out");
        nl->mark_global_input_net(g_in_net);
        nl->mark_global_output_net(g_out_net);
        g_in_net->add_dst(inv_gate, "I");
        g_out_net->set_src(inv_gate, "O");

        return nl;
    }
};

/**
 * Testing the access on usage information: the cli-options
 *
 * Functions: get_cli_options
 */
TEST_F(hdl_writer_dispatcher_test, check_cli_options)
{
    TEST_START
        {
            // Access the gui-options and the cli-options (shouldn't  be empty)
            EXPECT_FALSE(hdl_writer_dispatcher::get_cli_options().get_options().empty());
        }
    TEST_END
}

/**
 * Testing writing of a netlist using program arguments
 *
 * Functions: get_cli_options
 */
TEST_F(hdl_writer_dispatcher_test, check_write_by_program_args)
{
    TEST_START
        {
            // Write a netlist to two files, by passing the file path in the arguments
            std::string out_path_vhdl = get_tmp_file_path(".vhd").string();
            std::string out_path_verilog = get_tmp_file_path(".v").string();
            program_arguments p_args_vhdl;
            p_args_vhdl.set_option("--write-vhdl", std::vector<std::string>({out_path_vhdl}));
            p_args_vhdl.set_option("--write-verilog", std::vector<std::string>({out_path_verilog}));


            std::shared_ptr<netlist> simple_nl = create_simple_netlist();
            // Write the two files
            bool suc = hdl_writer_dispatcher::write(simple_nl, p_args_vhdl);
            EXPECT_TRUE(suc);

            // Verify the correctness of the output by parsing it
            std::shared_ptr<netlist> parsed_nl_vhdl = hdl_parser_dispatcher::parse("EXAMPLE_GATE_LIBRARY", "vhdl", out_path_vhdl);
            std::shared_ptr<netlist> parsed_nl_verilog = hdl_parser_dispatcher::parse("EXAMPLE_GATE_LIBRARY", "verilog", out_path_verilog);

            EXPECT_TRUE(test_utils::netlists_are_equal(parsed_nl_vhdl, simple_nl, true));
            // Since the verilog writer adds a "_inst" suffix to all gate names, it must
            // be added to the simple netlist for comparison
            for(auto g : simple_nl->get_gates()) g->set_name(g->get_name() + "_inst");

            EXPECT_TRUE(test_utils::netlists_are_equal(parsed_nl_verilog, simple_nl, true));
        }
    TEST_END
}

/**
 * Testing writing of a netlist by passing a format and a file name
 *
 * Functions: get_cli_options
 */
TEST_F(hdl_writer_dispatcher_test, check_write_by_format_and_filename)
{
    TEST_START
        {
            // Write a netlist to two files, by passing the file path in the arguments
            hal::path out_path_vhdl = get_tmp_file_path(".vhd");
            hal::path out_path_verilog = get_tmp_file_path(".v");

            std::shared_ptr<netlist> simple_nl = create_simple_netlist();
            // Write the two files
            bool suc_vhdl = hdl_writer_dispatcher::write(simple_nl, "vhdl", out_path_vhdl);
            bool suc_verilog = hdl_writer_dispatcher::write(simple_nl, "verilog", out_path_verilog);
            EXPECT_TRUE(suc_vhdl);
            EXPECT_TRUE(suc_verilog);

            // Verify the correctness of the output by parsing it
            std::shared_ptr<netlist> parsed_nl_vhdl = hdl_parser_dispatcher::parse("EXAMPLE_GATE_LIBRARY", "vhdl", out_path_vhdl);
            std::shared_ptr<netlist> parsed_nl_verilog = hdl_parser_dispatcher::parse("EXAMPLE_GATE_LIBRARY", "verilog", out_path_verilog);

            EXPECT_TRUE(test_utils::netlists_are_equal(parsed_nl_vhdl, simple_nl, true));
            // Since the verilog writer adds a "_inst" suffix to all gate names, it must
            // be added to the simple netlist for comparison
            for(auto g : simple_nl->get_gates()) g->set_name(g->get_name() + "_inst");

            EXPECT_TRUE(test_utils::netlists_are_equal(parsed_nl_verilog, simple_nl, true));
        }
        // NEAGTIVE
        {
            // The format is unknown
            hal::path out_path = get_tmp_file_path(".txt");

            std::shared_ptr<netlist> simple_nl = create_simple_netlist();
            // Write the two files
            EXPECT_FALSE( hdl_writer_dispatcher::write(simple_nl, "<unknown_format>", out_path) );
        }
    TEST_END
}