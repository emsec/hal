#pragma once

#include "utils/utils.h"

namespace hal
{
    namespace dataflow_utils
    {
        class UtilsXilinxUnisim : public Utils
        {
        public:
            UtilsXilinxUnisim()          = default;
            virtual ~UtilsXilinxUnisim() = default;

            /* library specific functions */
            bool is_sequential(const std::shared_ptr<Gate>& sg) const override;

            std::unordered_set<std::string> get_control_input_pin_types(const std::shared_ptr<Gate>& sg) const override;

            std::unordered_set<std::string> get_clock_ports(const std::shared_ptr<Gate>&) const override;
            std::unordered_set<std::string> get_enable_ports(const std::shared_ptr<Gate>&) const override;
            std::unordered_set<std::string> get_reset_ports(const std::shared_ptr<Gate>&) const override;
            std::unordered_set<std::string> get_data_ports(const std::shared_ptr<Gate>&) const override;
        };
    }    // namespace dataflow_utils
}    // namespace hal