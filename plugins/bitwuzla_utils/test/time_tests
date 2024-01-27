#include "hal_core/defines.h"
#include "hal_core/netlist/decorators/subgraph_netlist_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/netlist/netlist_parser/netlist_parser_manager.h"
#include "hal_core/netlist/netlist_utils.h"
#include "hal_core/netlist/persistent/netlist_serializer.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/utilities/log.h"
#include "netlist_test_utils.h"

#include <fstream>
#include <set>
#include <string>
#include <chrono>

namespace hal
{
    class ModuelIdentificationTest : public ::testing::Test
    {
    protected:
        ModuleIdentificationPlugin* plugin;
        GateLibrary* xilinx_lib;
        GateLibrary* lattice_lib;
        GateLibrary* nangate_lib;

        virtual void SetUp()
        {
            NO_COUT_BLOCK;
            plugin_manager::load_all_plugins();
            //gate_library_manager::get_gate_library("XILINX_UNISIM.hgl");
            gate_library_manager::get_gate_library("XILINX_UNISIM_hal.hgl");
            gate_library_manager::get_gate_library("ice40ultra_hal.hgl");
            gate_library_manager::get_gate_library("NangateOpenCellLibrary.hgl");

            xilinx_lib  = gate_library_manager::get_gate_library_by_name("XILINX_UNISIM_WITH_HAL_TYPES");
            lattice_lib = gate_library_manager::get_gate_library_by_name("ICE40ULTRA_WITH_HAL_TYPES");
            nangate_lib = gate_library_manager::get_gate_library_by_name("NangateOpenCellLibrary");
            plugin      = plugin_manager::get_plugin_instance<ModuleIdentificationPlugin>("module_identification");
        }

        std::unique_ptr<Netlist> parse_netlist(std::string netlist_path, GateLibrary* lib)
        {
            if (!utils::file_exists(netlist_path))
            {
                std::cout << "netlist not found: " << netlist_path << std::endl;
                return nullptr;
            }

            std::cout << "[+] new netlist \nloading " << netlist_path << std::endl;
            std::unique_ptr<Netlist> nl;
            {
                // nl = netlist_parser_manager::parse(netlist_path, lib);
                nl = netlist_factory::load_netlist(netlist_path, lib);
                if (nl == nullptr)
                {
                    std::cout << "netlist couldn't be parsed" << std::endl;
                    return nullptr;
                }
            }
            std::cout << "[+] loaded_netlist with " << nl->get_gates().size() << std::endl;

            return std::move(nl);
        }

        virtual void TearDown()
        {
            NO_COUT_BLOCK;
            plugin_manager::unload_all_plugins();
        }
    };

    /**
     * Test verification of base test  for Xilinx
     *
     * Functions: module_identification
     */
    TEST_F(ModuelIdentificationTest, xilinx_ibex){TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/xilinx/ibex.hal";

    std::unique_ptr<Netlist> nl;
    {
        // NO_COUT_BLOCK;
        nl = parse_netlist(netlist_path, xilinx_lib);
    }
    if (nl == nullptr)
    {
        FAIL() << "netlist couldn't be parsed";
    }
    std::vector<BooleanFunction> bf_vector;
    std::vector<z3::expr> z3_vector;
    std::vector<bw::expr> bw_vector;
    
    auto gates = nl->get_gates();
    auto all_out_nets = nl->get_global_output_nets ();
    auto z3_cfg = z3::Z3_mk_config();
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for(auto net: all_out_nets)
    {
        
        auto z3 = z3::get_subgraph_z3_function(gates, net, z3_cfg);
        z3_vector.push_back(z3.get());
        
    }
    end = std::chrono::steady_clock::now();
    std::cout << "Time difference z_3 = " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << "[s]" << std::endl;
    auto dec = SubgraphNetlistDecorator(nl);
    begin = std::chrono::steady_clock::now();
    for(auto net: all_out_nets)
    {
        
        
        auto bf =  dec.get_subgraph_function ( gates, net) 	;
        bf_vector.push_back(bf.get());

        
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time difference BF = " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << "[s]" << std::endl;
    begin = std::chrono::steady_clock::now();
    for(auto net: all_out_nets)
    {
        
        auto bw = bw::get_subgraph_bw_function(gates, net);
        bw_vector.push_back(bw.get());

        
    }
    end = std::chrono::steady_clock::now();
    std::cout << "Time difference bw  = " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << "[s]" << std::endl;
    std::cout << std::endl;

}    // namespace hal
TEST_END
}
;