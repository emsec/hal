#include "utils/utils_xilinx_unisim.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/utilities/log.h"

#include <string>
#include <unordered_set>

namespace hal
{
    namespace gate_library_specific_utils
    {
        bool UtilsXilinxUnisim::is_sequential(Gate* sg) const
        {
            static std::unordered_set<std::string> supported = {"FDRE", "FDE", "FD", "FDR", "FDCE", "FDS", "FDPE", "FDSE"};    // , "LDCE"};, "RAMB18E1", "RAMB36E1", "RAM32M", "RAM32X1D"};

            if (supported.find(sg->get_type()->get_name()) != supported.end())
            {
                return true;
            }

            if (sg->get_type()->get_name().find("RAM") != std::string::npos)
            {
                //log_error("gl specifics", "need to implement the following gate_type: {} for gate: {}", sg->get_type()->get_name(), sg->get_name());
            }
            return false;
        }

        std::unordered_set<std::string> UtilsXilinxUnisim::get_control_input_pin_types(Gate* sg) const
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

        std::unordered_set<std::string> UtilsXilinxUnisim::get_clock_ports(Gate* sg) const
        {
            if (!is_sequential(sg))
            {
                log_error("gl specifics", "gate is not sequential: {}, type: {}", sg->get_name(), sg->get_type()->get_name());
                return std::unordered_set<std::string>();
            }

            static std::map<std::string, std::unordered_set<std::string>> gate_to_clock_ports;
            if (gate_to_clock_ports.empty())
            {
                gate_to_clock_ports["FDRE"] = {"C"};
                gate_to_clock_ports["FDE"]  = {"C"};
                gate_to_clock_ports["FD"]   = {"C"};
                gate_to_clock_ports["FDR"]  = {"C"};
                gate_to_clock_ports["FDCE"] = {"C"};
                gate_to_clock_ports["FDS"]  = {"C"};
                gate_to_clock_ports["FDPE"] = {"C"};
                gate_to_clock_ports["FDSE"] = {"C"};
                // gate_to_clock_ports["LDCE"] = {};

                gate_to_clock_ports["RAMB18E1"] = {"CLKARDCLK", "CLKBWRCLK"};
                gate_to_clock_ports["RAMB36E1"] = {"CLKARDCLK", "CLKBWRCLK"};
                gate_to_clock_ports["RAM32M"]   = {"WCLK"};
                gate_to_clock_ports["RAM32X1D"] = {"WCLK"};
            }

            return gate_to_clock_ports.at(sg->get_type()->get_name());
        }

        std::unordered_set<std::string> UtilsXilinxUnisim::get_enable_ports(Gate* sg) const
        {
            if (!is_sequential(sg))
            {
                log_error("gl specifics", "gate is not sequential: {}, type: {}", sg->get_name(), sg->get_type()->get_name());
                return std::unordered_set<std::string>();
            }

            static std::map<std::string, std::unordered_set<std::string>> gate_to_enable_ports;
            if (gate_to_enable_ports.empty())
            {
                gate_to_enable_ports["FDRE"] = {"CE"};
                gate_to_enable_ports["FDE"]  = {"CE"};
                gate_to_enable_ports["FD"]   = {};
                gate_to_enable_ports["FDR"]  = {};
                gate_to_enable_ports["FDCE"] = {"CE"};
                gate_to_enable_ports["FDS"]  = {};
                gate_to_enable_ports["FDPE"] = {"CE"};
                gate_to_enable_ports["FDSE"] = {"CE"};
                // gate_to_enable_ports["LDCE"] = {"GE"};

                gate_to_enable_ports["RAMB18E1"] = {"ENARDEN", "ENBWREN", "REGCEAREGCE", "REGCEB", "WEA(0)", "WEA(1)", "WEBWE(0)", "WEBWE(1)", "WEBWE(2)", "WEBWE(3)"};
                gate_to_enable_ports["RAMB36E1"] = {"ENARDEN",
                                                    "ENBWREN",
                                                    "REGCEAREGCE",
                                                    "REGCEB",
                                                    "WEA(0)",
                                                    "WEA(1)",
                                                    "WEA(2)",
                                                    "WEA(3)",
                                                    "WEBWE(0)",
                                                    "WEBWE(1)",
                                                    "WEBWE(2)",
                                                    "WEBWE(3)",
                                                    "WEBWE(4)",
                                                    "WEBWE(5)",
                                                    "WEBWE(6)",
                                                    "WEBWE(7)"};
                gate_to_enable_ports["RAM32M"]   = {"WE"};
                gate_to_enable_ports["RAM32X1D"] = {"WE"};
            }

            return gate_to_enable_ports.at(sg->get_type()->get_name());
        }

