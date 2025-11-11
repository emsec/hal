#include "module_identification/api/result.h"

#include "hal_core/netlist/decorators/boolean_function_decorator.h"
#include "hal_core/utilities/utils.h"
#include "module_identification/processing/post_processing.h"
#include "module_identification/types/candidate_types.h"
#include "module_identification/utils/utils.h"

#include <algorithm>
#include <iterator>
#include <tuple>
#include <vector>

namespace hal
{
    namespace module_identification
    {
        Result::Result(Netlist* nl, const std::vector<std::pair<BaseCandidate, VerifiedCandidate>>& result, const std::string& timing_stats_json)
            : m_netlist{nl}, m_candidates{result}, m_timing_stats_json{timing_stats_json}
        {
        }

        Netlist* Result::get_netlist() const
        {
            return m_netlist;
        }

        std::map<u32, std::vector<Gate*>> Result::get_verified_candidate_gates() const
        {
            std::map<u32, std::vector<Gate*>> result;

            for (u32 idx = 0; idx < m_candidates.size(); idx++)
            {
                const auto& [base_candidate, verified_candidate] = m_candidates[idx];
                if (verified_candidate.is_verified())
                {
                    result.insert(std::make_pair(idx, verified_candidate.m_gates));
                }
            }
            return result;
        }

        std::map<u32, VerifiedCandidate> Result::get_verified_candidates() const
        {
            std::map<u32, VerifiedCandidate> result;
            for (u32 idx = 0; idx < m_candidates.size(); idx++)
            {
                const auto& [base_candidate, verified_candidate] = m_candidates[idx];
                if (verified_candidate.is_verified())
                {
                    result.insert(std::make_pair(idx, verified_candidate));
                }
            }
            return result;
        }

        std::map<u32, std::vector<Gate*>> Result::get_candidate_gates() const
        {
            std::map<u32, std::vector<Gate*>> result;
            for (u32 idx = 0; idx < m_candidates.size(); idx++)
            {
                const auto& [base_candidate, verified_candidate] = m_candidates[idx];
                result.insert(std::make_pair(idx, verified_candidate.m_gates));
            }
            return result;
        }

        std::map<u32, VerifiedCandidate> Result::get_candidates() const
        {
            std::map<u32, VerifiedCandidate> result;
            for (u32 idx = 0; idx < m_candidates.size(); idx++)
            {
                const auto& [base_candidate, verified_candidate] = m_candidates[idx];
                result.insert(std::make_pair(idx, verified_candidate));
            }
            return result;
        }

        hal::Result<std::vector<Gate*>> Result::get_candidate_gates_by_id(const u32 id) const
        {
            if (id >= m_candidates.size())
            {
                return ERR("cannot get candidate with id " + std::to_string(id));
            }

            const auto& [base_candidate, verified_candidate] = m_candidates[id];
            if (verified_candidate.is_verified())
            {
                return OK(verified_candidate.m_gates);
            }
            else
            {
                return OK(base_candidate.m_gates);
            }
        }

        hal::Result<VerifiedCandidate> Result::get_candidate_by_id(const u32 id) const
        {
            if (id >= m_candidates.size())
            {
                return ERR("cannot get candidate with id " + std::to_string(id));
            }
            auto [base_candidate, verified_candidate] = m_candidates[id];
            return OK(verified_candidate);
        }

        std::set<Gate*> Result::get_all_gates() const
        {
            std::set<Gate*> result;
            for (const auto& [base_candidate, verified_candidate] : m_candidates)
            {
                if (verified_candidate.is_verified())
                {
                    std::copy(verified_candidate.m_gates.begin(), verified_candidate.m_gates.end(), std::inserter(result, result.end()));
                }
                else
                {
                    std::copy(base_candidate.m_gates.begin(), base_candidate.m_gates.end(), std::inserter(result, result.end()));
                }
            }
            return result;
        }

        std::set<Gate*> Result::get_all_verified_gates() const
        {
            std::set<Gate*> result;
            for (const auto& [base_candidate, verified_candidate] : m_candidates)
            {
                if (verified_candidate.is_verified())
                {
                    std::copy(verified_candidate.m_gates.begin(), verified_candidate.m_gates.end(), std::inserter(result, result.end()));
                }
            }
            return result;
        }

