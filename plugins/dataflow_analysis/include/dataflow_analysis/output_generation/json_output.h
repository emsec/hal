#pragma once

#include "hal_core/defines.h"
#include "dataflow_analysis/evaluation/result.h"
#include "dataflow_analysis/output_generation/json.hpp"
#include "dataflow_analysis/processing/result.h"

namespace hal
{
    namespace json_output
    {
        void save_state_to_json(u32 iteration,
                                const NetlistAbstraction& netlist_abstr,
                                const processing::Result& processing_result,
                                const evaluation::Result& eval_result,
                                bool with_gates,
                                nlohmann::json& j);
    }    // namespace json_output
}    // namespace hal
