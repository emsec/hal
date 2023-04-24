#include "dataflow_analysis/dataflow.h"

#include "dataflow_analysis/evaluation/evaluation.h"
#include "dataflow_analysis/pre_processing/pre_processing.h"
#include "dataflow_analysis/processing/processing.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

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

        hal::Result<dataflow::Result>
            analyze(Netlist* nl, const std::vector<u32>& sizes, bool register_stage_identification, const std::vector<std::vector<u32>>& known_groups, const u32 bad_group_size)
        {
            if (nl == nullptr)
            {
                return ERR("netlist is a nullptr");
            }

            // set up dataflow analysis
            double total_time = 0;
            auto begin_time   = std::chrono::high_resolution_clock::now();

            dataflow::processing::Configuration config;
            config.pass_layers = 2;
            config.num_threads = std::thread::hardware_concurrency();

            dataflow::evaluation::Context eval_ctx;

            dataflow::evaluation::Configuration eval_config;
            eval_config.prioritized_sizes = sizes;
            eval_config.bad_group_size    = bad_group_size;

            if (!eval_config.prioritized_sizes.empty())
            {
                log_info("dataflow", "will prioritize sizes {}", utils::join(", ", sizes));
            }

            auto netlist_abstr    = dataflow::pre_processing::run(nl, register_stage_identification);
            auto initial_grouping = std::make_shared<dataflow::Grouping>(netlist_abstr, known_groups);
            std::shared_ptr<dataflow::Grouping> final_grouping;

            u32 iteration = 0;
            while (true)
            {
                log_info("dataflow", "iteration {}", iteration);

                // main dataflow analysis
                auto processing_result = dataflow::processing::run(config, initial_grouping);
                auto eval_result       = dataflow::evaluation::run(eval_config, eval_ctx, initial_grouping, processing_result);

                // end of analysis(?)
                if (eval_result.is_final_result)
                {
                    log_info("dataflow", "got final result");
                    final_grouping = eval_result.merged_result;
                    break;
                }

                initial_grouping = eval_result.merged_result;

                iteration++;
            }

            total_time = (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin_time).count() / 1000;

            log_info("dataflow", "dataflow processing finished in {:3.2f}s", total_time);

            return OK(dataflow::Result(nl, *final_grouping));
        }

        hal::Result<std::monostate> write_dot(const dataflow::Result& result, const std::filesystem::path& out_path, const std::unordered_set<u32>& group_ids)
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
                log_info("dataflow", "replacing invalid file extension '.{}' with '.dot'...", write_path.extension().string());
                write_path.replace_extension("dot");
            }

            log_info("dataflow", "writing dataflow graph to '{}'...", write_path.string());

            std::ofstream ofs(write_path, std::ofstream::out);
            if (ofs)
            {
                ofs << "digraph {\n\tnode [shape=box fillcolor=white style=filled];\n";

                // print node
                for (const auto& [group_id, gates] : result.get_groups())
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
                for (const auto& [group_id, gates] : result.get_groups())
                {
                    if (!group_ids.empty() && group_ids.find(group_id) == group_ids.end())
                    {
                        continue;
                    }

                    for (auto suc_id : result.get_group_successors(group_id).get())
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

        hal::Result<std::monostate> write_txt(const dataflow::Result& result, const std::filesystem::path& out_path, const std::unordered_set<u32>& group_ids)
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
                log_info("dataflow", "replacing invalid file extension '.{}' with '.txt'...", write_path.extension().string());
                write_path.replace_extension("txt");
            }

            log_info("dataflow", "writing dataflow gate groups to '{}'...", write_path.string());

            std::ofstream ofs(write_path, std::ofstream::out);
            if (ofs)
            {
                ofs << "State:";
                ofs << "\n\n";

                for (const auto& [group_id, gates] : result.get_groups())
                {
                    if (!group_ids.empty() && group_ids.find(group_id) == group_ids.end())
                    {
                        continue;
                    }

                    ofs << "ID:" << group_id << ", ";
                    ofs << "Size:" << gates.size() << ", ";
                    ofs << "CLK: {" << utils::join(", ", nets_to_id_set(result.get_control_nets_of_group(group_id, PinType::clock).get())) << "}, ";
                    ofs << "EN: {" << utils::join(", ", nets_to_id_set(result.get_control_nets_of_group(group_id, PinType::enable).get())) << "}, ";
                    ofs << "R: {" << utils::join(", ", nets_to_id_set(result.get_control_nets_of_group(group_id, PinType::reset).get())) << "}, ";
                    ofs << "S: {" << utils::join(", ", nets_to_id_set(result.get_control_nets_of_group(group_id, PinType::set).get())) << "}" << std::endl;

                    if (auto res = result.get_group_successors(group_id); res.is_ok())
                    {
                        auto unsorted_successors = res.get();
                        ofs << "  Successors:   {" + utils::join(", ", std::set<u32>(unsorted_successors.begin(), unsorted_successors.end())) << "}" << std::endl;
                    }

                    if (auto res = result.get_group_predecessors(group_id); res.is_ok())
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

        hal::Result<std::unordered_map<u32, Module*>> create_modules(const dataflow::Result& result, const std::unordered_set<u32>& group_ids)
        {
            auto* nl = result.get_netlist();

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
            for (const auto& [group_id, group] : result.get_groups())
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

        std::vector<std::vector<Gate*>> get_group_list(const dataflow::Result& result, const std::unordered_set<u32>& group_ids)
        {
            std::vector<std::vector<Gate*>> groups;
            for (const auto& [group_id, group] : result.get_groups())
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