        namespace
        {
            std::pair<std::map<Gate*, std::vector<u32>>, std::map<Gate*, std::vector<u32>>> check_for_conflicting_gates(const std::vector<std::pair<BaseCandidate, VerifiedCandidate>>& candidates)
            {
                std::map<Gate*, std::vector<u32>> gate_to_candidates;
                std::map<Gate*, std::vector<u32>> conflicts;

                u32 candidate_id = 0;
                for (auto [base_cand, verified_cand] : candidates)
                {
                    if (verified_cand.is_verified())
                    {
                        for (auto gate : verified_cand.m_gates)
                        {
                            if (gate_to_candidates.find(gate) == gate_to_candidates.end())
                            {
                                gate_to_candidates.insert(std::make_pair(gate, std::vector<u32>()));
                            }
                            gate_to_candidates[gate].push_back(candidate_id);
                        }
                    }
                    else
                    {
                        for (auto cur_gate : base_cand.m_gates)
                        {
                            if (gate_to_candidates.find(cur_gate) == gate_to_candidates.end())
                            {
                                gate_to_candidates.insert(std::make_pair(cur_gate, std::vector<u32>()));
                            }
                            gate_to_candidates[cur_gate].push_back(candidate_id);
                        }
                    }
                    candidate_id++;
                }

                for (auto [gate, list] : gate_to_candidates)
                {
                    if (list.size() > 1)
                    {
                        conflicts.insert(std::make_pair(gate, list));
                    }
                }

                return {gate_to_candidates, conflicts};
            }

            std::pair<Gate*, u32> get_possible_conflict(const std::map<Gate*, std::vector<u32>>& conflicts)
            {
                // NOTE: naming, both times they are called current follwoer oder current conflict, there might be better names
                for (auto [gate, conflicting_candidates] : conflicts)
                {
                    std::set<u32> successor_conflicts;
                    for (const auto& ep : gate->get_fan_out_endpoints())
                    {
                        for (const auto& dest : ep->get_net()->get_destinations())
                        {
                            if (dest->get_gate() == nullptr)
                            {
                                continue;
                            }
                            if (conflicts.find(dest->get_gate()) == conflicts.end())
                            {
                                continue;
                            }
                            const auto& dest_conflicts = conflicts.at(dest->get_gate());
                            for (u32 dest_conflict : dest_conflicts)
                            {
                                successor_conflicts.insert(dest_conflict);
                            }
                        }
                    }

                    // check whether there is a candidate where this gate causes a conflict that does not also include a conflicting successor of the gate
                    for (u32 conflict_id : conflicting_candidates)
                    {
                        if (std::find(successor_conflicts.begin(), successor_conflicts.end(), conflict_id) == successor_conflicts.end())
                        {
                            // found free id without conflicts
                            return std::make_pair(gate, conflict_id);
                        }
                    }
                }
                // couldnt find a free follower
                log_error("module_identification",
                          "could not resolve conflicts in duplicate gates due to cyclic dependency. Continuing with following broken gate: {}",
                          conflicts.begin()->first->get_name());

                return std::make_pair(conflicts.begin()->first, conflicts.begin()->second[0]);
            }

