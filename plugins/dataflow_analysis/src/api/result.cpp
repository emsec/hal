#include "dataflow_analysis/api/result.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"

#include <fstream>

namespace hal
{
    namespace dataflow
    {
        namespace
        {
            std::unordered_set<u32> nets_to_id_set(const std::unordered_set<Net*>& nets)
            {
                std::unordered_set<u32> ids;
                for (const auto* net : nets)
                {
                    ids.insert(net->get_id());
                }
                return ids;
            }
        }    // namespace

        dataflow::Result::Result(Netlist* nl, const Grouping& grouping)
        {
            m_netlist = nl;

            const auto& na = grouping.netlist_abstr;
            for (const auto* gate : na.target_gates)
            {
                auto gate_id = gate->get_id();

                if (const auto it = na.gate_to_successors.find(gate_id); it != na.gate_to_successors.end())
                {
                    for (auto suc_gate_id : std::get<1>(*it))
                    {
                        this->m_gate_successors[gate].insert(nl->get_gate_by_id(suc_gate_id));
                    }
                }

                if (const auto it = na.gate_to_predecessors.find(gate_id); it != na.gate_to_predecessors.end())
                {
                    for (auto pred_gate_id : std::get<1>(*it))
                    {
                        this->m_gate_predecessors[gate].insert(nl->get_gate_by_id(pred_gate_id));
                    }
                }

                if (const auto it = na.gate_to_control_signals.find(gate_id); it != na.gate_to_control_signals.end())
                {
                    for (const auto& [type, signals] : std::get<1>(*it))
                    {
                        for (auto signal_net_id : signals)
                        {
                            this->m_gate_signals[gate][type].insert(nl->get_net_by_id(signal_net_id));
                        }
                    }
                }
            }

            for (const auto& [group_id, gate_ids] : grouping.gates_of_group)
            {
                std::unordered_set<Gate*> gates;
                for (const auto gate_id : gate_ids)
                {
                    auto* gate = m_netlist->get_gate_by_id(gate_id);
                    gates.insert(gate);
                    this->m_parent_group_of_gate[gate] = group_id;
                }
                m_gates_of_group[group_id] = gates;

                for (const auto& [type, signals] : grouping.get_control_signals_of_group(group_id))
                {
                    for (auto signal_net_id : signals)
                    {
                        this->m_group_signals[group_id][type].insert(m_netlist->get_net_by_id(signal_net_id));
                    }
                }

                if (auto suc_ids = grouping.get_successor_groups_of_group(group_id); !suc_ids.empty())
                {
                    this->m_group_successors[group_id] = suc_ids;
                }

                if (auto pred_ids = grouping.get_predecessor_groups_of_group(group_id); !pred_ids.empty())
                {
                    this->m_group_predecessors[group_id] = pred_ids;
                }

                this->m_last_id = std::max(this->m_last_id, group_id);
            }
        }

        Netlist* dataflow::Result::get_netlist() const
        {
            return this->m_netlist;
        }

        const std::unordered_map<u32, std::unordered_set<Gate*>>& dataflow::Result::get_groups() const
        {
            return this->m_gates_of_group;
        }

        std::vector<Gate*> dataflow::Result::get_gates() const
        {
            std::vector<Gate*> gates;

            for (auto& group_gates : m_gates_of_group)
            {
                std::transform(std::get<1>(group_gates).begin(), std::get<1>(group_gates).end(), std::back_inserter(gates), [](auto* gate) { return gate; });
            }

            return gates;
        }

        hal::Result<std::unordered_set<Gate*>> dataflow::Result::get_gates_of_group(const u32 group_id) const
        {
            if (const auto it = this->m_gates_of_group.find(group_id); it != this->m_gates_of_group.end())
            {
                return OK(it->second);
            }
            else
            {
                return ERR("invalid group ID.");
            }
        }

