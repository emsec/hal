#include "utils/utils_nangate.h"

#include "core/log.h"
#include "netlist/gate.h"

#include <string>
#include <unordered_set>

namespace hal
{
    namespace dataflow_utils
    {
        bool UtilsNangate::is_sequential(Gate* sg) const
        {
            static std::unordered_set<std::string> supported = {"DFFRS_X1",
                                                                "DFFRS_X2",
                                                                "DFFR_X1",
                                                                "DFFR_X2",
                                                                "DFF_X1",
                                                                "DFF_X2",
                                                                "DFFS_X1",
                                                                "DFFS_X2",
                                                                "SDFFRS_X1",
                                                                "SDFFRS_X2",
                                                                "SDFFR_X1",
                                                                "SDFFR_X2",
                                                                "SDFFS_X1",
                                                                "SDFFS_X2",
                                                                "SDFF_X1",
                                                                "SDFF_X2",
                                                                "SDFFR_X1"};
            if (supported.find(sg->get_type()->get_name()) != supported.end())
            {
                return true;
            }
            static std::unordered_set<std::string> unsupported = {};
            if (supported.find(sg->get_type()->get_name()) != supported.end())
            {
                log_error("dataflow", "currently not supporting scan chain FFs");
                return true;
            }
            return false;
        }

        std::unordered_set<std::string> UtilsNangate::get_control_input_pin_types(Gate* sg) const
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

        std::unordered_set<std::string> UtilsNangate::get_clock_ports(Gate* sg) const
        {
            if (!is_sequential(sg))
            {
                log_error("dataflow", "gate is not sequential: {}, type: {}", sg->get_name(), sg->get_type()->get_name());
                return std::unordered_set<std::string>();
            }
            static std::map<std::string, std::unordered_set<std::string>> gate_to_clock_ports;

            if (gate_to_clock_ports.empty())
            {
                gate_to_clock_ports["DFFRS_X1"]  = {"CK"};
                gate_to_clock_ports["DFFRS_X2"]  = {"CK"};
                gate_to_clock_ports["DFFR_X1"]   = {"CK"};
                gate_to_clock_ports["DFFR_X2"]   = {"CK"};
                gate_to_clock_ports["DFFS_X1"]   = {"CK"};
                gate_to_clock_ports["DFFS_X2"]   = {"CK"};
                gate_to_clock_ports["DFF_X1"]    = {"CK"};
                gate_to_clock_ports["DFF_X2"]    = {"CK"};
                gate_to_clock_ports["SDFFRS_X1"] = {"CK"};
                gate_to_clock_ports["SDFFRS_X2"] = {"CK"};
                gate_to_clock_ports["SDFFR_X1"]  = {"CK"};
                gate_to_clock_ports["SDFFR_X2"]  = {"CK"};
                gate_to_clock_ports["SDFFS_X1"]  = {"CK"};
                gate_to_clock_ports["SDFFS_X2"]  = {"CK"};
                gate_to_clock_ports["SDFF_X1"]   = {"CK"};
                gate_to_clock_ports["SDFF_X2"]   = {"CK"};
            }

            return gate_to_clock_ports.at(sg->get_type()->get_name());
        }

        std::unordered_set<std::string> UtilsNangate::get_enable_ports(Gate* sg) const
        {
            if (!is_sequential(sg))
            {
                log_error("dataflow", "gate is not sequential: {}, type: {}", sg->get_name(), sg->get_type()->get_name());
                return std::unordered_set<std::string>();
            }
            static std::map<std::string, std::unordered_set<std::string>> gate_to_enable_ports;

            if (gate_to_enable_ports.empty())
            {
                gate_to_enable_ports["DFFRS_X1"]  = {};
                gate_to_enable_ports["DFFRS_X2"]  = {};
                gate_to_enable_ports["DFFR_X1"]   = {};
                gate_to_enable_ports["DFFR_X2"]   = {};
                gate_to_enable_ports["DFFS_X1"]   = {};
                gate_to_enable_ports["DFFS_X2"]   = {};
                gate_to_enable_ports["DFF_X1"]    = {};
                gate_to_enable_ports["DFF_X2"]    = {};
                gate_to_enable_ports["SDFFRS_X1"] = {"SE"};
                gate_to_enable_ports["SDFFRS_X2"] = {"SE"};
                gate_to_enable_ports["SDFFR_X1"]  = {"SE"};
                gate_to_enable_ports["SDFFR_X2"]  = {"SE"};
                gate_to_enable_ports["SDFFS_X1"]  = {"SE"};
                gate_to_enable_ports["SDFFS_X2"]  = {"SE"};
                gate_to_enable_ports["SDFF_X1"]   = {"SE"};
                gate_to_enable_ports["SDFF_X2"]   = {"SE"};
            }

            return gate_to_enable_ports.at(sg->get_type()->get_name());
        }