            void resolve_conflicts_by_cloning(Netlist* nl,
                                              std::map<Gate*, std::vector<u32>>& conflicts,
                                              std::map<Gate*, std::vector<u32>>& gate_to_candidates,
                                              std::vector<std::pair<BaseCandidate, VerifiedCandidate>>& candidates)
            {
                // find an order for gates that prioritizes gates at the end
                while (conflicts.size() > 0)
                {
                    std::pair<Gate*, u32> available_conflict = get_possible_conflict(conflicts);

                    // TODO remove debug printing
                    // std::cout << "Dealing with conflict " << available_conflict.first->get_name() << " in candidate " << available_conflict.second << std::endl;

                    auto gate              = available_conflict.first;
                    auto current_candidate = available_conflict.second;

                    u32 new_gate_id           = nl->get_unique_gate_id();
                    std::string new_gate_name = gate->get_name() + "_CLONE_" + std::to_string(gate->get_id()) + "_" + std::to_string(new_gate_id);
                    Gate* new_gate            = nl->create_gate(new_gate_id, gate->get_type(), new_gate_name);

                    // copy boolean functions
                    for (const auto& [pin, bf] : gate->get_boolean_functions())
                    {
                        new_gate->add_boolean_function(pin, bf);
                    }

                    // copy all sources from the conflicting gate to the new_gate
                    for (const auto& ep : gate->get_fan_in_endpoints())
                    {
                        ep->get_net()->add_destination(new_gate, ep->get_pin());
                    }

                    // copy data container
                    new_gate->set_data_map(gate->get_data_map());

                    // remove old gate from module and replace with new one
                    std::vector<Gate*>* relevant_vector;
                    auto& [base_candidate, verified_candidate] = candidates[current_candidate];
                    if (verified_candidate.is_verified())
                    {
                        relevant_vector = &(verified_candidate.m_gates);
                    }
                    else
                    {
                        relevant_vector = &(base_candidate.m_gates);
                    }
                    std::vector<Gate*>::iterator position = std::find(relevant_vector->begin(), relevant_vector->end(), gate);
                    if (position != relevant_vector->end())
                    {
                        relevant_vector->erase(position);
                    }
                    else
                    {
                        log_error("module_identification", "trying to erase gate {} / {} from candidate that the gate is not part of", gate->get_id(), gate->get_name());
                    }
                    relevant_vector->push_back(new_gate);

                    gate_to_candidates.insert({new_gate, {current_candidate}});

                    // remove candidate id from old gate
                    std::vector<u32>::iterator old_id_position = std::find(gate_to_candidates[gate].begin(), gate_to_candidates[gate].end(), current_candidate);
                    if (old_id_position != gate_to_candidates[gate].end())
                    {
                        gate_to_candidates[gate].erase(old_id_position);
                    }
                    else
                    {
                        log_error("module_identification", "trying to erase gate {} / {} from candidate that the gate is not part of", gate->get_id(), gate->get_name());
                    }

                    // find all destinations of the conflicting gate that lead to a gate inside the candidate
                    std::map<GatePin*, std::map<Gate*, std::vector<GatePin*>>> new_destinations;
                    for (const auto& ep : gate->get_fan_out_endpoints())
                    {
                        for (const auto& dest : ep->get_net()->get_destinations())
                        {
                            if (dest->get_gate() == nullptr)
                            {
                                continue;
                            }

                            // Check whether destination is part of the candidate where we are cloning in
                            auto dest_gate_mod = gate_to_candidates.find(dest->get_gate());
                            if (dest_gate_mod == gate_to_candidates.end())
                            {
                                // destination is not part of any module
                                // what do we do here?
                                // NOTE: For now we do nothing, that way only the orignal gate keeps this connection.

                                // TODO remove debug printing
                                // std::cout << dest->get_gate()->get_name() << " not part of any module" << std::endl;

                                continue;
                            }

                            const auto& dest_mod_vec = dest_gate_mod->second;
                            if (std::find(dest_mod_vec.begin(), dest_mod_vec.end(), current_candidate) == dest_mod_vec.end())
                            {
                                // dest is outside this module so no need to attach
                                // TODO remove debug printing
                                // std::cout << dest->get_gate()->get_name() << " not part of conflict module" << std::endl;
                                continue;
                            }

                            // safe destination inside the same candidate to later add to the cloned net/gate
                            new_destinations[ep->get_pin()][dest->get_gate()].push_back(dest->get_pin());

                            // TODO remove debug printing
                            // std::cout << "Adding dst " << dest->get_gate()->get_name() << " / " << dest->get_pin()->get_name() << std::endl;

                            // remove the new destination from the conflicting gate
                            ep->get_net()->remove_destination(dest);
                        }

                        // NOTE "!=" instead of "=="
                        // NOTE naming, we usually use camel_case for variables
                        // substitute net in verified cand if it is an output net
                        if (verified_candidate.is_verified()
                            && (std::find(verified_candidate.m_output_nets.begin(), verified_candidate.m_output_nets.end(), ep->get_net()) != verified_candidate.m_output_nets.end()))
                        {
                            // net is output net

                            // create new net
                            u32 new_net_id           = nl->get_unique_net_id();
                            std::string new_net_name = "n" + std::to_string(new_net_id) + "_OUTPUT";
                            Net* new_net             = nl->create_net(new_net_id, new_net_name);

                            // add source to new net
                            new_net->add_source(new_gate, ep->get_pin());
                            // replace relevant net
                            std::replace(verified_candidate.m_output_nets.begin(), verified_candidate.m_output_nets.end(), ep->get_net(), new_net);
                        }
                    }

                    // create new_nets and connect to new_gate and new_destinations
                    for (const auto& [src_pin, destinations] : new_destinations)
                    {
                        Net* new_net;
                        if (auto fan_out_net = new_gate->get_fan_out_net(src_pin); fan_out_net != nullptr)
                        {
                            new_net = fan_out_net;
                        }
                        else
                        {
                            u32 new_net_id           = nl->get_unique_net_id();
                            std::string new_net_name = "n" + std::to_string(new_net_id) + "_CLONED";
                            new_net                  = nl->create_net(new_net_id, new_net_name);

                            if (!new_net->add_source(new_gate, src_pin))
                            {
                                log_error("module_identification",
                                          "failed to add source to net {} with ID {} at gate {} with ID {} and pin {}",
                                          new_net->get_name(),
                                          new_net->get_id(),
                                          new_gate->get_name(),
                                          new_gate->get_id(),
                                          src_pin->get_name());
                            }
                        }

                        // TODO remove
                        // std::cout << "Created output net " << new_net->get_name() << " with ID " << new_net->get_id() << " at gate " << new_gate->get_name() << " with ID " << new_gate()->get_id()
                        //           << " and pin " << src_pin->get_name() << std::endl;

                        for (const auto& [dest_gate, dest_pins] : destinations)
                        {
                            for (const auto& dest_pin : dest_pins)
                            {
                                if (!new_net->add_destination(dest_gate, dest_pin))
                                {
                                    log_error("module_identification",
                                              "failed to add destination to net {} with ID {} at gate {} with ID {} and pin {}",
                                              new_net->get_name(),
                                              new_net->get_id(),
                                              dest_gate->get_name(),
                                              dest_gate->get_id(),
                                              dest_pin->get_name());
                                }
                            }
                        }
                    }

                    // remove the resolved conflict
                    std::vector<u32>::iterator conflict_position = std::find(conflicts[gate].begin(), conflicts[gate].end(), current_candidate);
                    if (conflict_position != conflicts[gate].end())
                    {
                        conflicts[gate].erase(conflict_position);
                    }
                    if (conflicts[gate].size() <= 1)
                    {
                        conflicts.erase(gate);
                    }
                }

                return;
            }

        }    // namespace

