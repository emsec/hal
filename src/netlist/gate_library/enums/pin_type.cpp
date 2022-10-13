#include "hal_core/netlist/gate_library/enums/pin_type.h"

namespace hal
{
    template<>
    std::map<PinType, std::string> EnumStrings<PinType>::data = {{PinType::none, "none"},
                                                                 {PinType::power, "power"},
                                                                 {PinType::ground, "ground"},
                                                                 {PinType::lut, "lut"},
                                                                 {PinType::state, "state"},
                                                                 {PinType::neg_state, "neg_state"},
                                                                 {PinType::clock, "clock"},
                                                                 {PinType::enable, "enable"},
                                                                 {PinType::set, "set"},
                                                                 {PinType::reset, "reset"},
                                                                 {PinType::data, "data"},
                                                                 {PinType::address, "address"},
                                                                 {PinType::io_pad, "io_pad"},
                                                                 {PinType::select, "select"}};
}