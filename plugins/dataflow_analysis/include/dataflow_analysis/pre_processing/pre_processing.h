#pragma once

#include "dataflow_analysis/common/netlist_abstraction.h"

namespace hal
{
    class Netlist;

    namespace dataflow
    {
        namespace pre_processing
        {
            NetlistAbstraction run(Netlist* netlist);
        }    // namespace pre_processing
    }        // namespace dataflow
}    // namespace hal