        hal::Result<std::monostate> Result::create_modules_in_netlist()
        {
            // search for base candidates that were not verified but are a subset of another verified candidate
            // these candidates do not need to have a own module and therefore dont have to be cloned
            std::vector<std::pair<BaseCandidate, VerifiedCandidate>> filtered_candidates;

            for (u32 i = 0; i < m_candidates.size(); i++)
            {
                const auto& [bi, vi] = m_candidates.at(i);
                if (vi.is_verified())
                {
                    filtered_candidates.push_back(m_candidates.at(i));
                    continue;
                }

                bool is_subset = false;
                for (u32 j = 0; i < m_candidates.size(); j++)
                {
                    if (i == j)
                    {
                        continue;
                    }

                    const auto& [bj, vj] = m_candidates.at(j);

                    if (!vj.is_verified())
                    {
                        continue;
                    }

                    if (utils::is_subset(bi.m_gates, vj.m_gates))
                    {
                        is_subset = true;
                        break;
                    }
                }

                if (!is_subset)
                {
                    filtered_candidates.push_back(m_candidates.at(i));
                }
            }

            auto [gate_to_candidates, conflicts] = check_for_conflicting_gates(filtered_candidates);

            resolve_conflicts_by_cloning(m_netlist, conflicts, gate_to_candidates, filtered_candidates);

            std::map<std::string, u32> type_counter;

            for (u32 candidate_idx = 0; candidate_idx < filtered_candidates.size(); candidate_idx++)
            {
                auto& [base_candidate, selected_candidate] = filtered_candidates.at(candidate_idx);

                const std::string candidate_name = selected_candidate.get_name();

                if (type_counter.find(candidate_name) == type_counter.end())
                {
                    type_counter.insert(std::make_pair(candidate_name, 0));
                }

                const auto mod_gates = selected_candidate.is_verified() ? selected_candidate.m_gates : selected_candidate.m_base_gates;

                auto mod = m_netlist->create_module(candidate_name + "_" + std::to_string(type_counter[candidate_name]), m_netlist->get_top_module(), mod_gates);
                type_counter[candidate_name]++;

                std::set<u32> ctrl_mapping_values;
                for (const auto& cm : selected_candidate.m_control_signal_mappings)
                {
                    u32 ctrl_val = 0;
                    for (const auto& [net, val] : cm)
                    {
                        ctrl_val = (ctrl_val << 1) + ((val == BooleanFunction::ONE) ? 1 : 0);
                    }

                    ctrl_mapping_values.insert(ctrl_val);
                }

                mod->set_data("ModuleIdentification", "VERIFIED_CANDIDATE_ID", "String", std::to_string(candidate_idx));

                mod->set_data("ModuleIdentification", "VERIFIED_TYPES", "String", utils::join(", ", selected_candidate.m_types));

                mod->set_data("ModuleIdentification", "CTRL_MAPPINGS", "String", utils::join(", ", ctrl_mapping_values));

                std::string word_level_operation_str = "";
                for (const auto& [cm, bf] : selected_candidate.m_word_level_operations)
                {
                    u32 ctrl_val = 0;
                    for (const auto& [net, val] : cm)
                    {
                        ctrl_val = (ctrl_val << 1) + ((val == BooleanFunction::ONE) ? 1 : 0);
                    }

                    word_level_operation_str += std::to_string(ctrl_val) + ": " + bf.to_string() + "\n";
                }

                mod->set_data("ModuleIdentification", "OPERATIONS", "String", word_level_operation_str);

                // add operands to module

                const std::vector<std::string> op_names = {"A", "B", "C", "D", "E", "F", "G", "H", "J", "K", "L", "M", "N", "P", "Q", "R", "S", "T", "U", "V", "W"};

                if (selected_candidate.m_operands.size() > op_names.size())
                {
                    return ERR("cannot create modules: encountered candidate with more operands than operand names");
                }

                std::map<std::string, std::vector<Net*>> named_operands;
                for (u32 op_idx = 0; op_idx < selected_candidate.m_operands.size(); op_idx++)
                {
                    named_operands.insert({op_names.at(op_idx), selected_candidate.m_operands.at(op_idx)});
                }

                // NOTE: this is a work around since modules do not allow for a net to lead to multiple pins
                std::map<Net*, std::map<std::string, std::vector<u32>>> nets_to_indices;
                for (const auto& [name, nets] : named_operands)
                {
                    for (u32 idx = 0; idx < nets.size(); idx++)
                    {
                        const auto& net = nets.at(idx);
                        nets_to_indices[net][name].push_back(idx);
                    }
                }

                // TODO remove debug printing
                // for (const auto& [net, names] : nets_to_indices)
                // {
                //     std::cout << "Net: " << net->get_id() << " / " << net->get_name() << std::endl;
                //     for (const auto& [name, indices] : names)
                //     {
                //         std::cout << "\t" << name << std::endl;
                //         for (const auto& index : indices)
                //         {
                //             std::cout << "\t\t" << index << std::endl;
                //         }
                //     }
                // }

                std::set<Net*> visited;
                for (const auto& [name, nets] : named_operands)
                {
                    std::vector<hal::ModulePin*> operand_pins;
                    for (const auto& net : nets)
                    {
                        auto pin = mod->get_pin_by_net(net);
                        if (pin == nullptr)
                        {
                            log_error("module_identification", "module {} / {} does not have a pin connected to net {} / {}", mod->get_name(), mod->get_id(), net->get_name(), net->get_id());
                            continue;
                        }

                        // if the net/pin already belongs to the group we dont try to add it again
                        if (std::find(visited.begin(), visited.end(), net) != visited.end())
                        {
                            continue;
                        }
                        visited.insert(net);

                        // NOTE this is a way to build an ugly pin name containing all the pins that a net is connected to but for now i dont know of any better solution
                        std::vector<std::string> operand_pin_names;
                        for (const auto& [op_name, indices] : nets_to_indices.at(net))
                        {
                            operand_pin_names.push_back(op_name + "_" + utils::join(", ", indices));
                        }
                        std::string new_pin_name = utils::join(" | ", operand_pin_names);

                        mod->set_pin_name(pin, new_pin_name);
                        operand_pins.push_back(pin);
                    }

                    auto res = mod->create_pin_group(name, operand_pins, hal::PinDirection::input, hal::PinType::data, false, 0, true);
                    if (res.is_error())
                    {
                        log_error("module_identification", "could not create input pin group: {}", res.get_error().get());
                    }
                }

                // add outputs
                std::vector<hal::ModulePin*> output_pins;
                u32 counter = 0;
                for (const auto& out_net : selected_candidate.m_output_nets)
                {
                    auto pin = mod->get_pin_by_net(out_net);
                    if (pin == nullptr)
                    {
                        log_error("module_identification", "module {} / {} does not have a pin connected to net {} / {}", mod->get_name(), mod->get_id(), out_net->get_name(), out_net->get_id());
                        continue;
                    }

                    std::string pin_name = "OUT_" + std::to_string(counter++);
                    mod->set_pin_name(pin, pin_name);
                    output_pins.push_back(pin);
                }

                auto output_res = mod->create_pin_group("OUT", output_pins, hal::PinDirection::output, hal::PinType::data, false, 0, true);
                if (output_res.is_error())
                {
                    hal::log_error("module_identification", "could not create output pin group: {}", output_res.get_error().get());
                }

                // add controls
                counter = 0;
                std::vector<hal::ModulePin*> ctrl_pins;
                for (const auto& ctrl_net : selected_candidate.m_control_signals)
                {
                    auto pin = mod->get_pin_by_net(ctrl_net);
                    if (pin == nullptr)
                    {
                        log_error("module_identification", "module {} / {} does not have a pin connected to net {} / {}", mod->get_name(), mod->get_id(), ctrl_net->get_name(), ctrl_net->get_id());
                        continue;
                    }

                    std::string pin_name = "CTRL_" + std::to_string(counter++);
                    mod->set_pin_name(pin, pin_name);
                    ctrl_pins.push_back(pin);
                }
                if (!ctrl_pins.empty())
                {
                    auto ctrl_res = mod->create_pin_group("CTRL", ctrl_pins, hal::PinDirection::input, hal::PinType::control, false, 0, true);
                    if (ctrl_res.is_error())
                    {
                        log_info("module_identification", "could not create ctrl pin group: {}", ctrl_res.get_error().get());
                    }
                }

                // This creates a more human readable form of the word level operations
                std::string word_level_operation_hr_str = "";
                for (const auto& [cm, bf] : selected_candidate.m_word_level_operations)
                {
                    u32 ctrl_val = 0;
                    for (const auto& [net, val] : cm)
                    {
                        ctrl_val = (ctrl_val << 1) + ((val == BooleanFunction::ONE) ? 1 : 0);
                    }

                    const auto bf_hr_res = BooleanFunctionDecorator(bf).substitute_module_pins({mod});
                    if (bf_hr_res.is_error())
                    {
                        log_warning("module_identification", "{}", bf_hr_res.get_error().get());
                        continue;
                    }
                    const auto bf_hr = bf_hr_res.get().simplify_local();

                    word_level_operation_hr_str += std::to_string(ctrl_val) + ": " + bf_hr.to_string() + "\n";
                }

                mod->set_data("ModuleIdentification", "OPERATIONS_HR", "String", word_level_operation_hr_str);
            }

            return OK({});
        }

