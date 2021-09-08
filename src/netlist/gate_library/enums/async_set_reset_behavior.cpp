#include "hal_core/netlist/gate_library/enums/async_set_reset_behavior.h"

namespace hal
{
    template<>
    std::vector<std::string> EnumStrings<AsyncSetResetBehavior>::data = {"L", "H", "N", "T", "X", "undef"};
}