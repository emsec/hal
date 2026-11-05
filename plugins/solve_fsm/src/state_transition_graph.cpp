// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "solve_fsm/state_transition_graph.h"

#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"

#include <bitset>
#include <fstream>
#include <set>
#include <sstream>

namespace hal
{
    namespace solve_fsm
    {
        namespace
        {
            /**
             * Print a value in the requested base, zero-padded to the given bit-size when printed as binary.
             */
            std::string format_value(const u64 value, const u32 size, const u32 base)
            {
                if (base == 2)
                {
                    return std::bitset<64>(value).to_string().substr(64 - size, 64);
                }
                return std::to_string(value);
            }

            /**
             * A Boolean function rendered either as the value it evaluates to or, if it still depends on the inputs of
             * the FSM, as the function itself.
             */
            std::string format_function(const BooleanFunction& bf, const u32 base)
            {
                if (bf.is_constant())
                {
                    if (const auto res = bf.get_constant_value_u64(); res.is_ok())
                    {
                        return format_value(res.get(), bf.size(), base);
                    }
                }
                return bf.to_string();
            }

            /**
             * The name and ID of a netlist element, as printed in the legend of the text representation.
             */
            template<typename T>
            std::string format_element(const T* element)
            {
                return "'" + element->get_name() + "' with ID " + std::to_string(element->get_id());
            }
        }    // namespace

        u32 StateTransitionGraph::get_state_size() const
        {
            return state_register.size();
        }

