#include "dataflow_analysis/utils/utils_ice40ultra.h"

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
            bool UtilsiCE40Ultra::is_sequential(Gate* sg) const
            {
                static std::unordered_set<std::string> supported = {"SB_DFF",  "SB_DFFE",  "SB_DFFSR",  "SB_DFFR",  "SB_DFFSS",  "SB_DFFS",  "SB_DFFESR",  "SB_DFFER",  "SB_DFFESS",  "SB_DFFES",
                                                                    "SB_DFFN", "SB_DFFNE", "SB_DFFNSR", "SB_DFFNR", "SB_DFFNSS", "SB_DFFNS", "SB_DFFNESR", "SB_DFFNER", "SB_DFFNESS", "SB_DFFNES"};

                if (supported.find(sg->get_type()->get_name()) != supported.end())
                {
                    return true;
                }

                if (sg->get_type()->get_name().find("RAM") != std::string::npos)
                {
                    //log_error("dataflow", "need to implement the following gate_type: {} for gate: {}", sg->get_type()->get_name(), sg->get_name());
                }
                return false;
            }

            std::unordered_set<std::string> UtilsiCE40Ultra::get_control_input_pin_types(Gate* sg) const
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

            std::unordered_set<std::string> UtilsiCE40Ultra::get_clock_ports(Gate* sg) const
            {
                if (!is_sequential(sg))
                {
                    log_error("dataflow", "gate is not sequential: {}, type: {}", sg->get_name(), sg->get_type()->get_name());
                    return std::unordered_set<std::string>();
                }

                static std::map<std::string, std::unordered_set<std::string>> gate_to_clock_ports;
                if (gate_to_clock_ports.empty())
                {
                    gate_to_clock_ports["SB_DFF"] = {"C"};
                    gate_to_clock_ports["SB_DFFE"]  = {"C"};
                    gate_to_clock_ports["SB_DFFSR"]   = {"C"};
                    gate_to_clock_ports["SB_DFFR"]  = {"C"};
                    gate_to_clock_ports["SB_DFFSS"] = {"C"};
                    gate_to_clock_ports["SB_DFFS"]  = {"C"};
                    gate_to_clock_ports["SB_DFFESR"] = {"C"};
                    gate_to_clock_ports["SB_DFFER"] = {"C"};
                    gate_to_clock_ports["SB_DFFESS"] = {"C"};
                    gate_to_clock_ports["SB_DFFES"] = {"C"};
                    gate_to_clock_ports["SB_DFFN"] = {"C"};
                    gate_to_clock_ports["SB_DFFNE"] = {"C"};
                    gate_to_clock_ports["SB_DFFNSR"] = {"C"};
                    gate_to_clock_ports["SB_DFFNR"] = {"C"};
                    gate_to_clock_ports["SB_DFFNSS"] = {"C"};
                    gate_to_clock_ports["SB_DFFNS"] = {"C"};
                    gate_to_clock_ports["SB_DFFNESR"] = {"C"};
                    gate_to_clock_ports["SB_DFFNER"] = {"C"};
                    gate_to_clock_ports["SB_DFFNESS"] = {"C"};
                    gate_to_clock_ports["SB_DFFNES"] = {"C"};
                }

                return gate_to_clock_ports.at(sg->get_type()->get_name());
            }

            std::unordered_set<std::string> UtilsiCE40Ultra::get_enable_ports(Gate* sg) const
            {
                if (!is_sequential(sg))
                {
                    log_error("dataflow", "gate is not sequential: {}, type: {}", sg->get_name(), sg->get_type()->get_name());
                    return std::unordered_set<std::string>();
                }

                static std::map<std::string, std::unordered_set<std::string>> gate_to_enable_ports;
                if (gate_to_enable_ports.empty())
                {
                    gate_to_enable_ports["SB_DFF"] = {};
                    gate_to_enable_ports["SB_DFFE"]  = {"E"};
                    gate_to_enable_ports["SB_DFFSR"]   = {};
                    gate_to_enable_ports["SB_DFFR"]  = {};
                    gate_to_enable_ports["SB_DFFSS"] = {};
                    gate_to_enable_ports["SB_DFFS"]  = {};
                    gate_to_enable_ports["SB_DFFESR"] = {"E"};
                    gate_to_enable_ports["SB_DFFER"] = {"E"};
                    gate_to_enable_ports["SB_DFFESS"] = {"E"};
                    gate_to_enable_ports["SB_DFFES"] = {"E"};
                    gate_to_enable_ports["SB_DFFN"] = {};
                    gate_to_enable_ports["SB_DFFNE"] = {"E"};
                    gate_to_enable_ports["SB_DFFNSR"] = {};
                    gate_to_enable_ports["SB_DFFNR"] = {};
                    gate_to_enable_ports["SB_DFFNSS"] = {};
                    gate_to_enable_ports["SB_DFFNS"] = {};
                    gate_to_enable_ports["SB_DFFNESR"] = {"E"};
                    gate_to_enable_ports["SB_DFFNER"] = {"E"};
                    gate_to_enable_ports["SB_DFFNESS"] = {"E"};
                    gate_to_enable_ports["SB_DFFNES"] = {"E"};
                }

                return gate_to_enable_ports.at(sg->get_type()->get_name());
            }

            std::unordered_set<std::string> UtilsiCE40Ultra::get_reset_ports(Gate* sg) const
            {
                if (!is_sequential(sg))
                {
                    log_error("dataflow", "gate is not sequential: {}, type: {}", sg->get_name(), sg->get_type()->get_name());
                    return std::unordered_set<std::string>();
                }

                static std::map<std::string, std::unordered_set<std::string>> gate_to_reset_ports;
                if (gate_to_reset_ports.empty())
                {
                    gate_to_reset_ports["SB_DFF"] = {};
                    gate_to_reset_ports["SB_DFFE"]  = {};
                    gate_to_reset_ports["SB_DFFSR"]   = {"R"};
                    gate_to_reset_ports["SB_DFFR"]  = {"R"};
                    gate_to_reset_ports["SB_DFFSS"] = {};
                    gate_to_reset_ports["SB_DFFS"]  = {};
                    gate_to_reset_ports["SB_DFFESR"] = {"R"};
                    gate_to_reset_ports["SB_DFFER"] = {"R"};
                    gate_to_reset_ports["SB_DFFESS"] = {};
                    gate_to_reset_ports["SB_DFFES"] = {};
                    gate_to_reset_ports["SB_DFFN"] = {};
                    gate_to_reset_ports["SB_DFFNE"] = {};
                    gate_to_reset_ports["SB_DFFNSR"] = {"R"};
                    gate_to_reset_ports["SB_DFFNR"] = {"R"};
                    gate_to_reset_ports["SB_DFFNSS"] = {};
                    gate_to_reset_ports["SB_DFFNS"] = {};
                    gate_to_reset_ports["SB_DFFNESR"] = {"R"};
                    gate_to_reset_ports["SB_DFFNER"] = {"R"};
                    gate_to_reset_ports["SB_DFFNESS"] = {};
                    gate_to_reset_ports["SB_DFFNES"] = {};
                }

                return gate_to_reset_ports.at(sg->get_type()->get_name());
            }

            std::unordered_set<std::string> UtilsiCE40Ultra::get_data_ports(Gate* sg) const
            {
                if (!is_sequential(sg))
                {
                    log_error("dataflow", "gate is not sequential: {}, type: {}", sg->get_name(), sg->get_type()->get_name());
                    return std::unordered_set<std::string>();
                }

                static std::map<std::string, std::unordered_set<std::string>> gate_to_data_ports;
                if (gate_to_data_ports.empty())
                {
                    gate_to_data_ports["SB_DFF"] = {"D"};
                    gate_to_data_ports["SB_DFFE"]  = {"D"};
                    gate_to_data_ports["SB_DFFSR"] = {"D"};
                    gate_to_data_ports["SB_DFFR"]  = {"D"};
                    gate_to_data_ports["SB_DFFSS"] = {"D", "S"};
                    gate_to_data_ports["SB_DFFS"]  = {"D", "S"};
                    gate_to_data_ports["SB_DFFESR"] = {"D"};
                    gate_to_data_ports["SB_DFFER"] = {"D"};
                    gate_to_data_ports["SB_DFFESS"] = {"D", "S"};
                    gate_to_data_ports["SB_DFFES"] = {"D", "S"};
                    gate_to_data_ports["SB_DFFN"] = {"D"};
                    gate_to_data_ports["SB_DFFNE"] = {"D"};
                    gate_to_data_ports["SB_DFFNSR"] = {"D"};
                    gate_to_data_ports["SB_DFFNR"] = {"D"};
                    gate_to_data_ports["SB_DFFNSS"] = {"D", "S"};
                    gate_to_data_ports["SB_DFFNS"] = {"D", "S"};
                    gate_to_data_ports["SB_DFFNESR"] = {"D"};
                    gate_to_data_ports["SB_DFFNER"] = {"D"};
                    gate_to_data_ports["SB_DFFNESS"] = {"D", "S"};
                    gate_to_data_ports["SB_DFFNES"] = {"D", "S"};
                }

                return gate_to_data_ports.at(sg->get_type()->get_name());
            }
        }    // namespace dataflow_utils
    }        // namespace dataflow
}    // namespace hal