#include "hal_core/netlist/gate_library/enums/pin_direction.h"

namespace hal
{
    template<>
    std::map<PinDirection, std::string> EnumStrings<PinDirection>::data = {{PinDirection::none, "none"},
                                                                           {PinDirection::input, "input"},
                                                                           {PinDirection::output, "output"},
                                                                           {PinDirection::inout, "inout"},
                                                                           {PinDirection::internal, "internal"}};
}