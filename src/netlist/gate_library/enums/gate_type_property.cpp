#include "hal_core/netlist/gate_library/enums/gate_type_property.h"

namespace hal
{
    template<>
    std::map<GateTypeProperty, std::string> EnumStrings<GateTypeProperty>::data = {{GateTypeProperty::combinational, "combinational"},
                                                                                   {GateTypeProperty::sequential, "sequential"},
                                                                                   {GateTypeProperty::tristate, "tristate"},
                                                                                   {GateTypeProperty::power, "power"},
                                                                                   {GateTypeProperty::ground, "ground"},
                                                                                   {GateTypeProperty::ff, "ff"},
                                                                                   {GateTypeProperty::latch, "latch"},
                                                                                   {GateTypeProperty::ram, "ram"},
                                                                                   {GateTypeProperty::io, "io"},
                                                                                   {GateTypeProperty::dsp, "dsp"},
                                                                                   {GateTypeProperty::pll, "pll"},
                                                                                   {GateTypeProperty::oscillator, "oscillator"},
                                                                                   {GateTypeProperty::scan, "scan"},
                                                                                   {GateTypeProperty::c_buffer, "c_buffer"},
                                                                                   {GateTypeProperty::c_inverter, "c_inverter"},
                                                                                   {GateTypeProperty::c_and, "c_and"},
                                                                                   {GateTypeProperty::c_nand, "c_nand"},
                                                                                   {GateTypeProperty::c_or, "c_or"},
                                                                                   {GateTypeProperty::c_nor, "c_nor"},
                                                                                   {GateTypeProperty::c_xor, "c_xor"},
                                                                                   {GateTypeProperty::c_xnor, "c_xnor"},
                                                                                   {GateTypeProperty::c_aoi, "c_aoi"},
                                                                                   {GateTypeProperty::c_oai, "c_oai"},
                                                                                   {GateTypeProperty::c_mux, "c_mux"},
                                                                                   {GateTypeProperty::c_carry, "c_carry"},
                                                                                   {GateTypeProperty::c_half_adder, "c_half_adder"},
                                                                                   {GateTypeProperty::c_full_adder, "c_full_adder"},
                                                                                   {GateTypeProperty::c_lut, "c_lut"}};
}