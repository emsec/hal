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
#include "dataflow_analysis/utils/gui_layout_locker.h"
#include "dataflow_analysis/utils/timing_utils.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
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

    std::string plugin_dataflow::get_description() const
    {
        return "Dataflow analysis for gate-level netlist reverse engineering";
    }

    std::string plugin_dataflow::get_version() const
    {
        return std::string("0.1");
    }

    plugin_dataflow::plugin_dataflow()
    {
        m_extensions.push_back(new CliExtensionDataflow(this));
        m_extensions.push_back(new GuiExtensionDataflow(this));
    }

    ProgramOptions CliExtensionDataflow::get_cli_options() const
    {
        ProgramOptions description;

        description.add("--dataflow", "execute the dataflow plugin");

        description.add("--path", "specify result path", {""});

        description.add("--sizes", "(optional) specify sizes to be prioritized", {""});

        description.add("--bad_group_size", "(optional) specify the bad group size", {""});

        return description;
    }

    bool CliExtensionDataflow::handle_cli_call(Netlist* nl, ProgramArguments& args)
    {
        UNUSED(args);
        std::string path;
        std::vector<u32> sizes;
        u32 bad_group_size = 7;

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

        if (args.is_option_set("--bad_group_size"))
        {
            std::istringstream f(args.get_parameter("--bad_group_size"));
            std::string s;
            while (std::getline(f, s, ','))
            {
                bad_group_size = std::stoi(s);
            }
        }

        if (m_parent->execute(nl, path, sizes, false, false, false, {{}}).empty(), bad_group_size)
        {
            return false;
        }

        return true;
    }

    std::vector<PluginParameter> GuiExtensionDataflow::get_parameter() const
    {
        std::vector<PluginParameter> retval;
        retval.push_back(PluginParameter(PluginParameter::String, "sizes", "Expected register size (optional)", "8,16,32"));
        retval.push_back(PluginParameter(PluginParameter::Integer, "bad_groups", "Bad group size (default: 7)", "7"));
        retval.push_back(PluginParameter(PluginParameter::ExistingDir, "output", "Directory for results (required)"));
        retval.push_back(
            PluginParameter(PluginParameter::Boolean, "register_stage_identification", "Register Stage Identification (default: off, this rule can sometimes can be too restrictive)", "false"));
        retval.push_back(PluginParameter(PluginParameter::Boolean, "create_modules", "Let DANA create HAL modules (default: on)", "true"));
        retval.push_back(PluginParameter(PluginParameter::Boolean, "draw", "Draw dot graph (not recommended for large netlist)", "false"));
        retval.push_back(PluginParameter(PluginParameter::PushButton, "exec", "Execute dataflow analysis"));
        return retval;
    }

    void GuiExtensionDataflow::set_parameter(const std::vector<PluginParameter>& params)
    {
        for (const PluginParameter& par : params)
        {
            if (par.get_tagname() == "sizes")
            {
                std::istringstream f(par.get_value());
                std::string s;
                while (std::getline(f, s, ','))
                {
                    m_sizes.emplace_back(std::stoi(s));
                }
            }
            else if (par.get_tagname() == "bad_groups")
            {
                m_bad_groups = atoi(par.get_value().c_str());
            }
            else if (par.get_tagname() == "draw")
            {
                m_draw_graph = (par.get_value() == "true");
            }
            else if (par.get_tagname() == "output")
            {
                m_output_path = par.get_value();
            }
            else if (par.get_tagname() == "create_modules")
            {
                m_create_modules = (par.get_value() == "true");
            }
            else if (par.get_tagname() == "register_stage_identification")
            {
                m_register_stage_identification = (par.get_value() == "true");
            }
            else if (par.get_tagname() == "exec")
            {
                m_button_clicked = (par.get_value() == "clicked");
            }
        }
    }

    void GuiExtensionDataflow::execute_function(std::string tag, Netlist* nl, const std::vector<u32>&, const std::vector<u32>&, const std::vector<u32>&)
    {
        if (!m_button_clicked)
            return;
        if (!nl)
        {
            log_warning("dataflow", "Error setting paramater: no netlist loaded.");
            return;
        }

        m_parent->execute(nl, m_output_path, m_sizes, m_draw_graph, m_create_modules, m_register_stage_identification, {}, m_bad_groups);
    }

    std::vector<std::vector<Gate*>> plugin_dataflow::execute(Netlist* nl,
                                                             std::string output_path,
                                                             const std::vector<u32> sizes,
                                                             bool draw_graph,
                                                             bool create_modules,
                                                             bool register_stage_identification,
                                                             std::vector<std::vector<u32>> known_groups,
                                                             u32 bad_group_size)
    {
        if (nl == nullptr)
        {
            log_error("dataflow", "dataflow can't be initialized (nullptr)");
            return std::vector<std::vector<Gate*>>();
        }

        if (GuiExtensionDataflow::s_progress_indicator_function)
            GuiExtensionDataflow::s_progress_indicator_function(0, "dataflow analysis running ...");

        dataflow::GuiLayoutLocker gll;

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
        else
        {
            log_error("dataflow", "you need to provide a valid output path");
            return std::vector<std::vector<Gate*>>();
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
            return std::vector<std::vector<Gate*>>();
        }
        auto nl_copy       = copy_res.get();
        auto netlist_abstr = dataflow::pre_processing::run(nl_copy.get(), register_stage_identification);

        auto initial_grouping = netlist_abstr.create_initial_grouping(known_groups);
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

        if (draw_graph)
        {
            write_dot_graph(final_grouping, output_path);
        }

        if (create_modules)
        {
            create_modules(final_grouping, nl);
        }

        log("dataflow processing finished in {:3.2f}s", total_time);

        if (GuiExtensionDataflow::s_progress_indicator_function)
            GuiExtensionDataflow::s_progress_indicator_function(100, "dataflow analysis finished");

        return dataflow::state_to_module::create_sets(nl, final_grouping);
    }

    std::shared_ptr<dataflow::Grouping> analyze(Netlist* nl,
                                                const std::filesystem::path& out_path,
                                                const std::vector<u32>& sizes,
                                                bool register_stage_identification,
                                                const std::vector<std::vector<u32>>& known_groups,
                                                const u32 bad_group_size)
    {
        // TODO
    }

    bool write_dot_graph(const std::shared_ptr<Grouping> state, const std::filesystem::path& out_path, const std::unordered_set<u32>& ids)
    {
        dataflow::dot_graph::create_graph(state, out_path + "result_", {"pdf"}, ids);
    }

    bool create_modules(const std::shared_ptr<Grouping> state, Netlist* nl, const std::unordered_set<u32>& ids)
    {
        dataflow::state_to_module::create_modules(nl, state, ids);
    }

    std::function<void(int, const std::string&)> GuiExtensionDataflow::s_progress_indicator_function = nullptr;

    void GuiExtensionDataflow::register_progress_indicator(std::function<void(int, const std::string&)> pif)
    {
        s_progress_indicator_function = pif;
    }

}    // namespace hal