        hal::Result<u32> dataflow::Result::get_group_id_of_gate(const Gate* gate) const
        {
            if (!gate)
            {
                return ERR("gate is a nullptr.");
            }

            if (const auto it = this->m_parent_group_of_gate.find(gate); it != this->m_parent_group_of_gate.end())
            {
                return OK(it->second);
            }
            else
            {
                return ERR("gate is not part of a group.");
            }
        }

        hal::Result<std::unordered_set<Net*>> dataflow::Result::get_group_control_nets(const u32 group_id, const PinType type) const
        {
            if (this->m_gates_of_group.find(group_id) == this->m_gates_of_group.end())
            {
                return ERR("invalid group ID.");
            }

            if (const auto group_it = this->m_group_signals.find(group_id); group_it != this->m_group_signals.end())
            {
                const auto& type_map = std::get<1>(*group_it);
                if (const auto type_it = type_map.find(type); type_it != type_map.end())
                {
                    return OK(type_it->second);
                }
            }

            return OK({});
        }

        hal::Result<std::unordered_set<Net*>> dataflow::Result::get_gate_control_nets(const Gate* gate, const PinType type) const
        {
            if (this->m_parent_group_of_gate.find(gate) == this->m_parent_group_of_gate.end())
            {
                return ERR("gate is not part of a group.");
            }

            if (const auto gate_it = this->m_gate_signals.find(gate); gate_it != this->m_gate_signals.end())
            {
                const auto& type_map = std::get<1>(*gate_it);
                if (const auto type_it = type_map.find(type); type_it != type_map.end())
                {
                    return OK(type_it->second);
                }
            }

            return OK({});
        }

        hal::Result<std::unordered_set<u32>> dataflow::Result::get_group_successors(const u32 group_id) const
        {
            if (this->m_gates_of_group.find(group_id) == this->m_gates_of_group.end())
            {
                return ERR("invalid group ID.");
            }

            if (const auto it = this->m_group_successors.find(group_id); it != this->m_group_successors.end())
            {
                return OK(it->second);
            }

            return OK({});
        }

        hal::Result<std::unordered_set<Gate*>> dataflow::Result::get_gate_successors(const Gate* gate) const
        {
            if (this->m_parent_group_of_gate.find(gate) == this->m_parent_group_of_gate.end())
            {
                return ERR("gate is not part of a group.");
            }

            if (const auto it = this->m_gate_successors.find(gate); it != this->m_gate_successors.end())
            {
                return OK(it->second);
            }

            return OK({});
        }

        hal::Result<std::unordered_set<u32>> dataflow::Result::get_group_predecessors(const u32 group_id) const
        {
            if (this->m_gates_of_group.find(group_id) == this->m_gates_of_group.end())
            {
                return ERR("invalid group ID.");
            }

            if (const auto it = this->m_group_predecessors.find(group_id); it != this->m_group_predecessors.end())
            {
                return OK(it->second);
            }

            return OK({});
        }

        hal::Result<std::unordered_set<Gate*>> dataflow::Result::get_gate_predecessors(const Gate* gate) const
        {
            if (this->m_parent_group_of_gate.find(gate) == this->m_parent_group_of_gate.end())
            {
                return ERR("gate is not part of a group.");
            }

            if (const auto it = this->m_gate_predecessors.find(gate); it != this->m_gate_predecessors.end())
            {
                return OK(it->second);
            }

            return OK({});
        }

