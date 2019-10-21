#ifndef HAL_PLUGIN_GATE_DECORATORS_H
#define HAL_PLUGIN_GATE_DECORATORS_H

#include "core/interface_base.h"
#include <map>
#include <memory>
#include <string>

class bdd;
class gate;

class PLUGIN_API plugin_gate_decorators : virtual public i_base
{
public:
    std::string get_name() override;

    std::string get_version() override;

    void on_load() override;

    void on_unload() override;

private:
    static std::map<std::string, std::shared_ptr<bdd>> bdd_generator_gsclib(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd);
    static bool bdd_availability_tester_gsclib(std::shared_ptr<gate> g);

    static std::map<std::string, std::shared_ptr<bdd>> bdd_generator_synopsys90(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd);
    static bool bdd_availability_tester_synopsys90(std::shared_ptr<gate> g);

    static std::map<std::string, std::shared_ptr<bdd>> bdd_generator_yosys_mycell(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd);
    static bool bdd_availability_tester_yosys_mycell(std::shared_ptr<gate> g);

    static std::map<std::string, std::shared_ptr<bdd>> bdd_generator_synopsys_nand_nor(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd);
    static bool bdd_availability_tester_synopsys_nand_nor(std::shared_ptr<gate> g);

    static std::map<std::string, std::shared_ptr<bdd>> bdd_generator_NangateOpenCellLibrary(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd);
    static bool bdd_availability_tester_NangateOpenCellLibrary(std::shared_ptr<gate> g);

    static std::map<std::string, std::shared_ptr<bdd>> bdd_generator_xilinx_simprim(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd);
    static bool bdd_availability_tester_xilinx_simprim(std::shared_ptr<gate> g);
    static std::tuple<u32, u32, std::string> lut_generator_xilinx_simprim(std::shared_ptr<gate> g);
    static bool lut_availability_tester_xilinx_simprim(std::shared_ptr<gate> g);

    static std::map<std::string, std::shared_ptr<bdd>> bdd_generator_xilinx_unisim(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd);
    static bool bdd_availability_tester_xilinx_unisim(std::shared_ptr<gate> g);
    static std::tuple<u32, u32, std::string> lut_generator_xilinx_unisim(std::shared_ptr<gate> g);
    static bool lut_availability_tester_xilinx_unisim(std::shared_ptr<gate> g);
};

#endif
