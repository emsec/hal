#pragma once

#include "dataflow_analysis/utils/utils.h"

namespace hal
{
    namespace dataflow
    {
        namespace dataflow_utils
        {
            class UtilsXilinxSimprim : public Utils
            {
            public:
                UtilsXilinxSimprim()          = default;
                virtual ~UtilsXilinxSimprim() = default;

                /* library specific functions */
                bool is_sequential(Gate* sg) const override;

                std::unordered_set<std::string> get_control_input_pin_types(Gate* sg) const override;

                std::unordered_set<std::string> get_clock_ports(Gate*) const override;
                std::unordered_set<std::string> get_enable_ports(Gate*) const override;
                std::unordered_set<std::string> get_reset_ports(Gate*) const override;
                std::unordered_set<std::string> get_data_ports(Gate*) const override;
            };
        }    // namespace dataflow_utils
    }        // namespace dataflow
}    // namespace hal