        std::unordered_set<std::string> UtilsNangate::get_reset_ports(Gate* sg) const
        {
            if (!is_sequential(sg))
            {
                log_error("dataflow", "gate is not sequential: {}, type: {}", sg->get_name(), sg->get_type()->get_name());
                return std::unordered_set<std::string>();
            }

            static std::map<std::string, std::unordered_set<std::string>> gate_to_reset_ports;
            if (gate_to_reset_ports.empty())
            {
                gate_to_reset_ports["DFFRS_X1"]  = {"RN", "SN"};
                gate_to_reset_ports["DFFRS_X2"]  = {"RN", "SN"};
                gate_to_reset_ports["DFFR_X1"]   = {"RN"};
                gate_to_reset_ports["DFFR_X2"]   = {"RN"};
                gate_to_reset_ports["DFFS_X1"]   = {"SN"};
                gate_to_reset_ports["DFFS_X2"]   = {"SN"};
                gate_to_reset_ports["DFF_X1"]    = {};
                gate_to_reset_ports["DFF_X2"]    = {};
                gate_to_reset_ports["SDFFRS_X1"] = {"RN", "SN"};
                gate_to_reset_ports["SDFFRS_X2"] = {"RN", "SN"};
                gate_to_reset_ports["SDFFR_X1"]  = {"RN"};
                gate_to_reset_ports["SDFFR_X2"]  = {"RN"};
                gate_to_reset_ports["SDFFS_X1"]  = {"SN"};
                gate_to_reset_ports["SDFFS_X2"]  = {"SN"};
                gate_to_reset_ports["SDFF_X1"]   = {"C"};
                gate_to_reset_ports["SDFF_X2"]   = {"C"};
            }

            return gate_to_reset_ports.at(sg->get_type()->get_name());
        }

        std::unordered_set<std::string> UtilsNangate::get_data_ports(Gate* sg) const
        {
            static std::map<std::string, std::unordered_set<std::string>> gate_to_data_ports;
            if (gate_to_data_ports.empty())
            {
                gate_to_data_ports["DFFRS_X1"]  = {"D"};
                gate_to_data_ports["DFFRS_X2"]  = {"D"};
                gate_to_data_ports["DFFR_X1"]   = {"D"};
                gate_to_data_ports["DFFR_X2"]   = {"D"};
                gate_to_data_ports["DFFS_X1"]   = {"D"};
                gate_to_data_ports["DFFS_X2"]   = {"D"};
                gate_to_data_ports["DFF_X1"]    = {"D"};
                gate_to_data_ports["DFF_X2"]    = {"D"};
                gate_to_data_ports["SDFFRS_X1"] = {"D", "SI"};
                gate_to_data_ports["SDFFRS_X2"] = {"D", "SI"};
                gate_to_data_ports["SDFFR_X1"]  = {"D", "SI"};
                gate_to_data_ports["SDFFR_X2"]  = {"D", "SI"};
                gate_to_data_ports["SDFFS_X1"]  = {"D", "SI"};
                gate_to_data_ports["SDFFS_X2"]  = {"D", "SI"};
                gate_to_data_ports["SDFF_X1"]   = {"D", "SI"};
                gate_to_data_ports["SDFF_X2"]   = {"D", "SI"};
            }

            return gate_to_data_ports.at(sg->get_type()->get_name());
        }
    }    // namespace dataflow_utils
}    // namespace hal
