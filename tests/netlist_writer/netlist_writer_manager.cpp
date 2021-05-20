#include "hal_core/netlist/netlist_writer/netlist_writer_manager.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist_parser/netlist_parser_manager.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/netlist.h"
#include "netlist_test_utils.h"
#include "gate_library_test_utils.h"
#include "hal_core/plugin_system/plugin_manager.h"

#include "gtest/gtest.h"
#include "hal_core/utilities/program_arguments.h"
#include <experimental/filesystem>

namespace hal {

    class hdl_writer_managerTest : public ::testing::Test {
    protected:
        const GateLibrary* m_gl;

        virtual void SetUp() 
        {
            NO_COUT_BLOCK;
            test_utils::init_log_channels();
            test_utils::create_sandbox_directory();
            plugin_manager::load_all_plugins();
            
            // gate library needs to be registered through gate_library_manager for serialization
            std::unique_ptr<GateLibrary> gl_tmp = test_utils::create_gate_library(test_utils::create_sandbox_path("testing_gate_library.hgl"));
            gate_library_manager::save(gl_tmp->get_path(), gl_tmp.get(), true);
            m_gl = gate_library_manager::load(gl_tmp->get_path());
        }

        virtual void TearDown() 
        {
            NO_COUT_BLOCK;
            plugin_manager::unload_all_plugins();
            test_utils::remove_sandbox_directory();    
        }

        // TODO replace by new example netlist as soon as ready
        std::unique_ptr<Netlist> create_simple_netlist() 
        {
            auto nl = std::make_unique<Netlist>(m_gl);
            nl->set_design_name("top_module_type");
            nl->get_top_module()->set_type("top_module_type");
            Gate* gate_0 = nl->create_gate(m_gl->get_gate_type_by_name("BUF"), "gate_0");
            Net* g_in_net = nl->create_net("global_in");
            Net* g_out_net = nl->create_net("global_out");
            nl->mark_global_input_net(g_in_net);
            nl->mark_global_output_net(g_out_net);
            g_in_net->add_destination(gate_0, "I");
            g_out_net->add_source(gate_0, "O");

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
                EXPECT_FALSE(netlist_writer_manager::get_cli_options().get_options().empty());
            }
        TEST_END
    }

    /**
     * Test writing a netlist using program arguments.
     *
     * Functions: write
     */
    TEST_F(hdl_writer_managerTest, check_write) {
        TEST_START
            {
                // pass by program arguments
                std::filesystem::path out_path_verilog = test_utils::create_sandbox_path("writer_out_verilog.v");
                ProgramArguments p_args_verilog;
                p_args_verilog.set_option("--write-hdl", std::vector<std::string>({out_path_verilog.string()}));

                auto simple_nl = create_simple_netlist();
                ASSERT_TRUE(netlist_writer_manager::write(simple_nl.get(), p_args_verilog));
                ASSERT_NE(netlist_parser_manager::parse(out_path_verilog, m_gl), nullptr);
            }
            {
                // pass by file name
                std::filesystem::path out_path_verilog = test_utils::create_sandbox_path("writer_out_verilog.v");

                auto simple_nl = create_simple_netlist();
                ASSERT_TRUE(netlist_writer_manager::write(simple_nl.get(), out_path_verilog));
                ASSERT_NE(netlist_parser_manager::parse(out_path_verilog, m_gl), nullptr);
            }
            // NEGATIVE
            {
                // unknown file format
                std::filesystem::path out_path = test_utils::create_sandbox_path("unknown_format.INVALID");

                auto simple_nl = create_simple_netlist();
                EXPECT_FALSE(netlist_writer_manager::write(simple_nl.get(), out_path));
            }
        TEST_END
    }
} //namespace hal
