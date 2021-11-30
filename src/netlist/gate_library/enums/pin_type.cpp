#include "hal_core/netlist/gate_library/enums/pin_type.h"

namespace hal
{
    template<>
    std::vector<std::string> EnumStrings<PinType>::data = {"none", "power", "ground", "lut", "state", "neg_state", "clock", "enable", "set", "reset", "data", "address", "io_pad", "select"};
}