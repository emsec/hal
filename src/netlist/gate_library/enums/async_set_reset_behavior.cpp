#include "hal_core/netlist/gate_library/enums/async_set_reset_behavior.h"

namespace hal
{
    template<>
    std::map<AsyncSetResetBehavior, std::string> EnumStrings<AsyncSetResetBehavior>::data = {{AsyncSetResetBehavior::L, "L"},
                                                                                             {AsyncSetResetBehavior::H, "H"},
                                                                                             {AsyncSetResetBehavior::N, "N"},
                                                                                             {AsyncSetResetBehavior::T, "T"},
                                                                                             {AsyncSetResetBehavior::X, "X"},
                                                                                             {AsyncSetResetBehavior::undef, "undef"}};
}