        std::string Result::get_timing_stats() const
        {
            return m_timing_stats_json;
        }

        hal::Result<Result> Result::merge(const Result& other, const std::vector<std::vector<Gate*>>& dana_cache) const
        {
            std::unordered_set<Gate*> base_gates;
            std::map<const std::set<Gate*>, std::vector<VerifiedCandidate>> base_candidate_to_verified_candidate;

            for (const auto& [bc, vc] : other.m_candidates)
            {
                const std::set<Gate*> bc_set = {bc.m_gates.begin(), bc.m_gates.end()};

                // check whether base candidate is already in map
                if (auto it = base_candidate_to_verified_candidate.find(bc_set); it != base_candidate_to_verified_candidate.end())
                {
                    it->second.push_back(vc);
                    continue;
                }

                // check whether parts of the base candidate are a nullptr, not in the netlist or already contained in other base candidates
                for (const auto& g : bc.m_gates)
                {
                    if (g == nullptr)
                    {
                        return ERR("failed to merge results: other result contains a base candidate with a nullptr gate");
                    }

                    if (!this->m_netlist->get_top_module()->contains_gate(g))
                    {
                        return ERR("failed to merge results: base candidate gate " + std::to_string((u64)(void**)g) + " is not (or no longer) part of the netlist!");
                    }

                    // TODO this should be the case, but it is not, since we have not yet resolved conflicts
                    // if (base_gates.find(g) != base_gates.end())
                    // {
                    //     return ERR("failed to merge results: base candidate gate " + g->get_name() + " with ID " + std::to_string(g->get_id()) + " is part of multiple different base candidates!");
                    // }

                    base_gates.insert(g);
                }

                // add verfied candidate
                base_candidate_to_verified_candidate[bc_set].push_back(vc);
            }

            for (const auto& [bc, vc] : m_candidates)
            {
                const std::set<Gate*> bc_set = {bc.m_gates.begin(), bc.m_gates.end()};

                // check whether base candidate is already in map
                if (auto it = base_candidate_to_verified_candidate.find(bc_set); it != base_candidate_to_verified_candidate.end())
                {
                    it->second.push_back(vc);
                    continue;
                }

                // check whether parts of the base candidate are a nullptr, not in the netlist or already contained in other base candidates
                for (const auto& g : bc.m_gates)
                {
                    if (g == nullptr)
                    {
                        return ERR("failed to merge results: result contains a base candidate with a nullptr gate");
                    }

                    if (!this->m_netlist->get_top_module()->contains_gate(g))
                    {
                        return ERR("failed to merge results: base candidate gate " + std::to_string((u64)(void**)g) + " is not part of the netlist!");
                    }

                    // TODO this should be the case, but it is not, since we have not yet resolved conflicts
                    // if (base_gates.find(g) != base_gates.end())
                    // {
                    //     return ERR("failed to merge results: base candidate gate " + g->get_name() + " with ID " + std::to_string(g->get_id()) + " is part of multiple different base candidates!");
                    // }

                    base_gates.insert(g);
                }

                // add verfied candidate
                base_candidate_to_verified_candidate[bc_set].push_back(vc);
            }

            std::vector<std::pair<BaseCandidate, VerifiedCandidate>> result_candidates;
            for (auto& [bc_set, vc] : base_candidate_to_verified_candidate)
            {
                result_candidates.push_back(std::make_pair(BaseCandidate({bc_set.begin(), bc_set.end()}), post_processing(vc, this->m_netlist, dana_cache)));
            }

            return OK(Result{this->m_netlist, result_candidates});
        }

