#include "dataflow_analysis/plugin_dataflow.h"

#include "dataflow_analysis/common/grouping.h"
#include "dataflow_analysis/utils/gui_layout_locker.h"
#include "dataflow_analysis/utils/timing_utils.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/utilities/log.h"

#include <chrono>
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
        m_extensions.push_back(new CliExtensionDataflow());
        m_extensions.push_back(new GuiExtensionDataflow());
    }

    ProgramOptions CliExtensionDataflow::get_cli_options() const
    {
        ProgramOptions description;

        description.add("--dataflow", "execute the dataflow plugin");

        description.add("--path", "specify output path", {""});

        description.add("--sizes", "(optional) specify sizes to be prioritized", {""});

        description.add("--bad_group_size", "(optional) specify the bad group size", {""});

        return description;
    }

    bool CliExtensionDataflow::handle_cli_call(Netlist* nl, ProgramArguments& args)
    {
        UNUSED(args);

        dataflow::Configuration config(nl);
        std::string path;

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
                config.expected_sizes.push_back(std::stoi(s));
            }
        }

        if (args.is_option_set("--bad_group_size"))
        {
            std::istringstream f(args.get_parameter("--bad_group_size"));
            std::string s;
            while (std::getline(f, s, ','))
            {
                config.min_group_size = std::stoi(s);
            }
        }

        auto grouping_res = dataflow::analyze(config);
        if (grouping_res.is_error())
        {
            log_error("dataflow", "dataflow analysis failed:\n{}", grouping_res.get_error().get());
            return false;
        }

        return true;
    }

    std::vector<PluginParameter> GuiExtensionDataflow::get_parameter() const
    {
        std::vector<PluginParameter> retval;
        retval.push_back(PluginParameter(PluginParameter::String, "sizes", "Expected register size (optional)", "8,16,32"));
        retval.push_back(PluginParameter(PluginParameter::Integer, "min_group_size", "Minimum size of a group, smaller groups will be penalized (default: 8)", "8"));
        retval.push_back(PluginParameter(PluginParameter::ExistingDir, "output", "Directory for results (required)"));
        retval.push_back(PluginParameter(PluginParameter::Boolean, "register_stage_identification", "Enable register stage identification (default: off)", "false"));
        retval.push_back(PluginParameter(PluginParameter::Boolean, "write_txt", "Write a .txt file containing analysis results (default: on)", "true"));
        retval.push_back(PluginParameter(PluginParameter::Boolean, "create_modules", "Create modules for all registers (default: on)", "true"));
        retval.push_back(PluginParameter(PluginParameter::Boolean, "write_dot", "Write a .dot file describing the dataflow graph (not recommended for large netlist; default: off)", "false"));
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
                    m_expected_sizes.emplace_back(std::stoi(s));
                }
            }
            else if (par.get_tagname() == "min_group_size")
            {
                m_min_group_size = atoi(par.get_value().c_str());
            }
            else if (par.get_tagname() == "write_txt")
            {
                m_write_txt = (par.get_value() == "true");
            }
            else if (par.get_tagname() == "write_dot")
            {
                m_write_dot = (par.get_value() == "true");
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
                m_enable_register_stages = (par.get_value() == "true");
            }
            else if (par.get_tagname() == "exec")
            {
                m_button_clicked = (par.get_value() == "clicked");
            }
        }
    }

    void GuiExtensionDataflow::execute_function(std::string tag, Netlist* nl, const std::vector<u32>&, const std::vector<u32>&, const std::vector<u32>&)
    {
        UNUSED(tag);

        if (!m_button_clicked)
            return;
        if (!nl)
        {
            log_warning("dataflow", "Error setting paramater: no netlist loaded.");
            return;
        }

        if (GuiExtensionDataflow::s_progress_indicator_function)
        {
            GuiExtensionDataflow::s_progress_indicator_function(0, "dataflow analysis running ...");
        }

        dataflow::GuiLayoutLocker gll;

        auto config = dataflow::Configuration(nl)
                          .with_expected_sizes(m_expected_sizes)
                          .with_min_group_size(m_min_group_size)
                          .with_register_stage_identification(m_enable_register_stages)
                          .with_control_pin_types({PinType::clock, PinType::enable, PinType::reset, PinType::set})
                          .with_gate_types({GateTypeProperty::ff});
        auto grouping_res = dataflow::analyze(config);
        if (grouping_res.is_error())
        {
            log_error("dataflow", "dataflow analysis failed:\n{}", grouping_res.get_error().get());
            return;
        }
        auto grouping = grouping_res.get();

        if (m_write_dot)
        {
            if (const auto res = grouping.write_dot(m_output_path); res.is_error())
            {
                log_error("dataflow", "could not write .dot file:\n{}", res.get_error().get());
            }
        }

        if (m_write_txt)
        {
            if (const auto res = grouping.write_txt(m_output_path); res.is_error())
            {
                log_error("dataflow", "could not write .txt file:\n{}", res.get_error().get());
            }
        }

        if (m_create_modules)
        {
            if (const auto res = grouping.create_modules(); res.is_error())
            {
                log_error("dataflow", "could not create modules:\n{}", res.get_error().get());
            }
        }

        if (GuiExtensionDataflow::s_progress_indicator_function)
        {
            GuiExtensionDataflow::s_progress_indicator_function(100, "dataflow analysis finished");
        }
    }

    std::vector<std::vector<Gate*>> plugin_dataflow::execute(Netlist* nl,
                                                             std::string output_path,
                                                             const std::vector<u32> sizes,
                                                             bool draw_graph,
                                                             bool create_modules,
                                                             bool register_stage_identification,
                                                             std::vector<std::vector<u32>> known_groups,
                                                             u32 min_group_size)
    {
        auto config = dataflow::Configuration(nl)
                          .with_min_group_size(min_group_size)
                          .with_expected_sizes(sizes)
                          .with_known_groups(known_groups)
                          .with_register_stage_identification(register_stage_identification)
                          .with_control_pin_types({PinType::clock, PinType::enable, PinType::reset, PinType::set})
                          .with_gate_types({GateTypeProperty::ff});

        const auto grouping_res = dataflow::analyze(config);
        if (grouping_res.is_error())
        {
            log_error("dataflow", "dataflow analysis failed:\n{}", grouping_res.get_error().get());
            return {};
        }
        const auto grouping = grouping_res.get();

        if (draw_graph)
        {
            if (const auto res = grouping.write_dot(output_path); res.is_error())
            {
                log_error("dataflow", "could not write DOT graph to file:\n{}", res.get_error().get());
            }
        }

        if (create_modules)
        {
            if (const auto res = grouping.create_modules(); res.is_error())
            {
                log_error("dataflow", "could not create modules:\n{}", res.get_error().get());
            }
        }

        return grouping.get_groups_as_list();
    }

    std::function<void(int, const std::string&)> GuiExtensionDataflow::s_progress_indicator_function = nullptr;

    void GuiExtensionDataflow::register_progress_indicator(std::function<void(int, const std::string&)> pif)
    {
        s_progress_indicator_function = pif;
    }

}    // namespace hal
