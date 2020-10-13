#pragma once

#include "hal_core/defines.h"

#include <map>
#include <unordered_set>

namespace hal
{
    namespace dataflow
    {
        /* forward declaration */
        struct Grouping;
        struct NetlistAbstraction;
        struct Context;

        namespace scoring
        {
            std::shared_ptr<Grouping> scoring(Context& eval_context, const std::shared_ptr<const NetlistAbstraction>& m_context);
        }    // namespace scoring
    }        // namespace dataflow
}