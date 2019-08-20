#include "plugin_gate_decorators.h"

#include "gate_decorator_system/decorators/gate_decorator_bdd.h"
#include "gate_decorator_system/decorators/gate_decorator_lut.h"
#include "gate_decorator_system/gate_decorator_system.h"

#include "core/log.h"
#include "core/utils.h"

#include "netlist/gate.h"


std::string plugin_gate_decorators::get_name()
{
    return std::string("gate_decorators");
}

std::string plugin_gate_decorators::get_version()
{
    return std::string("1.0");
}

void plugin_gate_decorators::on_load()
{
    gate_decorator_system::register_bdd_decorator_function("GSCLIB_3_0", &bdd_availability_tester_gsclib, &bdd_generator_gsclib);
    gate_decorator_system::register_bdd_decorator_function("SYNOPSYS_90NM", &bdd_availability_tester_synopsys90, &bdd_generator_synopsys90);
    gate_decorator_system::register_bdd_decorator_function("YOSYS_MYCELL", &bdd_availability_tester_yosys_mycell, &bdd_generator_yosys_mycell);
    gate_decorator_system::register_bdd_decorator_function("MYCELL", &bdd_availability_tester_mycell, &bdd_generator_mycell);

    gate_decorator_system::register_bdd_decorator_function("SCAN_FF_LIB", &bdd_availability_tester_scan_ff_lib, &bdd_generator_scan_ff_lib);

    gate_decorator_system::register_bdd_decorator_function("XILINX_SIMPRIM", &bdd_availability_tester_xilinx_simprim, &bdd_generator_xilinx_simprim);
    gate_decorator_system::register_lut_decorator_function("XILINX_SIMPRIM", &lut_availability_tester_xilinx_simprim, &lut_generator_xilinx_simprim);

    gate_decorator_system::register_bdd_decorator_function("XILINX_UNISIM", &bdd_availability_tester_xilinx_unisim, &bdd_generator_xilinx_unisim);
    gate_decorator_system::register_lut_decorator_function("XILINX_UNISIM", &lut_availability_tester_xilinx_unisim, &lut_generator_xilinx_unisim);
}

void plugin_gate_decorators::on_unload()
{
    gate_decorator_system::remove_bdd_decorator_function("GSCLIB_3_0");
    gate_decorator_system::remove_bdd_decorator_function("SYNOPSYS_90NM");
    gate_decorator_system::remove_bdd_decorator_function("YOSYS_MYCELL");
    gate_decorator_system::remove_bdd_decorator_function("MYCELL");

    gate_decorator_system::remove_bdd_decorator_function("SCAN_FF_LIB");


    gate_decorator_system::remove_bdd_decorator_function("XILINX_SIMPRIM");
    gate_decorator_system::remove_lut_decorator_function("XILINX_SIMPRIM");

    gate_decorator_system::remove_bdd_decorator_function("XILINX_UNISIM");
    gate_decorator_system::remove_lut_decorator_function("XILINX_UNISIM");
}