        hal::Result<std::monostate> dataflow::Result::write_dot(const std::filesystem::path& out_path, const std::unordered_set<u32>& group_ids) const
        {
            auto write_path = out_path;

            if (write_path.empty())
            {
                return ERR("output path is empty.");
            }

            if (std::filesystem::is_directory(write_path))
            {
                write_path /= "graph.dot";
            }

            if (write_path.extension() != "dot")
            {
                log_info("dataflow", "replacing invalid file extension '{}' with '.dot' ...", write_path.extension().string());
                write_path.replace_extension("dot");
            }

            log_info("dataflow", "writing dataflow graph to '{}' ...", write_path.string());

            std::ofstream ofs(write_path, std::ofstream::out);
            if (ofs)
            {
                ofs << "digraph {\n\tnode [shape=box fillcolor=white style=filled];\n";

                // print node
                for (const auto& [group_id, gates] : this->get_groups())
                {
                    if (!group_ids.empty() && group_ids.find(group_id) == group_ids.end())
                    {
                        continue;
                    }

                    auto size = gates.size() * 0.1;
                    if (size < 1500)
                    {
                        size = size * 0.02;
                    }

                    ofs << group_id << " [width=" << 0.05 * gates.size() << " label=\"" << gates.size() << " bit (id " << group_id << ")\"];\n";
                }

                // print edges
                for (const auto& [group_id, gates] : this->get_groups())
                {
                    if (!group_ids.empty() && group_ids.find(group_id) == group_ids.end())
                    {
                        continue;
                    }

                    auto sucs = this->get_group_successors(group_id).get();
                    for (auto suc_id : sucs)
                    {
                        if (!group_ids.empty() && group_ids.find(suc_id) == group_ids.end())
                        {
                            continue;
                        }
                        ofs << group_id << " -> " << suc_id << ";\n";
                    }
                }

                ofs << "}";
                ofs.close();

                log_info("dataflow", "successfully written dataflow graph to '{}'.", write_path.string());
                return OK({});
            }

            return ERR("failed to open file at '" + write_path.string() + "' for writing dataflow graph.");
        }

        hal::Result<std::monostate> dataflow::Result::write_txt(const std::filesystem::path& out_path, const std::unordered_set<u32>& group_ids) const
        {
            auto write_path = out_path;

            if (write_path.empty())
            {
                return ERR("output path is empty.");
            }

            if (std::filesystem::is_directory(write_path))
            {
                write_path /= "groups.txt";
            }

            if (write_path.extension() != "txt")
            {
                log_info("dataflow", "replacing invalid file extension '{}' with '.txt' ...", write_path.extension().string());
                write_path.replace_extension("txt");
            }

            log_info("dataflow", "writing dataflow gate groups to '{}' ...", write_path.string());

            std::ofstream ofs(write_path, std::ofstream::out);
            if (ofs)
            {
                ofs << "State:";
                ofs << "\n\n";

                for (const auto& [group_id, gates] : this->get_groups())
                {
                    if (!group_ids.empty() && group_ids.find(group_id) == group_ids.end())
                    {
                        continue;
                    }

                    ofs << "ID:" << group_id << ", ";
                    ofs << "Size:" << gates.size() << ", ";
                    ofs << "CLK: {" << utils::join(", ", nets_to_id_set(this->get_group_control_nets(group_id, PinType::clock).get())) << "}, ";
                    ofs << "EN: {" << utils::join(", ", nets_to_id_set(this->get_group_control_nets(group_id, PinType::enable).get())) << "}, ";
                    ofs << "R: {" << utils::join(", ", nets_to_id_set(this->get_group_control_nets(group_id, PinType::reset).get())) << "}, ";
                    ofs << "S: {" << utils::join(", ", nets_to_id_set(this->get_group_control_nets(group_id, PinType::set).get())) << "}" << std::endl;

                    if (auto res = this->get_group_successors(group_id); res.is_ok())
                    {
                        auto unsorted_successors = res.get();
                        ofs << "  Successors:   {" + utils::join(", ", std::set<u32>(unsorted_successors.begin(), unsorted_successors.end())) << "}" << std::endl;
                    }

                    if (auto res = this->get_group_predecessors(group_id); res.is_ok())
                    {
                        auto unsorted_predecessors = res.get();
                        ofs << "  Predecessors: {" + utils::join(", ", std::set<u32>(unsorted_predecessors.begin(), unsorted_predecessors.end())) << "}" << std::endl;
                    }

                    std::unordered_map<u32, std::vector<std::string>> texts;
                    std::unordered_map<u32, u32> text_max_lengths;

                    std::set<u32> gate_ids;
                    for (const auto& gate : gates)
                    {
                        auto name          = gate->get_name() + ", ";
                        auto type          = "type: " + gate->get_type()->get_name() + ", ";
                        auto id            = "id: " + std::to_string(gate->get_id()) + ", ";
                        std::string stages = "RS: ";

                        std::vector<std::string> data = {name, type, id, stages};
                        for (u32 i = 0; i < data.size(); ++i)
                        {
                            text_max_lengths[i] = std::max(text_max_lengths[i], (u32)data[i].size());
                        }

                        u32 gate_id = gate->get_id();
                        texts.emplace(gate_id, data);
                        gate_ids.insert(gate_id);
                    }

                    for (const auto& gate_id : gate_ids)
                    {
                        auto& data = texts[gate_id];
                        for (u32 i = 0; i < data.size(); ++i)
                        {
                            ofs << std::setw(text_max_lengths[i]) << std::left << data[i];
                        }
                        ofs << std::endl;
                    }
                    ofs << "\n";
                }
                ofs.close();

                log_info("dataflow", "successfully written dataflow gate groups to '{}'.", write_path.string());
                return OK({});
            }

            return ERR("failed to open file at '" + write_path.string() + "' for writing dataflow gate groups.");
        }

