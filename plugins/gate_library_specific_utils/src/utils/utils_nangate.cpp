#include "utils/utils_nangate.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/utilities/log.h"

#include <string>
#include <unordered_set>

namespace hal
{
    namespace gate_library_specific_utils
    {
        bool UtilsNangate::is_sequential(GateType* g) const
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
            if (supported.find(g->get_name()) != supported.end())
            {
                return true;
            }
            static std::unordered_set<std::string> unsupported = {};
            if (supported.find(g->get_name()) != supported.end())
            {
                log_error("gl specifics", "currently not supporting scan chain FFs");
                return true;
            }
            return false;
        }

        std::unordered_set<std::string> UtilsNangate::get_control_input_pin_types(GateType* g) const
        {
            // NOTE We include the set ports because sometimes the synthesizer uses the set ports as data inputs.
            auto data_ports = get_data_ports(g);
            auto set_ports  = get_set_ports(g);
            data_ports.insert(set_ports.begin(), set_ports.end());
            std::unordered_set<std::string> control_input_pin_types;
            for (const auto& pin_type : g->get_input_pins())
            {
                if (data_ports.find(pin_type) != data_ports.end())
                    continue;

                control_input_pin_types.insert(pin_type);
            }
            return control_input_pin_types;
        }

        std::unordered_set<std::string> UtilsNangate::get_clock_ports(GateType* g) const
        {
            if (!is_sequential(g))
            {
                log_error("gl specifics", "gate type is not sequential: type: {}", g->get_name());
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

            return gate_to_clock_ports.at(g->get_name());
        }

        std::unordered_set<std::string> UtilsNangate::get_enable_ports(GateType* g) const
        {
            if (!is_sequential(g))
            {
                log_error("gl specifics", "gate type is not sequential: type: {}", g->get_name());
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

            return gate_to_enable_ports.at(g->get_name());
        }

        std::unordered_set<std::string> UtilsNangate::get_reset_ports(GateType* g) const
        {
            if (!is_sequential(g))
            {
                log_error("gl specifics", "gate type is not sequential: type: {}", g->get_name());
                return std::unordered_set<std::string>();
            }

            static std::map<std::string, std::unordered_set<std::string>> gate_to_reset_ports;
            if (gate_to_reset_ports.empty())
            {
                gate_to_reset_ports["DFFRS_X1"]  = {"RN"};
                gate_to_reset_ports["DFFRS_X2"]  = {"RN"};
                gate_to_reset_ports["DFFR_X1"]   = {"RN"};
                gate_to_reset_ports["DFFR_X2"]   = {"RN"};
                gate_to_reset_ports["DFFS_X1"]   = {};
                gate_to_reset_ports["DFFS_X2"]   = {};
                gate_to_reset_ports["DFF_X1"]    = {};
                gate_to_reset_ports["DFF_X2"]    = {};
                gate_to_reset_ports["SDFFRS_X1"] = {"RN"};
                gate_to_reset_ports["SDFFRS_X2"] = {"RN"};
                gate_to_reset_ports["SDFFR_X1"]  = {"RN"};
                gate_to_reset_ports["SDFFR_X2"]  = {"RN"};
                gate_to_reset_ports["SDFFS_X1"]  = {};
                gate_to_reset_ports["SDFFS_X2"]  = {};
                gate_to_reset_ports["SDFF_X1"]   = {};
                gate_to_reset_ports["SDFF_X2"]   = {};
            }

            return gate_to_reset_ports.at(g->get_name());
        }

        std::unordered_set<std::string> UtilsNangate::get_data_ports(GateType* g) const
        {
            if (!is_sequential(g))
            {
                log_error("gl specifics", "gate type is not sequential: type: {}", g->get_name());
                return std::unordered_set<std::string>();
            }
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

            return gate_to_data_ports.at(g->get_name());
        }

        std::unordered_set<std::string> UtilsNangate::get_set_ports(GateType* g) const
        {
            if (!is_sequential(g))
            {
                log_error("gl specifics", "gate type is not sequential: type: {}", g->get_name());
                return std::unordered_set<std::string>();
            }

            static std::map<std::string, std::unordered_set<std::string>> gate_to_set_ports;
            if (gate_to_set_ports.empty())
            {
                gate_to_set_ports["DFFRS_X1"]  = {"SN"};
                gate_to_set_ports["DFFRS_X2"]  = {"SN"};
                gate_to_set_ports["DFFR_X1"]   = {};
                gate_to_set_ports["DFFR_X2"]   = {};
                gate_to_set_ports["DFFS_X1"]   = {"SN"};
                gate_to_set_ports["DFFS_X2"]   = {"SN"};
                gate_to_set_ports["DFF_X1"]    = {};
                gate_to_set_ports["DFF_X2"]    = {};
                gate_to_set_ports["SDFFRS_X1"] = {"SN"};
                gate_to_set_ports["SDFFRS_X2"] = {"SN"};
                gate_to_set_ports["SDFFR_X1"]  = {};
                gate_to_set_ports["SDFFR_X2"]  = {};
                gate_to_set_ports["SDFFS_X1"]  = {"SN"};
                gate_to_set_ports["SDFFS_X2"]  = {"SN"};
                gate_to_set_ports["SDFF_X1"]   = {};
                gate_to_set_ports["SDFF_X2"]   = {};
            }

            return gate_to_set_ports.at(g->get_name());
        }

        std::unordered_set<std::string> UtilsNangate::get_regular_outputs(GateType* g) const
        {
            if (!is_sequential(g))
            {
                log_error("gl specifics", "gate type is not sequential: type: {}", g->get_name());
                return std::unordered_set<std::string>();
            }

            static std::map<std::string, std::unordered_set<std::string>> gate_to_regular_output_ports;
            if (gate_to_regular_output_ports.empty())
            {
                gate_to_regular_output_ports["DFFRS_X1"]  = {"Q"};
                gate_to_regular_output_ports["DFFRS_X2"]  = {"Q"};
                gate_to_regular_output_ports["DFFR_X1"]   = {"Q"};
                gate_to_regular_output_ports["DFFR_X2"]   = {"Q"};
                gate_to_regular_output_ports["DFFS_X1"]   = {"Q"};
                gate_to_regular_output_ports["DFFS_X2"]   = {"Q"};
                gate_to_regular_output_ports["DFF_X1"]    = {"Q"};
                gate_to_regular_output_ports["DFF_X2"]    = {"Q"};
                gate_to_regular_output_ports["SDFFRS_X1"] = {"Q"};
                gate_to_regular_output_ports["SDFFRS_X2"] = {"Q"};
                gate_to_regular_output_ports["SDFFR_X1"]  = {"Q"};
                gate_to_regular_output_ports["SDFFR_X2"]  = {"Q"};
                gate_to_regular_output_ports["SDFFS_X1"]  = {"Q"};
                gate_to_regular_output_ports["SDFFS_X2"]  = {"Q"};
                gate_to_regular_output_ports["SDFF_X1"]   = {"Q"};
                gate_to_regular_output_ports["SDFF_X2"]   = {"Q"};
            }

            return gate_to_regular_output_ports.at(g->get_name());
        }

        std::unordered_set<std::string> UtilsNangate::get_negated_outputs(GateType* g) const
        {
            if (!is_sequential(g))
            {
                log_error("gl specifics", "gate type is not sequential: type: {}", g->get_name());
                return std::unordered_set<std::string>();
            }

            static std::map<std::string, std::unordered_set<std::string>> gate_to_negated_output_ports;
            if (gate_to_negated_output_ports.empty())
            {
                gate_to_negated_output_ports["DFFRS_X1"]  = {"QN"};
                gate_to_negated_output_ports["DFFRS_X2"]  = {"QN"};
                gate_to_negated_output_ports["DFFR_X1"]   = {"QN"};
                gate_to_negated_output_ports["DFFR_X2"]   = {"QN"};
                gate_to_negated_output_ports["DFFS_X1"]   = {"QN"};
                gate_to_negated_output_ports["DFFS_X2"]   = {"QN"};
                gate_to_negated_output_ports["DFF_X1"]    = {"QN"};
                gate_to_negated_output_ports["DFF_X2"]    = {"QN"};
                gate_to_negated_output_ports["SDFFRS_X1"] = {"QN"};
                gate_to_negated_output_ports["SDFFRS_X2"] = {"QN"};
                gate_to_negated_output_ports["SDFFR_X1"]  = {"QN"};
                gate_to_negated_output_ports["SDFFR_X2"]  = {"QN"};
                gate_to_negated_output_ports["SDFFS_X1"]  = {"QN"};
                gate_to_negated_output_ports["SDFFS_X2"]  = {"QN"};
                gate_to_negated_output_ports["SDFF_X1"]   = {"QN"};
                gate_to_negated_output_ports["SDFF_X2"]   = {"QN"};
            }

            return gate_to_negated_output_ports.at(g->get_name());
        }
    }    // namespace gate_library_specific_utils
}    // namespace hal