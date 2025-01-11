#include "module_identification/candidates/verified_candidate.h"

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/utilities/utils.h"
#include "module_identification/types/candidate_types.h"
#include "module_identification/utils/utils.h"

#include <string>
namespace hal
{
    namespace module_identification
    {
        VerifiedCandidate::VerifiedCandidate(const std::vector<std::vector<Net*>>& operands,
                                             const std::vector<Net*>& output_nets,
                                             const std::vector<Net*>& control_signals,
                                             const std::vector<std::map<Net*, BooleanFunction::Value>>& control_signal_mappings,
                                             const std::map<std::map<Net*, BooleanFunction::Value>, BooleanFunction>& word_level_operations,
                                             const std::vector<Gate*>& gates,
                                             const std::vector<Gate*>& base_gates,
                                             const std::map<std::string, std::string>& additional_data,
                                             const std::set<CandidateType>& types)
            : m_operands{operands}, m_output_nets{output_nets}, m_control_signals{control_signals}, m_control_signal_mappings{control_signal_mappings}, m_word_level_operations{word_level_operations},
              m_gates{gates}, m_base_gates{base_gates}, m_total_input_nets{get_input_nets(gates)}, m_total_output_nets{get_output_nets(gates)}, m_types{types}, m_verified{!types.empty()},
              m_additional_data{additional_data} {};

        VerifiedCandidate::VerifiedCandidate()
        {
            m_operands                = {};
            m_control_signals         = {};
            m_control_signal_mappings = {};
            m_word_level_operations   = {};
            m_output_nets             = {};
            m_gates                   = {};
            m_base_gates              = {};
            m_types                   = {};
            m_verified                = false;
            m_additional_data         = {};
        };

        bool VerifiedCandidate::is_verified() const
        {
            return m_verified;
        }

        std::string VerifiedCandidate::get_candidate_info() const
        {
            std::string result = "";
            result += "Verified: " + std::to_string(m_verified) + "\n";
            result += "Candidate Types: \n";
            for (const auto& t : m_types)
            {
                result += "\t" + enum_to_string(t) + "\n";
            }

            result += "GATES [" + std::to_string(m_gates.size()) + "]" + ": \n";
            for (const auto& g : m_gates)
            {
                result += "\t" + g->get_name() + " / " + std::to_string(g->get_id()) + "\n";
            }

            for (u32 op_idx = 0; op_idx < m_operands.size(); op_idx++)
            {
                const auto& nets = m_operands.at(op_idx);
                result += "OPERAND " + std::to_string(op_idx) + " [" + std::to_string(nets.size()) + "]" + ": \n";
                for (const auto& n : nets)
                {
                    result += "\t" + n->get_name() + " / " + std::to_string(n->get_id()) + "\n";
                }
            }
            result += "OUTPUT: [" + std::to_string(m_output_nets.size()) + "]\n";
            for (auto n : m_output_nets)
            {
                result += "\t" + n->get_name() + " / " + std::to_string(n->get_id()) + "\n";
            }
            result += "CONTROL SIGNALS: [" + std::to_string(m_control_signals.size()) + "]\n";
            result += "\t" + utils::join(", ", m_control_signals, [](const auto& n) { return n->get_name(); }) + "\n";
            result += "CONTROL MAPPINGS: [" + std::to_string(m_control_signal_mappings.size()) + "]\n";
            for (const auto& cm : m_control_signal_mappings)
            {
                for (const auto& [n, v] : cm)
                {
                    result += "\t" + n->get_name() + " / " + std::to_string(n->get_id()) + " : " + std::to_string(v) + "\n";
                }
                result += "--------------------------------------------------\n";
            }
            result += "ADDITIONAL DATA: \n";
            for (const auto& [key, data] : m_additional_data)
            {
                result += "\t" + key + ": " + data + "\n";
            }

            return result;
        }

