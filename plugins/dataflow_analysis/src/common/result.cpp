#include "dataflow_analysis/common/result.h"

#include "hal_core/netlist/gate.h"
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
            std::set<u32> nets_to_id_set(const std::unordered_set<Net*>& nets)
            {
                std::set<u32> ids;
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

            for (const auto& [group_id, gate_ids] : grouping.gates_of_group)
            {
                std::unordered_set<Gate*> gates;
                for (const auto gate_id : gate_ids)
                {
                    auto* gate = m_netlist->get_gate_by_id(gate_id);
                    gates.insert(gate);
                    m_parent_group_of_gate[gate] = group_id;
                }
                m_gates_of_group[group_id] = gates;

                for (const auto net_id : grouping.get_clock_signals_of_group(group_id))
                {
                    m_group_signals[group_id][PinType::clock].insert(m_netlist->get_net_by_id(net_id));
                }

                for (const auto net_id : grouping.get_control_signals_of_group(group_id))
                {
                    m_group_signals[group_id][PinType::enable].insert(m_netlist->get_net_by_id(net_id));
                }

                for (const auto net_id : grouping.get_reset_signals_of_group(group_id))
                {
                    m_group_signals[group_id][PinType::reset].insert(m_netlist->get_net_by_id(net_id));
                }

                for (const auto net_id : grouping.get_set_signals_of_group(group_id))
                {
                    m_group_signals[group_id][PinType::set].insert(m_netlist->get_net_by_id(net_id));
                }

                m_group_successors[group_id]   = grouping.get_successor_groups_of_group(group_id);
                m_group_predecessors[group_id] = grouping.get_predecessor_groups_of_group(group_id);
            }
        }

        Netlist* dataflow::Result::get_netlist() const
        {
            return m_netlist;
        }

        const std::unordered_map<u32, std::unordered_set<Gate*>>& dataflow::Result::get_groups() const
        {
            return m_gates_of_group;
        }

        hal::Result<std::unordered_set<Gate*>> dataflow::Result::get_gates_of_group(const u32 group_id) const
        {
            if (const auto it = m_gates_of_group.find(group_id); it != m_gates_of_group.end())
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

            if (const auto it = m_parent_group_of_gate.find(gate); it != m_parent_group_of_gate.end())
            {
                return OK(it->second);
            }
            else
            {
                return ERR("gate is not part of a group.");
            }
        }

        hal::Result<std::unordered_set<Net*>> dataflow::Result::get_control_nets_of_group(const u32 group_id, const PinType type) const
        {
            if (const auto group_it = m_group_signals.find(group_id); group_it != m_group_signals.end())
            {
                if (const auto type_it = group_it->second.find(type); type_it != group_it->second.end())
                {
                    return OK(type_it->second);
                }
                else
                {
                    return OK({});
                }
            }
            else
            {
                return ERR("invalid group ID.");
            }
        }

        hal::Result<std::unordered_set<u32>> dataflow::Result::get_group_successors(const u32 group_id) const
        {
            if (const auto it = m_group_successors.find(group_id); it != m_group_successors.end())
            {
                return OK(it->second);
            }
            else
            {
                return ERR("invalid group ID.");
            }
        }

        hal::Result<std::unordered_set<u32>> dataflow::Result::get_group_predecessors(const u32 group_id) const
        {
            if (const auto it = m_group_predecessors.find(group_id); it != m_group_predecessors.end())
            {
                return OK(it->second);
            }
            else
            {
                return ERR("invalid group ID.");
            }
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

                    for (auto suc_id : this->get_group_successors(group_id).get())
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
                    ofs << "CLK: {" << utils::join(", ", nets_to_id_set(this->get_control_nets_of_group(group_id, PinType::clock).get())) << "}, ";
                    ofs << "EN: {" << utils::join(", ", nets_to_id_set(this->get_control_nets_of_group(group_id, PinType::enable).get())) << "}, ";
                    ofs << "R: {" << utils::join(", ", nets_to_id_set(this->get_control_nets_of_group(group_id, PinType::reset).get())) << "}, ";
                    ofs << "S: {" << utils::join(", ", nets_to_id_set(this->get_control_nets_of_group(group_id, PinType::set).get())) << "}" << std::endl;

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
            auto* nl = this->get_netlist();

            // delete all modules that start with DANA
            for (const auto mod : nl->get_modules())
            {
                if (mod->get_name().find("DANA") != std::string::npos)
                {
                    nl->delete_module(mod);
                }
            }
            log_info("dataflow", "succesufully deleted old DANA modules");

            // create new modules and try to keep hierachy if possible
            std::unordered_map<u32, Module*> group_to_module;
            for (const auto& [group_id, group] : this->get_groups())
            {
                if (!group_ids.empty() && group_ids.find(group_id) == group_ids.end())
                {
                    continue;
                }

                bool gate_hierachy_matches_for_all = true;
                bool first_run                     = true;
                auto* reference_module             = nl->get_top_module();

                std::vector<Gate*> gates;
                for (const auto gate : group)
                {
                    gates.push_back(gate);

                    if (first_run)
                    {
                        reference_module = gate->get_module();
                        first_run        = false;
                    }
                    else if (!gate_hierachy_matches_for_all)
                    {
                        continue;
                    }
                    else if (gate->get_module() != reference_module)
                    {
                        gate_hierachy_matches_for_all = false;
                    }
                }

                if (!gate_hierachy_matches_for_all)
                {
                    reference_module = nl->get_top_module();
                }
                group_to_module[group_id] = nl->create_module("DANA_register_" + std::to_string(group_id), reference_module, gates);
            }
            return OK(group_to_module);
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
    }    // namespace dataflow
}    // namespace hal