        hal::Result<std::unordered_map<u32, Module*>> dataflow::Result::create_modules(const std::unordered_set<u32>& group_ids) const
        {
            return create_modules(std::map<const GateType*, std::string>(), std::map<std::pair<PinDirection, std::string>, std::string>(), group_ids);
        }

        hal::Result<std::unordered_map<u32, Module*>> dataflow::Result::create_modules(const std::map<const GateType*, std::string>& module_suffixes,
                                                                                       const std::map<std::pair<PinDirection, std::string>, std::string>& pin_prefixes,
                                                                                       const std::unordered_set<u32>& group_ids) const
        {
            auto* nl = this->get_netlist();

            // delete all modules that start with DANA
            std::vector<Module*> modules_to_delete;
            for (const auto mod : nl->get_modules())
            {
                if (utils::starts_with(mod->get_name(), std::string("DANA_")))
                {
                    modules_to_delete.push_back(mod);
                }
            }

            for (auto* mod : modules_to_delete)
            {
                nl->delete_module(mod);
            }
            log_info("dataflow", "successfully deleted old DANA modules");

            // create new modules and try to keep hierarchy if possible
            std::unordered_map<u32, Module*> group_to_module;
            for (const auto& [group_id, group] : this->get_groups())
            {
                if (!group_ids.empty() && group_ids.find(group_id) == group_ids.end())
                {
                    continue;
                }

                bool gate_hierachy_matches_for_all = true;
                bool gate_type_matches_for_all     = true;
                bool first_run                     = true;
                const GateType* gate_type;
                auto* reference_module = nl->get_top_module();

                std::vector<Gate*> gates;
                for (const auto gate : group)
                {
                    gates.push_back(gate);

                    if (first_run)
                    {
                        reference_module = gate->get_module();
                        gate_type        = gate->get_type();
                        first_run        = false;
                    }
                    else if (gate->get_module() != reference_module)
                    {
                        gate_hierachy_matches_for_all = false;
                    }

                    if (gate_type != gate->get_type())
                    {
                        gate_hierachy_matches_for_all = false;
                    }
                }

                if (!gate_hierachy_matches_for_all)
                {
                    reference_module = nl->get_top_module();
                }

                std::string suffix;
                if (const auto it = module_suffixes.find(gate_type); gate_type_matches_for_all && it != module_suffixes.end())
                {
                    suffix = it->second;
                }
                else
                {
                    suffix = "module";
                }

                auto* new_mod             = nl->create_module("DANA_" + suffix + "_" + std::to_string(group_id), reference_module, gates);
                group_to_module[group_id] = new_mod;

                std::map<const std::pair<PinDirection, std::string>, PinGroup<ModulePin>*> pin_groups;
                for (auto* pin : new_mod->get_pins())
                {
                    if (pin->get_direction() == PinDirection::input)
                    {
                        const auto destinations = pin->get_net()->get_destinations([new_mod](const Endpoint* ep) { return ep->get_gate()->get_module() == new_mod; });

                        const auto* first_pin = destinations.front()->get_pin();
                        auto pin_type         = first_pin->get_type();
                        auto pin_name         = first_pin->get_name();
                        if (std::all_of(destinations.begin(), destinations.end(), [pin_name](const Endpoint* ep) { return ep->get_pin()->get_name() == pin_name; }))
                        {
                            const auto pg_key = std::make_pair(PinDirection::input, pin_name);

                            std::string prefix;
                            if (const auto prefix_it = pin_prefixes.find(pg_key); prefix_it != pin_prefixes.end())
                            {
                                prefix = prefix_it->second;
                            }
                            else
                            {
                                prefix = "i_" + pin_name;
                            }

                            if (const auto pg_it = pin_groups.find(pg_key); pg_it == pin_groups.end())
                            {
                                auto* pin_group    = pin->get_group().first;
                                pin_groups[pg_key] = pin_group;
                                new_mod->set_pin_group_name(pin_group, prefix);
                                new_mod->set_pin_group_type(pin_group, pin_type);
                            }
                            else
                            {
                                if (!new_mod->assign_pin_to_group(pg_it->second, pin))
                                {
                                    log_warning("dataflow", "Assign pin to group failed.");
                                }
                            }

                            new_mod->set_pin_name(pin, prefix + "(" + std::to_string(pin->get_group().second) + ")");
                            new_mod->set_pin_type(pin, pin_type);
                        }
                    }
                    else if (pin->get_direction() == PinDirection::output)
                    {
                        const auto sources = pin->get_net()->get_sources([new_mod](const Endpoint* ep) { return ep->get_gate()->get_module() == new_mod; });

                        const auto* first_pin = sources.front()->get_pin();
                        auto pin_type         = first_pin->get_type();
                        auto pin_name         = first_pin->get_name();
                        if (sources.size() == 1)
                        {
                            const auto pg_key = std::make_pair(PinDirection::output, pin_name);

                            std::string prefix;
                            if (const auto prefix_it = pin_prefixes.find(pg_key); prefix_it != pin_prefixes.end())
                            {
                                prefix = prefix_it->second;
                            }
                            else
                            {
                                prefix = "o_" + pin_name;
                            }

                            if (const auto pg_it = pin_groups.find(pg_key); pg_it == pin_groups.end())
                            {
                                auto* pin_group    = pin->get_group().first;
                                pin_groups[pg_key] = pin_group;
                                new_mod->set_pin_group_name(pin_group, prefix);
                                new_mod->set_pin_group_type(pin_group, pin_type);
                            }
                            else
                            {
                                if (!new_mod->assign_pin_to_group(pg_it->second, pin))
                                {
                                    log_warning("dataflow", "Assign pin to group failed.");
                                }
                            }

                            new_mod->set_pin_name(pin, prefix + "(" + std::to_string(pin->get_group().second) + ")");
                            new_mod->set_pin_type(pin, pin_type);
                        }
                    }
                }

                // rename pins if only single pin in pin group (remove "(" and ")")
                for (const auto* pin_group : new_mod->get_pin_groups())
                {
                    if (pin_group->size() == 1)
                    {
                        auto* pin = pin_group->get_pins().front();
                        new_mod->set_pin_name(pin, pin_group->get_name());
                    }
                }
            }
            return OK(group_to_module);
        }