        Result<VerifiedCandidate> VerifiedCandidate::merge(const std::vector<VerifiedCandidate>& candidates)
        {
            std::set<CandidateType> merged_types;

            std::vector<Net*> merged_control_signals = candidates.front().m_control_signals;
            std::vector<std::map<Net*, BooleanFunction::Value>> merged_control_signal_mappings;
            std::map<std::map<Net*, BooleanFunction::Value>, BooleanFunction> merged_word_level_operations;

            std::vector<std::vector<Net*>> merged_operands;

            std::vector<Gate*> merged_gates      = candidates.front().m_gates;
            std::vector<Gate*> merged_base_gates = candidates.front().m_base_gates;

            // TODO maybe add sanity check that all output nets match
            std::vector<Net*> merged_output_nets = candidates.front().m_output_nets;
            // std::vector<Net*> merged_total_input_nets  = candidates.front().m_total_input_nets;
            // std::vector<Net*> merged_total_output_nets = candidates.front().m_total_output_nets;

            std::map<std::string, std::string> merged_additional_data;

            // helper variable
            std::set<Net*> assigned_to_operand;

            for (const auto& c : candidates)
            {
                merged_types.insert(c.m_types.begin(), c.m_types.end());

                for (const auto& cm : c.m_control_signal_mappings)
                {
                    if (std::find(merged_control_signal_mappings.begin(), merged_control_signal_mappings.end(), cm) != merged_control_signal_mappings.end())
                    {
                        return ERR("cannot merge verified candidates with overlapping control signal mapping");
                    }

                    merged_control_signal_mappings.push_back(cm);
                    merged_word_level_operations.insert({cm, c.m_word_level_operations.at(cm)});
                }

                for (const auto& nets : c.m_operands)
                {
                    // check whether identical operand
                    if (const auto it = std::find(merged_operands.begin(), merged_operands.end(), nets); it != merged_operands.end())
                    {
                        continue;
                    }

                    bool is_not_intersecting = true;
                    for (const auto& n : nets)
                    {
                        if (n->is_gnd_net() || n->is_vcc_net())
                        {
                            continue;
                        }

                        if (assigned_to_operand.find(n) != assigned_to_operand.end())
                        {
                            log_warning("module_identification", "found intersecting operand at net {} with ID {}", n->get_name(), n->get_id());
                            is_not_intersecting = false;
                            break;
                        }
                    }

                    if (is_not_intersecting)
                    {
                        merged_operands.push_back(nets);
                        for (const auto& n : nets)
                        {
                            if (n->is_gnd_net() || n->is_vcc_net())
                            {
                                continue;
                            }
                            assigned_to_operand.insert(n);
                        }
                    }
                }
            }

            return OK(VerifiedCandidate(merged_operands,
                                        merged_output_nets,
                                        merged_control_signals,
                                        merged_control_signal_mappings,
                                        merged_word_level_operations,
                                        merged_gates,
                                        merged_base_gates,
                                        merged_additional_data,
                                        merged_types));
        }

        std::string VerifiedCandidate::get_name() const
        {
            if (m_types.empty())
            {
                return "NONE_CARRY_CHAIN";
            }

            return utils::join("_", m_types);
        }

        BooleanFunction VerifiedCandidate::get_merged_word_level_operation() const
        {
            // TODO add the ommission of the UNKNOWN OPERATION, if there is an exhaustive amount of control mappings
            BooleanFunction res = BooleanFunction::Var("UNKNOWN_OPERATION", m_output_nets.size());

            for (const auto& [ctrl_mapping, bf] : m_word_level_operations)
            {
                if (ctrl_mapping.empty())
                {
                    if (m_word_level_operations.size() != 1)
                    {
                        log_error("module_identification", "Found a module with {} word level operations but at least one being unconditional!", m_word_level_operations.size());
                    }
                    else
                    {
                        res = bf;
                    }
                    break;
                }

                BooleanFunction cond;
                for (const auto& [net, val] : ctrl_mapping)
                {
                    auto net_var  = BooleanFunctionNetDecorator(*net).get_boolean_variable();
                    auto new_cond = BooleanFunction::Eq(std::move(net_var), BooleanFunction::Const({val}), 1).get();

                    if (cond.is_empty())
                    {
                        cond = new_cond;
                    }
                    else
                    {
                        cond = BooleanFunction::And(std::move(cond), std::move(new_cond), 1).get();
                    }
                }

                res = BooleanFunction::Ite(std::move(cond), bf.clone(), res.clone(), res.size()).get();
            }

            return res;
        }
    }    // namespace module_identification
}    // namespace hal