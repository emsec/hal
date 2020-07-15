#pragma once

#include "common/netlist_abstraction.h"

namespace hal
{
    class Netlist;

    namespace pre_processing
    {
        NetlistAbstraction run(std::shared_ptr<Netlist> netlist);
    }    // namespace pre_processing
}    // namespace hal