        hal::Result<std::unordered_map<u32, Module*>> dataflow::Result::create_modules(const std::map<GateTypeProperty, std::string>& module_suffixes,
                                                                                       const std::map<std::pair<PinDirection, std::string>, std::string>& pin_prefixes,
                                                                                       const std::unordered_set<u32>& group_ids) const
        {
            const auto* gl = this->m_netlist->get_gate_library();
            std::map<const GateType*, std::string> gate_type_suffixes;
            for (const auto& suffix : module_suffixes)
            {
                for (const auto& [_, type] : gl->get_gate_types([suffix](const GateType* gt) { return gt->has_property(suffix.first); }))
                {
                    gate_type_suffixes[type] = suffix.second;
                }
            }

            return this->create_modules(gate_type_suffixes, pin_prefixes, group_ids);
        }

        std::vector<std::vector<Gate*>> dataflow::Result::get_groups_as_list(const std::unordered_set<u32>& group_ids) const
        {
            std::vector<std::vector<Gate*>> groups;
            for (const auto& [group_id, group] : this->get_groups())
            {
                if (!group_ids.empty() && group_ids.find(group_id) == group_ids.end())
                {
                    continue;
                }

                std::vector<Gate*> group_vector;
                for (const auto& gate : group)
                {
                    group_vector.push_back(gate);
                }
                groups.push_back(group_vector);
            }
            return groups;
        }

