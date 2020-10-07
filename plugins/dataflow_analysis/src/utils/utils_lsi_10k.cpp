#include "dataflow_analysis/utils/utils_lsi_10k.h"

#include "hal_core/utilities/log.h"
#include "hal_core/netlist/gate.h"

#include <string>
#include <unordered_set>

namespace hal
{
    namespace dataflow_utils
    {
        bool UtilsLSI_10K::is_sequential(Gate* sg) const
        {
            static std::unordered_set<std::string> supported = {"FD1",   "FD1P",   "FD1S",  "FD1SP", "FD2",    "FD2P",  "FD2S",   "FD2SP", "FD3",   "FD3P",  "FD3S",   "FD3SP", "FD4",
                                                                "FD4P",  "FD4S",   "FD4SP", "FJK1",  "FJK1P",  "FJK1S", "FJK1SP", "FJK2",  "FJK2P", "FJK2S", "FJK2SP", "FJK3",  "FJK3P",
                                                                "FJK3S", "FJK3SP", "FDS2",  "FDS2L", "FDS2LP", "FDS2P", "FT2",    "FT2P",  "FT4",   "FT4P",  "FD2TS",  "FD2TSP"};
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

        std::unordered_set<std::string> UtilsLSI_10K::get_control_input_pin_types(Gate* sg) const
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

        std::unordered_set<std::string> UtilsLSI_10K::get_clock_ports(Gate* sg) const
        {
            if (!is_sequential(sg))
            {
                log_error("dataflow", "gate is not sequential: {}, type: {}", sg->get_name(), sg->get_type()->get_name());
                return std::unordered_set<std::string>();
            }
            static std::map<std::string, std::unordered_set<std::string>> gate_to_clock_ports;

            if (gate_to_clock_ports.empty())
            {
                gate_to_clock_ports["FD1"]    = {"CP"};
                gate_to_clock_ports["FD1P"]   = {"CP"};
                gate_to_clock_ports["FD1S"]   = {"CP"};
                gate_to_clock_ports["FD1SP"]  = {"CP"};
                gate_to_clock_ports["FD2"]    = {"CP"};
                gate_to_clock_ports["FD2P"]   = {"CP"};
                gate_to_clock_ports["FD2S"]   = {"CP"};
                gate_to_clock_ports["FD2SP"]  = {"CP"};
                gate_to_clock_ports["FD3"]    = {"CP"};
                gate_to_clock_ports["FD3P"]   = {"CP"};
                gate_to_clock_ports["FD3S"]   = {"CP"};
                gate_to_clock_ports["FD3SP"]  = {"CP"};
                gate_to_clock_ports["FD4"]    = {"CP"};
                gate_to_clock_ports["FD4P"]   = {"CP"};
                gate_to_clock_ports["FD4S"]   = {"CP"};
                gate_to_clock_ports["FD4SP"]  = {"CP"};
                gate_to_clock_ports["FJK1"]   = {"CP"};
                gate_to_clock_ports["FJK1P"]  = {"CP"};
                gate_to_clock_ports["FJK1S"]  = {"CP"};
                gate_to_clock_ports["FJK1SP"] = {"CP"};
                gate_to_clock_ports["FJK2"]   = {"CP"};
                gate_to_clock_ports["FJK2P"]  = {"CP"};
                gate_to_clock_ports["FJK2S"]  = {"CP"};
                gate_to_clock_ports["FJK2SP"] = {"CP"};
                gate_to_clock_ports["FJK3"]   = {"CP"};
                gate_to_clock_ports["FJK3P"]  = {"CP"};
                gate_to_clock_ports["FJK3S"]  = {"CP"};
                gate_to_clock_ports["FJK3SP"] = {"CP"};
                gate_to_clock_ports["FDS2"]   = {"CP"};
                gate_to_clock_ports["FDS2L"]  = {"CP"};
                gate_to_clock_ports["FDS2LP"] = {"CP"};
                gate_to_clock_ports["FDS2P"]  = {"CP"};
                gate_to_clock_ports["FT2"]    = {"CP"};
                gate_to_clock_ports["FT2P"]   = {"CP"};
                gate_to_clock_ports["FT4"]    = {"CP"};
                gate_to_clock_ports["FT4P"]   = {"CP"};
                gate_to_clock_ports["FD2TS"]  = {"CP"};
                gate_to_clock_ports["FD2TSP"] = {"CP"};
            }

            return gate_to_clock_ports.at(sg->get_type()->get_name());
        }

        std::unordered_set<std::string> UtilsLSI_10K::get_enable_ports(Gate* sg) const
        {
            if (!is_sequential(sg))
            {
                log_error("dataflow", "gate is not sequential: {}, type: {}", sg->get_name(), sg->get_type()->get_name());
                return std::unordered_set<std::string>();
            }
            static std::map<std::string, std::unordered_set<std::string>> gate_to_enable_ports;

            if (gate_to_enable_ports.empty())
            {
                gate_to_enable_ports["FD1"]    = {};
                gate_to_enable_ports["FD1P"]   = {};
                gate_to_enable_ports["FD1S"]   = {"TE"};
                gate_to_enable_ports["FD1SP"]  = {"TE"};
                gate_to_enable_ports["FD2"]    = {};
                gate_to_enable_ports["FD2P"]   = {};
                gate_to_enable_ports["FD2S"]   = {"TE"};
                gate_to_enable_ports["FD2SP"]  = {"TE"};
                gate_to_enable_ports["FD3"]    = {};
                gate_to_enable_ports["FD3P"]   = {};
                gate_to_enable_ports["FD3S"]   = {"TE"};
                gate_to_enable_ports["FD3SP"]  = {"TE"};
                gate_to_enable_ports["FD4"]    = {};
                gate_to_enable_ports["FD4P"]   = {};
                gate_to_enable_ports["FD4S"]   = {"TE"};
                gate_to_enable_ports["FD4SP"]  = {"TE"};
                gate_to_enable_ports["FJK1"]   = {};
                gate_to_enable_ports["FJK1P"]  = {};
                gate_to_enable_ports["FJK1S"]  = {"TE"};
                gate_to_enable_ports["FJK1SP"] = {"TE"};
                gate_to_enable_ports["FJK2"]   = {};
                gate_to_enable_ports["FJK2P"]  = {};
                gate_to_enable_ports["FJK2S"]  = {"TE"};
                gate_to_enable_ports["FJK2SP"] = {"TE"};
                gate_to_enable_ports["FJK3"]   = {};
                gate_to_enable_ports["FJK3P"]  = {};
                gate_to_enable_ports["FJK3S"]  = {"TE"};
                gate_to_enable_ports["FJK3SP"] = {"TE"};
                gate_to_enable_ports["FDS2"]   = {};
                gate_to_enable_ports["FDS2L"]  = {};
                gate_to_enable_ports["FDS2LP"] = {};
                gate_to_enable_ports["FDS2P"]  = {};
                gate_to_enable_ports["FT2"]    = {};
                gate_to_enable_ports["FT2P"]   = {};
                gate_to_enable_ports["FT4"]    = {};
                gate_to_enable_ports["FT4P"]   = {};
                gate_to_enable_ports["FD2TS"]  = {};
                gate_to_enable_ports["FD2TSP"] = {};
            }

            return gate_to_enable_ports.at(sg->get_type()->get_name());
        }

        std::unordered_set<std::string> UtilsLSI_10K::get_reset_ports(Gate* sg) const
        {
            if (!is_sequential(sg))
            {
                log_error("dataflow", "gate is not sequential: {}, type: {}", sg->get_name(), sg->get_type()->get_name());
                return std::unordered_set<std::string>();
            }

            static std::map<std::string, std::unordered_set<std::string>> gate_to_reset_ports;
            if (gate_to_reset_ports.empty())
            {
                gate_to_reset_ports["FD1"]    = {};
                gate_to_reset_ports["FD1P"]   = {};
                gate_to_reset_ports["FD1S"]   = {};
                gate_to_reset_ports["FD1SP"]  = {};
                gate_to_reset_ports["FD2"]    = {"CD"};
                gate_to_reset_ports["FD2P"]   = {"CD"};
                gate_to_reset_ports["FD2S"]   = {"CD"};
                gate_to_reset_ports["FD2SP"]  = {"CD"};
                gate_to_reset_ports["FD3"]    = {"CD", "SD"};
                gate_to_reset_ports["FD3P"]   = {"CD", "SD"};
                gate_to_reset_ports["FD3S"]   = {"CD", "SD"};
                gate_to_reset_ports["FD3SP"]  = {"CD", "SD"};
                gate_to_reset_ports["FD4"]    = {"SD"};
                gate_to_reset_ports["FD4P"]   = {"SD"};
                gate_to_reset_ports["FD4S"]   = {"SD"};
                gate_to_reset_ports["FD4SP"]  = {"SD"};
                gate_to_reset_ports["FJK1"]   = {"K"};
                gate_to_reset_ports["FJK1P"]  = {"K"};
                gate_to_reset_ports["FJK1S"]  = {"K"};
                gate_to_reset_ports["FJK1SP"] = {"K"};
                gate_to_reset_ports["FJK2"]   = {"K", "CD"};
                gate_to_reset_ports["FJK2P"]  = {"K", "CD"};
                gate_to_reset_ports["FJK2S"]  = {"K", "CD"};
                gate_to_reset_ports["FJK2SP"] = {"K", "CD"};
                gate_to_reset_ports["FJK3"]   = {"K", "CD", "SD"};
                gate_to_reset_ports["FJK3P"]  = {"K", "CD", "SD"};
                gate_to_reset_ports["FJK3S"]  = {"K", "CD", "SD"};
                gate_to_reset_ports["FJK3SP"] = {"K", "CD", "SD"};
                gate_to_reset_ports["FDS2"]   = {};
                gate_to_reset_ports["FDS2L"]  = {};
                gate_to_reset_ports["FDS2LP"] = {};
                gate_to_reset_ports["FDS2P"]  = {};
                gate_to_reset_ports["FT2"]    = {"CD"};
                gate_to_reset_ports["FT2P"]   = {"CD"};
                gate_to_reset_ports["FT4"]    = {"SD"};
                gate_to_reset_ports["FT4P"]   = {"SD"};
                gate_to_reset_ports["FD2TS"]  = {"CD"};
                gate_to_reset_ports["FD2TSP"] = {"CD"};
            }

            return gate_to_reset_ports.at(sg->get_type()->get_name());
        }

        std::unordered_set<std::string> UtilsLSI_10K::get_data_ports(Gate* sg) const
        {
            static std::map<std::string, std::unordered_set<std::string>> gate_to_data_ports;
            if (gate_to_data_ports.empty())
            {
                gate_to_data_ports["FD1"]    = {"D"};
                gate_to_data_ports["FD1P"]   = {"D"};
                gate_to_data_ports["FD1S"]   = {"D", "TI"};
                gate_to_data_ports["FD1SP"]  = {"D", "TI"};
                gate_to_data_ports["FD2"]    = {"D"};
                gate_to_data_ports["FD2P"]   = {"D"};
                gate_to_data_ports["FD2S"]   = {"D", "TI"};
                gate_to_data_ports["FD2SP"]  = {"D", "TI"};
                gate_to_data_ports["FD3"]    = {"D"};
                gate_to_data_ports["FD3P"]   = {"D"};
                gate_to_data_ports["FD3S"]   = {"D", "TI"};
                gate_to_data_ports["FD3SP"]  = {"D", "TI"};
                gate_to_data_ports["FD4"]    = {"D"};
                gate_to_data_ports["FD4P"]   = {"D"};
                gate_to_data_ports["FD4S"]   = {"D", "TI"};
                gate_to_data_ports["FD4SP"]  = {"D", "TI"};
                gate_to_data_ports["FJK1"]   = {"J"};
                gate_to_data_ports["FJK1P"]  = {"J"};
                gate_to_data_ports["FJK1S"]  = {"J", "TI"};
                gate_to_data_ports["FJK1SP"] = {"J", "TI"};
                gate_to_data_ports["FJK2"]   = {"J"};
                gate_to_data_ports["FJK2P"]  = {"J"};
                gate_to_data_ports["FJK2S"]  = {"J", "TI"};
                gate_to_data_ports["FJK2SP"] = {"J", "TI"};
                gate_to_data_ports["FJK3"]   = {"J"};
                gate_to_data_ports["FJK3P"]  = {"J"};
                gate_to_data_ports["FJK3S"]  = {"J", "TI"};
                gate_to_data_ports["FJK3SP"] = {"J", "TI"};
                gate_to_data_ports["FDS2"]   = {"D"};
                gate_to_data_ports["FDS2L"]  = {"D"};
                gate_to_data_ports["FDS2LP"] = {"D"};
                gate_to_data_ports["FDS2P"]  = {"D"};
                gate_to_data_ports["FT2"]    = {"D"};
                gate_to_data_ports["FT2P"]   = {"D"};
                gate_to_data_ports["FT4"]    = {"D"};
                gate_to_data_ports["FT4P"]   = {"D"};
                gate_to_data_ports["FD2TS"]  = {"D", "RD"};
                gate_to_data_ports["FD2TSP"] = {"D", "RD"};
            }
            return gate_to_data_ports.at(sg->get_type()->get_name());
        }
    }    // namespace dataflow_utils
}    // namespace hal
