#include "utils/utils_xilinx_simprim.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/utilities/log.h"

#include <string>
#include <unordered_set>

namespace hal
{
    namespace gate_library_specific_utils
    {
        bool UtilsXilinxSimprim::is_sequential(GateType* g) const
        {
            static std::unordered_set<std::string> supported = {"X_FF"};

            if (supported.find(g->get_name()) != supported.end())
            {
                return true;
            }
            return false;
        }

        std::unordered_set<std::string> UtilsXilinxSimprim::get_control_input_pin_types(GateType* g) const
        {
            auto data_ports = get_data_ports(g);
            std::unordered_set<std::string> control_input_pin_types;
            for (const auto& pin_type : g->get_input_pins())
            {
                if (data_ports.find(pin_type) != data_ports.end())
                    continue;

                control_input_pin_types.insert(pin_type);
            }
            return control_input_pin_types;
        }

        std::unordered_set<std::string> UtilsXilinxSimprim::get_clock_ports(GateType* g) const
        {
            if (!is_sequential(g))
            {
                log_error("gl specifics", "gate type is not sequential: type: {}", g->get_name());
                return std::unordered_set<std::string>();
            }

            static std::map<std::string, std::unordered_set<std::string>> gate_to_clock_ports;
            if (gate_to_clock_ports.empty())
            {
                gate_to_clock_ports["X_FF"] = {"CLK"};
            }

            return gate_to_clock_ports.at(g->get_name());
        }

        std::unordered_set<std::string> UtilsXilinxSimprim::get_enable_ports(GateType* g) const
        {
            if (!is_sequential(g))
            {
                log_error("gl specifics", "gate type is not sequential: type: {}", g->get_name());
                return std::unordered_set<std::string>();
            }

            static std::map<std::string, std::unordered_set<std::string>> gate_to_enable_ports;
            if (gate_to_enable_ports.empty())
            {
                gate_to_enable_ports["X_FF"] = {"CE"};
            }

            return gate_to_enable_ports.at(g->get_name());
        }

        std::unordered_set<std::string> UtilsXilinxSimprim::get_reset_ports(GateType* g) const
        {
            if (!is_sequential(g))
            {
                log_error("gl specifics", "gate type is not sequential: type: {}", g->get_name());
                return std::unordered_set<std::string>();
            }

            static std::map<std::string, std::unordered_set<std::string>> gate_to_reset_ports;
            if (gate_to_reset_ports.empty())
            {
                gate_to_reset_ports["X_FF"] = {"RST"};
            }

            return gate_to_reset_ports.at(g->get_name());
        }

        std::unordered_set<std::string> UtilsXilinxSimprim::get_data_ports(GateType* g) const
        {
            if (!is_sequential(g))
            {
                log_error("gl specifics", "gate type is not sequential: type: {}", g->get_name());
                return std::unordered_set<std::string>();
            }

            static std::map<std::string, std::unordered_set<std::string>> gate_to_data_ports;
            if (gate_to_data_ports.empty())
            {
                gate_to_data_ports["X_FF"] = {"I"};
            }

            return gate_to_data_ports.at(g->get_name());
        }

        std::unordered_set<std::string> UtilsXilinxSimprim::get_set_ports(GateType* g) const
        {
            if (!is_sequential(g))
            {
                log_error("gl specifics", "gate type is not sequential: type: {}", g->get_name());
                return std::unordered_set<std::string>();
            }

            static std::map<std::string, std::unordered_set<std::string>> gate_to_set_ports;
            if (gate_to_set_ports.empty())
            {
                gate_to_set_ports["X_FF"] = {"SET"};
            }

            return gate_to_set_ports.at(g->get_name());
        }

        std::unordered_set<std::string> UtilsXilinxSimprim::get_regular_outputs(GateType* g) const
        {
            if (!is_sequential(g))
            {
                log_error("gl specifics", "gate type is not sequential: type: {}", g->get_name());
                return std::unordered_set<std::string>();
            }

            static std::map<std::string, std::unordered_set<std::string>> gate_to_regular_output_ports;
            if (gate_to_regular_output_ports.empty())
            {
                gate_to_regular_output_ports["X_FF"] = {"O"};
            }

            return gate_to_regular_output_ports.at(g->get_name());
        }

        std::unordered_set<std::string> UtilsXilinxSimprim::get_negated_outputs(GateType* g) const
        {
            if (!is_sequential(g))
            {
                log_error("gl specifics", "gate type is not sequential: type: {}", g->get_name());
                return std::unordered_set<std::string>();
            }

            static std::map<std::string, std::unordered_set<std::string>> gate_to_negated_output_ports;
            if (gate_to_negated_output_ports.empty())
            {
                gate_to_negated_output_ports["X_FF"] = {};
            }

            return gate_to_negated_output_ports.at(g->get_name());
        }
    }    // namespace gate_library_specific_utils
}    // namespace hal