        hal::Result<u32> dataflow::Result::merge_groups(const std::vector<u32>& group_ids)
        {
            if (group_ids.empty())
            {
                return ERR("no group IDs provided.");
            }

            if (group_ids.size() < 2)
            {
                return ERR("at least two groups are required for merging.");
            }

            if (const auto it = std::find_if(group_ids.begin(), group_ids.end(), [this](u32 id) { return this->m_gates_of_group.find(id) == this->m_gates_of_group.end(); }); it != group_ids.end())
            {
                return ERR("a group with ID " + std::to_string(*it) + " does not exist.");
            }

            u32 target_group_id = ++(this->m_last_id);

            // iterate set to make sure that every ID is contained only once
            std::unordered_set<u32> group_ids_set(group_ids.begin(), group_ids.end());
            for (auto group_id : group_ids_set)
            {
                // new group
                auto& gates_at_i = this->m_gates_of_group.at(group_id);
                this->m_gates_of_group[target_group_id].insert(gates_at_i.begin(), gates_at_i.end());
                this->m_gates_of_group.erase(group_id);

                std::for_each(gates_at_i.begin(), gates_at_i.end(), [this, target_group_id](const Gate* g) { this->m_parent_group_of_gate.at(g) = target_group_id; });

                // signals
                if (const auto signals_it = this->m_group_signals.find(group_id); signals_it != this->m_group_signals.end())
                {
                    for (auto& signals_of_type : std::get<1>(*signals_it))
                    {
                        auto& signals = std::get<1>(signals_of_type);
                        this->m_group_signals[target_group_id][signals_of_type.first].insert(signals.begin(), signals.end());
                    }
                    this->m_group_signals.erase(signals_it);
                }

                // successors / predecessors
                if (const auto suc_it = this->m_group_successors.find(group_id); suc_it != this->m_group_successors.end())
                {
                    auto& target_suc_ids = this->m_group_successors[target_group_id];
                    for (auto suc_id : std::get<1>(*suc_it))
                    {
                        auto& pred_ids = this->m_group_predecessors.at(suc_id);
                        pred_ids.insert(target_group_id);
                        target_suc_ids.insert(suc_id);
                        pred_ids.erase(group_id);
                    }
                }

                if (const auto pred_it = this->m_group_predecessors.find(group_id); pred_it != this->m_group_predecessors.end())
                {
                    auto& target_pred_ids = this->m_group_predecessors[target_group_id];
                    for (auto pred_id : std::get<1>(*pred_it))
                    {
                        auto& suc_ids = this->m_group_successors.at(pred_id);
                        suc_ids.insert(target_group_id);
                        target_pred_ids.insert(pred_id);
                        suc_ids.erase(group_id);
                    }
                }

                this->m_group_successors.erase(group_id);
                this->m_group_predecessors.erase(group_id);
            }

            return OK(target_group_id);
        }