        std::unordered_set<std::string> UtilsXilinxUnisim::get_reset_ports(Gate* sg) const
        {
            if (!is_sequential(sg))
            {
                log_error("gl specifics", "gate is not sequential: {}, type: {}", sg->get_name(), sg->get_type()->get_name());
                return std::unordered_set<std::string>();
            }

            static std::map<std::string, std::unordered_set<std::string>> gate_to_reset_ports;
            if (gate_to_reset_ports.empty())
            {
                gate_to_reset_ports["FDRE"] = {"R"};
                gate_to_reset_ports["FDE"]  = {};
                gate_to_reset_ports["FD"]   = {};
                gate_to_reset_ports["FDR"]  = {"R"};
                gate_to_reset_ports["FDCE"] = {"CLR"};
                gate_to_reset_ports["FDS"]  = {};
                gate_to_reset_ports["FDPE"] = {"PRE"};
                gate_to_reset_ports["FDSE"] = {};
                // gate_to_reset_ports["LDCE"] = {"CLR"};

                gate_to_reset_ports["RAMB18E1"] = {"RSTRAMARSTRAM", "RSTRAMB", "RSTREGARSTREG", "RSTREGB"};
                gate_to_reset_ports["RAMB36E1"] = {"RSTRAMARSTRAM", "RSTRAMB", "RSTREGARSTREG", "RSTREGB"};
                gate_to_reset_ports["RAM32M"]   = {};
                gate_to_reset_ports["RAM32X1D"] = {};
            }

            return gate_to_reset_ports.at(sg->get_type()->get_name());
        }