        namespace
        {
            // TODO this is duplicated in the post processing. find a common place for this
            u64 calculate_ignored_input_signals(const VerifiedCandidate& vc)
            {
                // first collect all operand nets
                std::set<Net*> covered_nets;
                for (const auto& op : vc.m_operands)
                {
                    for (const auto& net : op)
                    {
                        if (net->is_gnd_net() || net->is_vcc_net())
                        {
                            continue;
                        }
                        covered_nets.insert(net);
                    }
                }
                covered_nets.insert(vc.m_control_signals.begin(), vc.m_control_signals.end());

                // second collect all nets that are not part of the operands
                u64 ignored_inputs = 0;
                for (const auto& input_net : vc.m_total_input_nets)
                {
                    if (covered_nets.find(input_net) == covered_nets.end())
                    {
                        ignored_inputs++;
                    }
                }

                return ignored_inputs;
            }

            // calculates the maximum amount of outputs not covered by the candidate output
            u64 calculate_ignored_output_signals(const VerifiedCandidate& vc)
            {
                const auto all_outputs     = vc.m_total_output_nets.size();
                const auto c_outputs       = vc.m_output_nets.size();
                const auto outputs_ignored = (c_outputs > all_outputs) ? 0 : all_outputs - c_outputs;

                return outputs_ignored;
            }

