#include "dataflow_analysis/plugin_dataflow.h"

#include "dataflow_analysis/common/grouping.h"
#include "dataflow_analysis/common/netlist_abstraction.h"
#include "dataflow_analysis/evaluation/configuration.h"
#include "dataflow_analysis/evaluation/context.h"
#include "dataflow_analysis/evaluation/evaluation.h"
#include "dataflow_analysis/output_generation/dot_graph.h"
#include "dataflow_analysis/output_generation/json.hpp"
#include "dataflow_analysis/output_generation/json_output.h"
#include "dataflow_analysis/output_generation/state_to_module.h"
#include "dataflow_analysis/output_generation/svg_output.h"
#include "dataflow_analysis/output_generation/textual_output.h"
#include "dataflow_analysis/pre_processing/pre_processing.h"
#include "dataflow_analysis/processing/passes/group_by_control_signals.h"
#include "dataflow_analysis/processing/processing.h"
#include "dataflow_analysis/utils/timing_utils.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_utils.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/utilities/log.h"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<plugin_dataflow>();
    }

    std::string plugin_dataflow::get_name() const
    {
        return std::string("dataflow");
    }

    std::string plugin_dataflow::get_version() const
    {
        return std::string("0.1");
    }

    ProgramOptions plugin_dataflow::get_cli_options() const
    {
        ProgramOptions description;

        description.add("--dataflow", "executes the plugin dataflow");

        description.add("--path", "provide path where results should be stored", {""});

        description.add("--sizes", "(optional) sizes which should be prioritized", {""});

        return description;
    }

    bool plugin_dataflow::handle_cli_call(Netlist* nl, ProgramArguments& args)
    {
        UNUSED(args);
        std::string path;
        u32 layer;
        std::vector<u32> sizes;

        if (args.is_option_set("--path"))
        {
            if (args.get_parameter("--path").back() == '/')
                path = args.get_parameter("--path");
            else
                path = args.get_parameter("--path") + "/";
        }
        else
        {
            log_error("dataflow", "path parameter not set");
        }

        if (args.is_option_set("--sizes"))
        {
            std::istringstream f(args.get_parameter("--sizes"));
            std::string s;
            while (std::getline(f, s, ','))
            {
                sizes.emplace_back(std::stoi(s));
            }
        }

        if (execute(nl, path, sizes, false).empty())
        {
            return false;
        }

        return true;
    }

    std::vector<std::vector<Gate*>> plugin_dataflow::execute(Netlist* nl, std::string output_path, const std::vector<u32> sizes, bool draw_graph)
    {
        log("--- starting dataflow analysis ---");

        if (nl == nullptr)
        {
            log_error("dataflow", "dataflow can't be initialized (nullptr)");
            return std::vector<std::vector<Gate*>>();
        }

        // manage output
        if (!output_path.empty())
        {
            if (output_path.back() != '/')
            {
                output_path += "/";
            }

            if (sizes.empty())
            {
                output_path += nl->get_design_name() + "/";
            }
            else
            {
                output_path += nl->get_design_name() + "_sizes";
                for (const auto& size : sizes)
                {
                    output_path += "_" + std::to_string(size);
                }
                output_path += "/";
            }

            struct stat buffer;

            if (stat(output_path.c_str(), &buffer) != 0)
            {
                int return_value = system(("exec mkdir -p " + output_path).c_str());
                if (return_value != 0)
                {
                    log_warning("dataflow", "there was a problem during the creation");
                }
            }
            else
            {
                log_info("dataflow", "deleting everything in: {}", output_path);
                int return_value = system(("exec rm -r " + output_path + "*").c_str());
                if (return_value != 0)
                {
                    log_warning("dataflow", "there might have been a problem with the clean-up");
                }
            }
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

        if (!eval_config.prioritized_sizes.empty())
        {
            log_info("dataflow", "will prioritize sizes {}", utils::join(", ", sizes));
            log_info("dataflow", "");
        }

        auto nl_copy       = netlist_utils::copy_netlist(nl);
        auto netlist_abstr = dataflow::pre_processing::run(nl_copy.get());

        auto initial_grouping = netlist_abstr.create_initial_grouping();
        std::shared_ptr<dataflow::Grouping> final_grouping;

        total_time += (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin_time).count() / 1000;

        log_info("dataflow", "");

        nlohmann::json output_json;

        u32 iteration = 0;
        while (true)
        {
            log("iteration {}", iteration);
            // log("iteration {}, phase {}", iteration, eval_ctx.phase);

            // main dataflow analysis

            begin_time = std::chrono::high_resolution_clock::now();

            auto processing_result = dataflow::processing::run(config, initial_grouping);
            auto eval_result       = dataflow::evaluation::run(eval_config, eval_ctx, initial_grouping, processing_result);

            total_time += (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin_time).count() / 1000;

            std::string file_name = "result" + std::to_string(iteration) + ".txt";
            dataflow::textual_output::write_register_output(eval_result.merged_result, output_path, file_name);

            dataflow::json_output::save_state_to_json(iteration, netlist_abstr, processing_result, eval_result, false, output_json);

            // end of analysis(?)

            if (eval_result.is_final_result)
            {
                log("got final result");
                final_grouping = eval_result.merged_result;
                break;
            }

            initial_grouping = eval_result.merged_result;

            iteration++;
        }

        // svg_output::save_state_to_svg(final_grouping, output_path, "svg_result.json");

        // add some meta data to json
        output_json[netlist_abstr.nl->get_design_name()]["sequential_gates"] = netlist_abstr.all_sequential_gates.size();
        output_json[netlist_abstr.nl->get_design_name()]["all_gates"]        = netlist_abstr.nl->get_gates().size();
        output_json[netlist_abstr.nl->get_design_name()]["total_time"]       = total_time;

        std::ofstream(output_path + "eval.json", std::ios_base::app) << std::setw(4) << output_json << std::endl;

        // dot_graph::create_graph(final_grouping, output_path + "result_", {"png", "pdf"});
        //dataflow::dot_graph::create_graph(final_grouping, output_path + "result_", {"png"});

        if (draw_graph)
        {
            dataflow::dot_graph::create_graph(final_grouping, output_path + "result_", {"pdf"});
        }

        log("dataflow processing finished in {:3.2f}s", total_time);

        return dataflow::state_to_module::create_sets(nl, final_grouping);
    }
}    // namespace hal