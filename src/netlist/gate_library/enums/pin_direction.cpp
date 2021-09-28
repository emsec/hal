#include "hal_core/netlist/gate_library/enums/pin_direction.h"

namespace hal
{
    template<>
    std::vector<std::string> EnumStrings<PinDirection>::data = {"none", "input", "output", "inout", "internal"};
}