            u64 calculate_ctrl_score(const VerifiedCandidate& vc)
            {
                return vc.m_control_signals.size();
            }

            u64 compute_total_io_score(const VerifiedCandidate& vc)
            {
                std::set<Net*> total_data_io;
                for (const auto& nets : vc.m_operands)
                {
                    for (const auto& n : nets)
                    {
                        if (n->is_gnd_net() || n->is_vcc_net())
                        {
                            continue;
                        }
                        total_data_io.insert(n);
                    }
                }
                for (const auto& n : vc.m_output_nets)
                {
                    total_data_io.insert(n);
                }

                return (u64)total_data_io.size();
            }

            u64 compute_is_verified(const VerifiedCandidate& vc)
            {
                return (vc.is_verified() ? 1 : 0);
            }

            bool compare_candidates(const VerifiedCandidate& vc1, const VerifiedCandidate& vc2)
            {
                const std::vector<std::pair<bool, std::function<u64(const VerifiedCandidate&)>>> metrics = {
                    {false, compute_is_verified}, {false, compute_total_io_score}, {true, calculate_ignored_input_signals}, {true, calculate_ctrl_score}, {true, calculate_ignored_output_signals}};

                for (const auto& [higher_is_better, metric_func] : metrics)
                {
                    const auto score_1 = metric_func(vc1);
                    const auto score_2 = metric_func(vc2);

                    if (score_1 < score_2)
                    {
                        return higher_is_better;
                    }
                    else if (score_1 > score_2)
                    {
                        return !higher_is_better;
                    }
                }

                return true;
            }
        }    // namespace

