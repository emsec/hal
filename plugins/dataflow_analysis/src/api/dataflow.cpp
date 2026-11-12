#include "dataflow_analysis/api/dataflow.h"

#include "dataflow_analysis/evaluation/evaluation.h"
#include "dataflow_analysis/pre_processing/pre_processing.h"
#include "dataflow_analysis/processing/processing.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/plugin_system/user_feedback.h"
#include "hal_core/utilities/log.h"

#include <algorithm>

namespace hal
{
    namespace dataflow
    {
        hal::Result<dataflow::Result> analyze(const Configuration& config)
        {
            if (config.netlist == nullptr)
            {
                return ERR("netlist is a nullptr");
            }

            if (config.gate_types.empty())
            {
                return ERR("no gate types specified");
            }

            if (config.control_pin_types.empty())
            {
                return ERR("no control pin types specified");
            }

            for (const auto* gates : {&config.gates, &config.excluded_gates})
            {
                for (const auto* g : *gates)
                {
                    if (g == nullptr || g->get_netlist() != config.netlist)
                    {
                        return ERR("the gate selection or exclusion contains a gate that is not part of the netlist");
                    }
                }
            }

            if (!config.gates.empty())
            {
                const auto not_target_type = std::count_if(config.gates.begin(), config.gates.end(), [&config](const Gate* g) { return config.gate_types.find(g->get_type()) == config.gate_types.end(); });
                if (not_target_type > 0)
                {
                    log_warning("dataflow", "{} gates of the gate selection are not of a target gate type and are ignored", not_target_type);
                }
            }

            // the analysis reports its progress to the GUI from here on, so it also has to dismiss the progress
            // indicator again once it is done, no matter whether it was invoked from the GUI or from a script
            const user_feedback::ProgressScope progress("dataflow analysis …");

            // set up dataflow analysis
            double total_time = 0;
            auto begin_time   = std::chrono::high_resolution_clock::now();

            dataflow::processing::Configuration proc_config;
            proc_config.pass_layers              = 2;
            proc_config.num_threads              = std::thread::hardware_concurrency();
            proc_config.enforce_type_consistency = config.enforce_type_consistency;
            proc_config.has_known_groups         = !config.known_net_groups.empty();

            dataflow::evaluation::Context eval_ctx;

            dataflow::evaluation::Configuration eval_config;
            eval_config.prioritized_sizes = config.expected_sizes;
            eval_config.min_group_size    = config.min_group_size;

            if (!eval_config.prioritized_sizes.empty())
            {
                log_info("dataflow", "will prioritize sizes {}", utils::join(", ", config.expected_sizes));
            }

            std::shared_ptr<dataflow::Grouping> initial_grouping = nullptr;
            auto netlist_abstr                                   = dataflow::pre_processing::run(config, initial_grouping);
            if (netlist_abstr.target_gates.empty() && (!config.gates.empty() || !config.excluded_gates.empty()))
            {
                return ERR("no gate is left to group after applying the gate selection and exclusion");
            }
            std::shared_ptr<dataflow::Grouping> final_grouping   = nullptr;

            u32 iteration = 0;
            while (true)
            {
                log_info("dataflow", "iteration {}", iteration);

                // main dataflow analysis
                auto processing_result = dataflow::processing::run(proc_config, initial_grouping);
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

            dataflow::processing::clear();

            total_time = (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin_time).count() / 1000;

            log_info("dataflow", "dataflow processing finished in {:3.2f}s", total_time);

            return OK(dataflow::Result(config.netlist, *final_grouping));
        }
    }    // namespace dataflow
}    // namespace hal