        std::unordered_set<std::string> UtilsXilinxUnisim::get_data_ports(Gate* sg) const
        {
            if (!is_sequential(sg))
            {
                log_error("gl specifics", "gate is not sequential: {}, type: {}", sg->get_name(), sg->get_type()->get_name());
                return std::unordered_set<std::string>();
            }

            static std::map<std::string, std::unordered_set<std::string>> gate_to_data_ports;
            if (gate_to_data_ports.empty())
            {
                gate_to_data_ports["FDRE"] = {"D"};
                gate_to_data_ports["FDE"]  = {"D"};
                gate_to_data_ports["FD"]   = {"D"};
                gate_to_data_ports["FDR"]  = {"D"};
                gate_to_data_ports["FDCE"] = {"D"};
                gate_to_data_ports["FDS"]  = {"D"};
                gate_to_data_ports["FDPE"] = {"D"};
                gate_to_data_ports["FDSE"] = {"D"};
                // gate_to_data_ports["LDCE"] = {"D"};

                gate_to_data_ports["RAMB18E1"] = {
                    "ADDRARDADDR(0)",  "ADDRARDADDR(1)",  "ADDRARDADDR(10)", "ADDRARDADDR(11)", "ADDRARDADDR(12)", "ADDRARDADDR(13)", "ADDRARDADDR(2)", "ADDRARDADDR(3)",
                    "ADDRARDADDR(4)",  "ADDRARDADDR(5)",  "ADDRARDADDR(6)",  "ADDRARDADDR(7)",  "ADDRARDADDR(8)",  "ADDRARDADDR(9)",  "ADDRBWRADDR(0)", "ADDRBWRADDR(1)",
                    "ADDRBWRADDR(10)", "ADDRBWRADDR(11)", "ADDRBWRADDR(12)", "ADDRBWRADDR(13)", "ADDRBWRADDR(2)",  "ADDRBWRADDR(3)",  "ADDRBWRADDR(4)", "ADDRBWRADDR(5)",
                    "ADDRBWRADDR(6)",  "ADDRBWRADDR(7)",  "ADDRBWRADDR(8)",  "ADDRBWRADDR(9)",  "DIADI(0)",        "DIADI(1)",        "DIADI(10)",      "DIADI(11)",
                    "DIADI(12)",       "DIADI(13)",       "DIADI(14)",       "DIADI(15)",       "DIADI(2)",        "DIADI(3)",        "DIADI(4)",       "DIADI(5)",
                    "DIADI(6)",        "DIADI(7)",        "DIADI(8)",        "DIADI(9)",        "DIBDI(0)",        "DIBDI(1)",        "DIBDI(10)",      "DIBDI(11)",
                    "DIBDI(12)",       "DIBDI(13)",       "DIBDI(14)",       "DIBDI(15)",       "DIBDI(2)",        "DIBDI(3)",        "DIBDI(4)",       "DIBDI(5)",
                    "DIBDI(6)",        "DIBDI(7)",        "DIBDI(8)",        "DIBDI(9)",        "DIPADIP(0)",      "DIPADIP(1)",      "DIPBDIP(0)",     "DIPBDIP(1)"};

                gate_to_data_ports["RAMB36E1"] = {
                    "ADDRARDADDR(0)", "ADDRARDADDR(1)", "ADDRARDADDR(10)", "ADDRARDADDR(11)", "ADDRARDADDR(12)", "ADDRARDADDR(13)", "ADDRARDADDR(14)", "ADDRARDADDR(15)",
                    "ADDRARDADDR(2)", "ADDRARDADDR(3)", "ADDRARDADDR(4)",  "ADDRARDADDR(5)",  "ADDRARDADDR(6)",  "ADDRARDADDR(7)",  "ADDRARDADDR(8)",  "ADDRARDADDR(9)",

                    "ADDRBWRADDR(0)", "ADDRBWRADDR(1)", "ADDRBWRADDR(10)", "ADDRBWRADDR(11)", "ADDRBWRADDR(12)", "ADDRBWRADDR(13)", "ADDRBWRADDR(14)", "ADDRBWRADDR(15)",
                    "ADDRBWRADDR(2)", "ADDRBWRADDR(3)", "ADDRBWRADDR(4)",  "ADDRBWRADDR(5)",  "ADDRBWRADDR(6)",  "ADDRBWRADDR(7)",  "ADDRBWRADDR(8)",  "ADDRBWRADDR(9)",

                    "DIADI(0)",       "DIADI(1)",       "DIADI(10)",       "DIADI(11)",       "DIADI(12)",       "DIADI(13)",       "DIADI(14)",       "DIADI(15)",
                    "DIADI(16)",      "DIADI(17)",      "DIADI(18)",       "DIADI(19)",       "DIADI(2)",        "DIADI(20)",       "DIADI(21)",       "DIADI(22)",
                    "DIADI(23)",      "DIADI(24)",      "DIADI(25)",       "DIADI(26)",       "DIADI(27)",       "DIADI(28)",       "DIADI(29)",       "DIADI(3)",
                    "DIADI(30)",      "DIADI(31)",      "DIADI(4)",        "DIADI(5)",        "DIADI(6)",        "DIADI(7)",        "DIADI(8)",

                    "DIBDI(0)",       "DIBDI(1)",       "DIBDI(10)",       "DIBDI(11)",       "DIBDI(12)",       "DIBDI(13)",       "DIBDI(14)",       "DIBDI(15)",
                    "DIBDI(16)",      "DIBDI(17)",      "DIBDI(18)",       "DIBDI(19)",       "DIBDI(2)",        "DIBDI(20)",       "DIBDI(21)",       "DIBDI(22)",
                    "DIBDI(23)",      "DIBDI(24)",      "DIBDI(25)",       "DIBDI(26)",       "DIBDI(27)",       "DIBDI(28)",       "DIBDI(29)",       "DIBDI(3)",
                    "DIBDI(30)",      "DIBDI(31)",      "DIBDI(4)",        "DIBDI(5)",        "DIBDI(6)",        "DIBDI(7)",        "DIBDI(8)"};

                gate_to_data_ports["RAM32M"] = {"ADDRA(0)", "ADDRA(1)", "ADDRA(2)", "ADDRA(3)", "ADDRA(4)", "ADDRB(0)", "ADDRB(1)", "ADDRB(2)", "ADDRB(3)", "ADDRB(4)",
                                                "ADDRC(0)", "ADDRC(1)", "ADDRC(2)", "ADDRC(3)", "ADDRC(4)", "ADDRD(0)", "ADDRD(1)", "ADDRD(2)", "ADDRD(3)", "ADDRD(4)",
                                                "DIA(0)",   "DIA(1)",   "DIB(0)",   "DIB(1)",   "DIC(0)",   "DIC(1)",   "DID(0)",   "DID(1)"};

                gate_to_data_ports["RAM32X1D"] = {"A0", "A1", "A2", "A3", "A4", "D", "DPRA0", "DPRA1", "DPRA2", "DPRA3", "DPRA4"};
            }

            return gate_to_data_ports.at(sg->get_type()->get_name());
        }

