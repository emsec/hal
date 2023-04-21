#include "dataflow_analysis/dataflow.h"

#include "dataflow_analysis/evaluation/evaluation.h"
#include "dataflow_analysis/output_generation/dot_graph.h"
#include "dataflow_analysis/output_generation/json_output.h"
#include "dataflow_analysis/output_generation/state_to_module.h"
#include "dataflow_analysis/output_generation/textual_output.h"
#include "dataflow_analysis/pre_processing/pre_processing.h"
#include "dataflow_analysis/processing/processing.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

#include <fstream>

namespace hal
{
    namespace dataflow
    {
        std::shared_ptr<dataflow::Grouping> analyze(Netlist* nl,
                                                    std::filesystem::path out_path,
                                                    const std::vector<u32>& sizes,
                                                    bool register_stage_identification,
                                                    const std::vector<std::vector<u32>>& known_groups,
                                                    const u32 bad_group_size)
        {
            if (nl == nullptr)
            {
                log_error("dataflow", "dataflow can't be initialized (nullptr)");
                return nullptr;
            }

            // manage output
            if (!out_path.empty())
            {
                if (sizes.empty())
                {
                    out_path /= nl->get_design_name();
                }
                else
                {
                    out_path /= nl->get_design_name() + "_sizes";
                    for (const auto& size : sizes)
                    {
                        out_path += "_" + std::to_string(size);
                    }
                }

                struct stat buffer;

                if (stat(out_path.c_str(), &buffer) != 0)
                {
                    int return_value = system(("exec mkdir -p " + out_path.string()).c_str());
                    if (return_value != 0)
                    {
                        log_warning("dataflow", "there was a problem during the creation");
                    }
                }
                else
                {
                    log_info("dataflow", "deleting everything in: {}", out_path.string());
                    int return_value = system(("exec rm -r " + out_path.string() + "*").c_str());
                    if (return_value != 0)
                    {
                        log_warning("dataflow", "there might have been a problem with the clean-up");
                    }
                }
            }
            else
            {
                log_error("dataflow", "you need to provide a valid output path");
                return nullptr;
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
                log_info("dataflow", "");
            }

            auto copy_res = nl->copy();
            if (copy_res.is_error())
            {
                log_error("dataflow", "failed to create copy of netlist");
                return nullptr;
            }
            auto netlist_abstr = dataflow::pre_processing::run(copy_res.get().get(), register_stage_identification);

            auto initial_grouping = std::make_shared<dataflow::Grouping>(netlist_abstr, known_groups);
            std::shared_ptr<dataflow::Grouping> final_grouping;

            total_time += (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin_time).count() / 1000;

            log_info("dataflow", "");

            nlohmann::json output_json;

            u32 iteration = 0;
            while (true)
            {
                log_info("dataflow", "iteration {}", iteration);

                // main dataflow analysis
                begin_time = std::chrono::high_resolution_clock::now();

                auto processing_result = dataflow::processing::run(config, initial_grouping);
                auto eval_result       = dataflow::evaluation::run(eval_config, eval_ctx, initial_grouping, processing_result);

                total_time += (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin_time).count() / 1000;

                std::string file_name = "result" + std::to_string(iteration) + ".txt";
                dataflow::textual_output::write_register_output(eval_result.merged_result, out_path, file_name);

                dataflow::json_output::save_state_to_json(iteration, netlist_abstr, processing_result, eval_result, false, output_json);

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

            // add some meta data to json
            output_json[netlist_abstr->nl->get_design_name()]["sequential_gates"] = netlist_abstr->all_sequential_gates.size();
            output_json[netlist_abstr->nl->get_design_name()]["all_gates"]        = netlist_abstr->nl->get_gates().size();
            output_json[netlist_abstr->nl->get_design_name()]["total_time"]       = total_time;

            std::ofstream(out_path / "eval.json", std::ios_base::app) << std::setw(4) << output_json << std::endl;

            log_info("dataflow", "dataflow processing finished in {:3.2f}s", total_time);

            return final_grouping;
        }

        bool write_dot_graph(const std::shared_ptr<dataflow::Grouping> grouping, const std::filesystem::path& out_path, const std::unordered_set<u32>& ids)
        {
            return dataflow::dot_graph::create_graph(grouping, out_path, {}, ids);
        }

        bool create_grouping_modules(const std::shared_ptr<dataflow::Grouping> grouping, Netlist* nl, const std::unordered_set<u32>& ids)
        {
            return dataflow::state_to_module::create_modules(nl, grouping, ids);
        }
    }    // namespace dataflow
}    // namespace hal