        hal::Result<std::vector<u32>> dataflow::Result::split_group(u32 group_id, const std::vector<std::unordered_set<Gate*>>& new_groups)
        {
            if (new_groups.empty())
            {
                return ERR("no gates provided to define splits.");
            }

            const auto group_it = this->m_gates_of_group.find(group_id);
            if (group_it == this->m_gates_of_group.end())
            {
                return ERR("a group with ID " + std::to_string(group_id) + " does not exist.");
            }

            const auto& group_gates = std::get<1>(*group_it);

            std::unordered_set<const Gate*> seen;
            for (const auto& gates : new_groups)
            {
                for (auto* g : gates)
                {
                    if (!g)
                    {
                        return ERR("gate is a nullptr.");
                    }

                    if (group_gates.find(g) == group_gates.end())
                    {
                        return ERR("gate '" + g->get_name() + "' with ID " + std::to_string(g->get_id()) + " does not belong to group with ID " + std::to_string(group_id) + ".");
                    }

                    if (seen.find(g) != seen.end())
                    {
                        return ERR("gate '" + g->get_name() + "' with ID " + std::to_string(g->get_id()) + " cannot be assigned to two groups after splitting.");
                    }
                    seen.insert(g);
                }
            }

            if (seen.size() != group_gates.size())
            {
                return ERR("size of the target group does not match combined size of the split groups.");
            }

            this->m_gates_of_group.erase(group_id);
            this->m_group_successors.erase(group_id);
            this->m_group_predecessors.erase(group_id);
            this->m_group_signals.erase(group_id);

            std::vector<u32> new_group_ids;
            for (const auto& gates : new_groups)
            {
                auto current_id = ++(this->m_last_id);

                // new group
                this->m_gates_of_group[current_id] = gates;

                for (const auto* g : gates)
                {
                    this->m_parent_group_of_gate.at(g) = current_id;

                    // signals
                    if (const auto signals_it = this->m_gate_signals.find(g); signals_it != this->m_gate_signals.end())
                    {
                        const auto& signals = std::get<1>(*signals_it);

                        for (const auto type : std::vector<PinType>({PinType::clock, PinType::enable, PinType::reset, PinType::set}))
                        {
                            if (const auto nets_it = signals.find(type); nets_it != signals.end())
                            {
                                const auto& nets = std::get<1>(*nets_it);
                                this->m_group_signals[current_id][type].insert(nets.begin(), nets.end());
                            }
                        }
                    }
                }

                new_group_ids.push_back(current_id);
            }

            // successors / predecessors (can only be inferred once all new groups have been created)
            for (const auto current_id : new_group_ids)
            {
                for (const auto* g : this->m_gates_of_group.at(current_id))
                {
                    if (const auto suc_it = this->m_gate_successors.find(g); suc_it != this->m_gate_successors.end())
                    {
                        for (const auto* suc_g : std::get<1>(*suc_it))
                        {
                            this->m_group_successors[current_id].insert(this->m_parent_group_of_gate.at(suc_g));
                        }
                    }

                    if (const auto pred_it = this->m_gate_predecessors.find(g); pred_it != this->m_gate_predecessors.end())
                    {
                        for (const auto* pred_g : std::get<1>(*pred_it))
                        {
                            this->m_group_predecessors[current_id].insert(this->m_parent_group_of_gate.at(pred_g));
                        }
                    }
                }
            }

            return OK(new_group_ids);
        }
    }    // namespace dataflow
}    // namespace hal