        std::unordered_set<std::string> UtilsXilinxUnisim::get_set_ports(Gate* sg) const
        {
            if (!is_sequential(sg))
            {
                log_error("gl specifics", "gate is not sequential: {}, type: {}", sg->get_name(), sg->get_type()->get_name());
                return std::unordered_set<std::string>();
            }

            static std::map<std::string, std::unordered_set<std::string>> gate_to_set_ports;
            if (gate_to_set_ports.empty())
            {
                gate_to_set_ports["FDRE"] = {};
                gate_to_set_ports["FDE"]  = {};
                gate_to_set_ports["FD"]   = {};
                gate_to_set_ports["FDR"]  = {};
                gate_to_set_ports["FDCE"] = {};
                gate_to_set_ports["FDS"]  = {"S"};
                gate_to_set_ports["FDPE"] = {};
                gate_to_set_ports["FDSE"] = {"S"};
                // gate_to_set_ports["LDCE"] = {"D"};

                // TODO mot sure if RAM has any
                gate_to_set_ports["RAMB18E1"] = {};
                gate_to_set_ports["RAMB36E1"] = {};
                gate_to_set_ports["RAM32M"] = {};
                gate_to_set_ports["RAM32X1D"] = {};
            }

            return gate_to_set_ports.at(sg->get_type()->get_name());
        }

        std::unordered_set<std::string> UtilsXilinxUnisim::get_regular_outputs(Gate* sg) const 
        {
            if (!is_sequential(sg))
            {
                log_error("gl specifics", "gate is not sequential: {}, type: {}", sg->get_name(), sg->get_type()->get_name());
                return std::unordered_set<std::string>();
            }

            static std::map<std::string, std::unordered_set<std::string>> gate_to_regular_output_ports;
            if (gate_to_regular_output_ports.empty())
            {
                gate_to_regular_output_ports["FDRE"] = {"Q"};
                gate_to_regular_output_ports["FDE"]  = {"Q"};
                gate_to_regular_output_ports["FD"]   = {"Q"};
                gate_to_regular_output_ports["FDR"]  = {"Q"};
                gate_to_regular_output_ports["FDCE"] = {"Q"};
                gate_to_regular_output_ports["FDS"]  = {"Q"};
                gate_to_regular_output_ports["FDPE"] = {"Q"};
                gate_to_regular_output_ports["FDSE"] = {"Q"};
                // gate_to_regular_output_ports["LDCE"] = {"D"};

                // TODO
                gate_to_regular_output_ports["RAMB18E1"] = {};
                gate_to_regular_output_ports["RAMB36E1"] = {};
                gate_to_regular_output_ports["RAM32M"] = {};
                gate_to_regular_output_ports["RAM32X1D"] = {};
            }

            return gate_to_regular_output_ports.at(sg->get_type()->get_name());
        }

        std::unordered_set<std::string> UtilsXilinxUnisim::get_negated_outputs(Gate* sg) const 
        {
            if (!is_sequential(sg))
            {
                log_error("gl specifics", "gate is not sequential: {}, type: {}", sg->get_name(), sg->get_type()->get_name());
                return std::unordered_set<std::string>();
            }

            static std::map<std::string, std::unordered_set<std::string>> gate_to_negated_output_ports;
            if (gate_to_negated_output_ports.empty())
            {
                gate_to_negated_output_ports["FDRE"] = {};
                gate_to_negated_output_ports["FDE"]  = {};
                gate_to_negated_output_ports["FD"]   = {};
                gate_to_negated_output_ports["FDR"]  = {};
                gate_to_negated_output_ports["FDCE"] = {};
                gate_to_negated_output_ports["FDS"]  = {};
                gate_to_negated_output_ports["FDPE"] = {};
                gate_to_negated_output_ports["FDSE"] = {};
                // gate_to_negated_output_ports["LDCE"] = {};

                // TODO
                gate_to_negated_output_ports["RAMB18E1"] = {};
                gate_to_negated_output_ports["RAMB36E1"] = {};
                gate_to_negated_output_ports["RAM32M"] = {};
                gate_to_negated_output_ports["RAM32X1D"] = {};
            }

            return gate_to_negated_output_ports.at(sg->get_type()->get_name());
        }

    }    // namespace gate_library_specific_utils
}    // namespace hal