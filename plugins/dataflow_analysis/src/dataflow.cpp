#include "dataflow_analysis/dataflow.h"

#include "dataflow_analysis/evaluation/evaluation.h"
#include "dataflow_analysis/pre_processing/pre_processing.h"
#include "dataflow_analysis/processing/processing.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/module.h"
#include "hal_core/utilities/log.h"

namespace hal
{
    namespace dataflow
    {
        Configuration& Configuration::with_min_group_size(u32 size)
        {
            this->min_group_size = size;
            return *this;
        }

        Configuration& Configuration::with_expected_sizes(const std::vector<u32>& sizes)
        {
            this->expected_sizes = sizes;
            return *this;
        }

        Configuration& Configuration::with_known_groups(const std::vector<Module*>& groups)
        {
            for (const auto* mod : groups)
            {
                std::vector<u32> group;
                const auto& gates = mod->get_gates();
                std::transform(gates.cbegin(), gates.cend(), std::back_inserter(group), [](const Gate* g) { return g->get_id(); });
                this->known_groups.push_back(std::move(group));
            }
            return *this;
        }

        Configuration& Configuration::with_known_groups(const std::vector<hal::Grouping*>& groups)
        {
            for (const auto* grp : groups)
            {
                std::vector<u32> group;
                const auto& gates = grp->get_gates();
                std::transform(gates.cbegin(), gates.cend(), std::back_inserter(group), [](const Gate* g) { return g->get_id(); });
                this->known_groups.push_back(std::move(group));
            }
            return *this;
        }

        Configuration& Configuration::with_known_groups(const std::vector<std::vector<Gate*>>& groups)
        {
            for (const auto& gates : groups)
            {
                std::vector<u32> group;
                std::transform(gates.cbegin(), gates.cend(), std::back_inserter(group), [](const Gate* g) { return g->get_id(); });
                this->known_groups.push_back(std::move(group));
            }
            return *this;
        }

        Configuration& Configuration::with_known_groups(const std::vector<std::vector<u32>>& groups)
        {
            this->known_groups = groups;
            return *this;
        }

        Configuration& Configuration::with_register_stage_identification(bool enable)
        {
            this->enable_register_stages = enable;
            return *this;
        }

        hal::Result<dataflow::Result> analyze(Netlist* nl, const Configuration& config)
        {
            if (nl == nullptr)
            {
                return ERR("netlist is a nullptr");
            }

            // set up dataflow analysis
            double total_time = 0;
            auto begin_time   = std::chrono::high_resolution_clock::now();

            dataflow::processing::Configuration proc_config;
            proc_config.pass_layers = 2;
            proc_config.num_threads = std::thread::hardware_concurrency();

            dataflow::evaluation::Context eval_ctx;

            dataflow::evaluation::Configuration eval_config;
            eval_config.prioritized_sizes = config.expected_sizes;
            eval_config.min_group_size    = config.min_group_size;

            if (!eval_config.prioritized_sizes.empty())
            {
                log_info("dataflow", "will prioritize sizes {}", utils::join(", ", config.expected_sizes));
            }

            auto netlist_abstr    = dataflow::pre_processing::run(nl, config.enable_register_stages);
            auto initial_grouping = std::make_shared<dataflow::Grouping>(netlist_abstr, config.known_groups);
            std::shared_ptr<dataflow::Grouping> final_grouping;

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

            total_time = (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin_time).count() / 1000;

            log_info("dataflow", "dataflow processing finished in {:3.2f}s", total_time);

            return OK(dataflow::Result(nl, *final_grouping));
        }
    }    // namespace dataflow
}    // namespace hal