#pragma once

#include "utils/utils.h"

namespace hal
{
    namespace gate_library_specific_utils
    {
        class UtilsNangate : public Utils
        {
        public:
            UtilsNangate()          = default;
            virtual ~UtilsNangate() = default;

            /* library specific functions */
            bool is_sequential(GateType* g) const override;

            std::unordered_set<std::string> get_control_input_pin_types(GateType* g) const override;

            std::unordered_set<std::string> get_clock_ports(GateType* g) const override;
            std::unordered_set<std::string> get_enable_ports(GateType* g) const override;
            std::unordered_set<std::string> get_reset_ports(GateType* g) const override;
            std::unordered_set<std::string> get_set_ports(GateType* g) const override;
            std::unordered_set<std::string> get_data_ports(GateType* g) const override;
            std::unordered_set<std::string> get_regular_outputs(GateType* g) const override;
            std::unordered_set<std::string> get_negated_outputs(GateType* g) const override;
        };
    }    // namespace gate_library_specific_utils
}    // namespace hal
