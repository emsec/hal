#include "dataflow_analysis/utils/utils_xilinx_simprim.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/utilities/log.h"

#include <string>
#include <unordered_set>

namespace hal
{
    namespace dataflow
    {
        namespace dataflow_utils
        {
            bool UtilsXilinxSimprim::is_sequential(Gate* sg) const
            {
                static std::unordered_set<std::string> supported = {"X_FF"};

                if (supported.find(sg->get_type()->get_name()) != supported.end())
                {
                    return true;
                }
                return false;
            }

            std::unordered_set<std::string> UtilsXilinxSimprim::get_control_input_pin_types(Gate* sg) const
            {
                auto data_ports = get_data_ports(sg);
                std::unordered_set<std::string> control_input_pin_types;
                for (const auto& pin_type : sg->get_input_pins())
                {
                    if (data_ports.find(pin_type) != data_ports.end())
                        continue;

                    control_input_pin_types.insert(pin_type);
                }
                return control_input_pin_types;
            }

            std::unordered_set<std::string> UtilsXilinxSimprim::get_clock_ports(Gate* sg) const
            {
                if (!is_sequential(sg))
                {
                    log_error("dataflow", "gate is not sequential: {}, type: {}", sg->get_name(), sg->get_type()->get_name());
                    return std::unordered_set<std::string>();
                }

                static std::map<std::string, std::unordered_set<std::string>> gate_to_clock_ports;
                if (gate_to_clock_ports.empty())
                {
                    gate_to_clock_ports["X_FF"] = {"CLK"};
                }

                return gate_to_clock_ports.at(sg->get_type()->get_name());
            }

            std::unordered_set<std::string> UtilsXilinxSimprim::get_enable_ports(Gate* sg) const
            {
                if (!is_sequential(sg))
                {
                    log_error("dataflow", "gate is not sequential: {}, type: {}", sg->get_name(), sg->get_type()->get_name());
                    return std::unordered_set<std::string>();
                }

                static std::map<std::string, std::unordered_set<std::string>> gate_to_enable_ports;
                if (gate_to_enable_ports.empty())
                {
                    gate_to_enable_ports["X_FF"] = {"CE"};
                }

                return gate_to_enable_ports.at(sg->get_type()->get_name());
            }

            std::unordered_set<std::string> UtilsXilinxSimprim::get_reset_ports(Gate* sg) const
            {
                if (!is_sequential(sg))
                {
                    log_error("dataflow", "gate is not sequential: {}, type: {}", sg->get_name(), sg->get_type()->get_name());
                    return std::unordered_set<std::string>();
                }

                static std::map<std::string, std::unordered_set<std::string>> gate_to_reset_ports;
                if (gate_to_reset_ports.empty())
                {
                    gate_to_reset_ports["X_FF"] = {"RST"};
                }

                return gate_to_reset_ports.at(sg->get_type()->get_name());
            }

            std::unordered_set<std::string> UtilsXilinxSimprim::get_data_ports(Gate* sg) const
            {
                if (!is_sequential(sg))
                {
                    log_error("dataflow", "gate is not sequential: {}, type: {}", sg->get_name(), sg->get_type()->get_name());
                    return std::unordered_set<std::string>();
                }

                static std::map<std::string, std::unordered_set<std::string>> gate_to_data_ports;
                if (gate_to_data_ports.empty())
                {
                    gate_to_data_ports["X_FF"] = {"I", "SET"};
                }

                return gate_to_data_ports.at(sg->get_type()->get_name());
            }
        }    // namespace dataflow_utils
    }        // namespace dataflow
}    // namespace hal