        std::vector<std::vector<std::set<Gate*>>> Result::assign_base_candidates_to_iterations(const std::vector<Result>& iteration_results, const bool create_block_lists)
        {
            // collect all base candidates
            std::set<std::set<Gate*>> all_base_candidates;
            for (const auto& res : iteration_results)
            {
                for (const auto& [bc, _] : res.m_candidates)
                {
                    all_base_candidates.insert({bc.m_gates.begin(), bc.m_gates.end()});
                }
            }

            // for each base candidate collect the verified candidates in the iteration results
            std::map<const std::set<Gate*>, std::vector<std::pair<u32, VerifiedCandidate>>> base_candidate_to_verified_candidates;

            for (u32 iteration_idx = 0; iteration_idx < iteration_results.size(); iteration_idx++)
            {
                const auto& res = iteration_results.at(iteration_idx);

                for (const auto& bc : all_base_candidates)
                {
                    const auto it =
                        std::find_if(res.m_candidates.begin(), res.m_candidates.end(), [&bc](const auto& p) { return std::set<Gate*>{p.first.m_gates.begin(), p.first.m_gates.end()} == bc; });

                    if (it == res.m_candidates.end())
                    {
                        base_candidate_to_verified_candidates[bc].push_back({iteration_idx, VerifiedCandidate{}});
                    }
                    else
                    {
                        base_candidate_to_verified_candidates[bc].push_back({iteration_idx, it->second});
                    }
                }
            }

            std::vector<std::vector<std::set<Gate*>>> iteration_assignments{iteration_results.size()};

            for (const auto& [bc, candidates] : base_candidate_to_verified_candidates)
            {
                // select the best result (iteration) for each base candidate
                auto candidates_sorted = candidates;
                std::sort(candidates_sorted.begin(), candidates_sorted.end(), [](const auto& p1, const auto& p2) { return compare_candidates(p1.second, p2.second); });
                const auto best_iteration = candidates_sorted.front().first;

                // TODO remove debug printing
                const std::string base_name = candidates_sorted.front().second.m_base_gates.empty() ? "EMPTY" : candidates_sorted.front().second.m_base_gates.front()->get_name();
                std::cout << "Found the following candidates [" << base_name << "]: " << std::endl;
                for (const auto& [it_idx, c] : candidates)
                {
                    const auto c_type = c.m_types.empty() ? CandidateType::none : *(c.m_types.begin());
                    std::cout << it_idx << " - " << c.is_verified() << " " << enum_to_string(c_type) << " [" << compute_total_io_score(c) << " " << calculate_ignored_input_signals(c) << " "
                              << calculate_ctrl_score(c) << " " << calculate_ignored_output_signals(c) << "]" << std::endl;
                }
                std::cout << "Chose iteration " << best_iteration << " as best iteration." << std::endl;

                // create allow/block list for each base candidate containing the base candidates for which it produces the best results
                for (u32 iteration_idx = 0; iteration_idx < iteration_results.size(); iteration_idx++)
                {
                    bool assign_to_iteration = create_block_lists ? (iteration_idx != best_iteration) : (iteration_idx == best_iteration);
                    if (assign_to_iteration)
                    {
                        iteration_assignments.at(iteration_idx).push_back(bc);
                    }
                }
            }

            // TODO remove debug printing
            for (u32 idx = 0; idx < iteration_assignments.size(); idx++)
            {
                std::cout << "Iteration Assignment " << idx << ": " << std::endl;
                for (const auto& gate_vec : iteration_assignments.at(idx))
                {
                    std::cout << "\t" << (*gate_vec.begin())->get_id() << " - " << (*gate_vec.begin())->get_name() << std::endl;
                }
            }

            return iteration_assignments;
        }

    }    // namespace module_identification
}    // namespace hal