        Result<std::string> StateTransitionGraph::generate_dot_graph(const std::filesystem::path& graph_path, const u32 max_condition_length, const u32 base) const
        {
            std::string graph_str = "digraph {\ncomment=\"created by HAL plugin solve_fsm\"\n";

            const auto format_state = [this, base](const u64 state) -> std::string {
                if (base == 2)
                {
                    return std::bitset<64>(state).to_string().substr(64 - this->get_state_size(), 64);
                }
                return std::to_string(state);
            };

            if (base != 2 && base != 10)
            {
                return ERR("failed to generate DOT graph: base " + std::to_string(base) + "not implemented.");
            }

            // states only carry an explicit node statement if there is something to annotate them with
            if (!outputs.empty())
            {
                std::set<u64> states;
                for (const auto& [org, successors] : transitions)
                {
                    states.insert(org);
                    for (const auto& [suc, _] : successors)
                    {
                        states.insert(suc);
                    }
                }

                for (const auto& state : states)
                {
                    std::string label = format_state(state);

                    if (const auto it = outputs.find(state); it != outputs.end())
                    {
                        for (const auto& [name, bf] : it->second)
                        {
                            // an output that only depends on the state has a constant value, one that also depends on
                            // the inputs of the FSM is printed as the function that it is
                            std::string value;
                            if (bf.is_constant())
                            {
                                if (const auto val_res = bf.get_constant_value_u64(); val_res.is_ok())
                                {
                                    value = (base == 2) ? std::bitset<64>(val_res.get()).to_string().substr(64 - bf.size(), 64) : std::to_string(val_res.get());
                                }
                            }
                            if (value.empty())
                            {
                                value = bf.to_string().substr(0, max_condition_length);
                            }

                            label += "\\n" + name + " = " + value;
                        }
                    }

                    graph_str += format_state(state) + " [label=\"" + label + "\"];\n";
                }
            }

            for (const auto& [org, successors] : transitions)
            {
                for (const auto& [suc, cond] : successors)
                {
                    const std::string start_name = format_state(org);
                    const std::string end_name   = format_state(suc);

                    graph_str +=
                        start_name + " -> " + end_name + "[label=\"" + cond.to_string().substr(0, max_condition_length) + "\", weight=\"" + cond.to_string().substr(0, max_condition_length) + "\"];\n";
                    ;
                }
            }

            graph_str += "}";

            // write to file
            if (!graph_path.empty())
            {
                std::ofstream ofs(graph_path);
                if (!ofs.is_open())
                {
                    return ERR("failed to generate DOT graph: could not open file '" + graph_path.string() + "' for writing.");
                }
                ofs << graph_str;
                ofs.close();

            }

            return OK(graph_str);
        }
        Result<std::string> StateTransitionGraph::to_string(const u32 base) const
        {
            if (base != 2 && base != 10)
            {
                return ERR("failed to print state transition graph: base " + std::to_string(base) + " is not implemented.");
            }

            const u32 state_size = get_state_size();
            std::stringstream ss;

            ss << "FSM with " << state_size << " state bits and " << transitions.size() << " reachable states" << std::endl << std::endl;

            // the legend maps a state and its outputs back to the netlist elements they are made of
            ss << "state register (least significant bit first):" << std::endl;
            for (u32 i = 0; i < state_register.size(); i++)
            {
                ss << "  bit " << i << ": " << format_element(state_register.at(i)) << std::endl;
            }
            ss << std::endl;

            if (!output_nets.empty())
            {
                ss << "outputs (least significant bit first):" << std::endl;
                for (const auto& [name, nets] : output_nets)
                {
                    ss << "  " << name << ":";
                    for (u32 i = 0; i < nets.size(); i++)
                    {
                        ss << (i == 0 ? " " : ", ") << format_element(nets.at(i));
                    }
                    ss << std::endl;
                }
                ss << std::endl;
            }

            // the Boolean functions refer to nets by a variable derived from the net ID, which matches no name in the
            // netlist, so every variable that appears anywhere below is resolved here
            std::set<std::string> variables;
            for (const auto& [state, successors] : transitions)
            {
                for (const auto& [successor, condition] : successors)
                {
                    const auto vars = condition.get_variable_names();
                    variables.insert(vars.begin(), vars.end());
                }
            }
            for (const auto& [state, state_outputs] : outputs)
            {
                for (const auto& [name, bf] : state_outputs)
                {
                    const auto vars = bf.get_variable_names();
                    variables.insert(vars.begin(), vars.end());
                }
            }

            if (!variables.empty())
            {
                ss << "nets referenced in the Boolean functions below:" << std::endl;
                for (const auto& variable : variables)
                {
                    ss << "  " << variable << ": ";
                    if (netlist == nullptr)
                    {
                        ss << "unknown, the netlist is not available" << std::endl;
                        continue;
                    }

                    if (const auto net_res = BooleanFunctionNetDecorator::get_net_from(netlist, variable); net_res.is_ok())
                    {
                        ss << "'" << net_res.get()->get_name() << "' with ID " << net_res.get()->get_id() << std::endl;
                    }
                    else
                    {
                        ss << "not a net of this netlist" << std::endl;
                    }
                }
                ss << std::endl;
            }

            for (const auto& [state, successors] : transitions)
            {
                ss << "state " << format_value(state, state_size, base) << std::endl;

                if (const auto it = outputs.find(state); it != outputs.end() && !it->second.empty())
                {
                    ss << "  outputs:" << std::endl;
                    for (const auto& [name, bf] : it->second)
                    {
                        ss << "    " << name << " = " << format_function(bf, base) << std::endl;
                    }
                }

                ss << "  transitions:" << std::endl;
                if (successors.empty())
                {
                    ss << "    none" << std::endl;
                }
                for (const auto& [successor, condition] : successors)
                {
                    ss << "    to " << format_value(successor, state_size, base) << " if " << condition.to_string() << std::endl;
                }

                ss << std::endl;
            }

            return OK(ss.str());
        }

        Result<std::monostate> StateTransitionGraph::write_txt(const std::filesystem::path& file_path, const u32 base) const
        {
            auto res = to_string(base);
            if (res.is_error())
            {
                return ERR_APPEND(res.get_error(), "failed to write state transition graph to '" + file_path.string() + "'.");
            }

            std::ofstream ofs(file_path);
            if (!ofs.is_open())
            {
                return ERR("failed to write state transition graph: could not open file '" + file_path.string() + "' for writing.");
            }
            ofs << res.get();
            ofs.close();

            return OK({});
        }
    }    // namespace solve_fsm
}    // namespace hal
