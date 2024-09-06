#include "module_identification/plugin_module_identification.h"

#include "hal_core/defines.h"
#include "hal_core/utilities/result.h"
#include "module_identification/api/configuration.h"
#include "module_identification/api/module_identification.h"
#include "module_identification/api/result.h"
#include "module_identification/utils/gui_layout_locker.h"

#include <algorithm>
#include <fstream>
#include <iostream>

namespace hal
{
    ModuleIdentificationPlugin::ModuleIdentificationPlugin()
    {
        m_extensions.push_back(new GuiExtensionModuleIdentification());
    }

    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<ModuleIdentificationPlugin>();
    }

    std::string ModuleIdentificationPlugin::get_name() const
    {
        return std::string("module_identification");
    }

    std::string ModuleIdentificationPlugin::get_version() const
    {
        return std::string("0.1");
    }

    std::string ModuleIdentificationPlugin::get_description() const
    {
        return std::string("Plugin for module classification against a library of predefined types.");
    }

    std::set<std::string> ModuleIdentificationPlugin::get_dependencies() const
    {
        std::set<std::string> retval;
        retval.insert("boolean_influence");
        retval.insert("z3_utils");
        return retval;
    }

    /**
     * GUI extension
     */
    void GuiExtensionModuleIdentification::set_parameter(const std::vector<PluginParameter>& params)
    {
        for (const PluginParameter& par : params)
        {
            if (par.get_tagname() == "general_config/max_thread_count")
            {
                m_max_thread_count = atoi(par.get_value().c_str());
            }
            else if (par.get_tagname() == "general_config/max_control_signals")
            {
                m_max_control_signals = atoi(par.get_value().c_str());
            }
            else if (par.get_tagname() == "general_config/check_selector")
            {
                if (par.get_value() == "whole netlist")
                {
                    m_check_selector = 0;
                }
                else if (par.get_value() == "selected gates")
                {
                    m_check_selector = 1;
                }
                else if (par.get_value() == "selected modules")
                {
                    m_check_selector = 2;
                }
                else
                {
                    log_error("module_identification", "check selector returned not valid result");
                }
            }
            else if (par.get_tagname() == "general_config/multi_selector")
            {
                if (par.get_value() == "time_priority")
                {
                    m_multithreading_priority = module_identification::MultithreadingPriority::time_priority;
                }
                else if (par.get_value() == "memory_priority")
                {
                    m_multithreading_priority = module_identification::MultithreadingPriority::memory_priority;
                }
                else
                {
                    log_error("module_identification", "check selector returned not valid result");
                }
            }
            else if (par.get_tagname() == "exec")
            {
                m_button_clicked = (par.get_value() == "clicked");
            }
            else
            {
                for (module_identification::CandidateType cur_type : module_identification::all_checkable_candidate_types)
                {
                    if (par.get_tagname() == ("to_check_types/" + enum_to_string(cur_type)))
                    {
                        if (par.get_value() == "true")
                        {
                            if (std::find(m_types_to_check.begin(), m_types_to_check.end(), cur_type) == m_types_to_check.end())
                            {
                                m_types_to_check.push_back(cur_type);
                            }
                        }
                        else
                        {
                            auto type_it = std::find(m_types_to_check.begin(), m_types_to_check.end(), cur_type);
                            if (type_it != m_types_to_check.end())
                            {
                                m_types_to_check.erase(type_it);
                            }
                        }
                    }
                }
            }
        }
    }

    std::vector<PluginParameter> GuiExtensionModuleIdentification::get_parameter() const
    {
        std::vector<PluginParameter> retval;

        retval.push_back(PluginParameter(PluginParameter::TabName, "general_config", "Select your options for the module identification run", "1"));
        retval.push_back(PluginParameter(PluginParameter::Integer, "general_config/max_thread_count", "Maximum Number of threads utilized (default: 1)", "1"));
        retval.push_back(PluginParameter(PluginParameter::Boolean, "general_config/m_do_multithreading", "Determines whether multithreading should be done at all (default: off)", "false"));
        retval.push_back(PluginParameter(PluginParameter::Integer, "general_config/max_control_signals", "Maximum number of control signals that will be checked for (default: 3)", "3"));

        retval.push_back(PluginParameter(PluginParameter::ComboBox, "general_config/check_selector", "Select what the run should be executed on", "whole netlist;selected gates;selected modules"));

        retval.push_back(PluginParameter(PluginParameter::ComboBox, "general_config/multi_selector", "Select how  multithreading should be handled", "none;time_priority;memory_priority"));

        //second page
        retval.push_back(PluginParameter(PluginParameter::TabName, "to_check_types", "Select Module Types that shall be checked for", "1"));
        for (module_identification::CandidateType cur_type : module_identification::all_checkable_candidate_types)
        {
            std::string type_name = enum_to_string(cur_type);
            std::transform(type_name.begin(), type_name.end(), type_name.begin(), [](unsigned char c) { return std::tolower(c); });
            retval.push_back(
                PluginParameter(PluginParameter::Boolean, "to_check_types/" + enum_to_string(cur_type), "Check wether the selected gates belong to the " + type_name + " type (default: on)", "true"));
        }
        retval.push_back(PluginParameter(PluginParameter::PushButton, "exec", "Execute module identification analysis"));
        return retval;
    }

    void GuiExtensionModuleIdentification::execute_function(std::string tag, Netlist* nl, const std::vector<u32>& module_ids, const std::vector<u32>& gate_ids, const std::vector<u32>&)
    {
        std::vector<module_identification::CandidateType> types_to_check = module_identification::all_checkable_candidate_types;
        if (tag == "exec_gates")
        {
            m_max_thread_count    = 1;
            m_max_control_signals = 4;
            m_check_selector      = 1;
            m_do_multithreading   = false;
        }
        else if (tag == "exec_nl")
        {
            m_max_thread_count    = 1;
            m_max_control_signals = 4;
            m_check_selector      = 0;
            m_do_multithreading   = false;
        }
        else if (tag == "exec_module")
        {
            m_max_thread_count    = 1;
            m_max_control_signals = 4;
            m_check_selector      = 2;
            m_do_multithreading   = false;
        }
        else
        {
            if (!m_button_clicked)
            {
                return;
            }

            types_to_check = m_types_to_check;
        }

        if (!nl)
        {
            log_warning("module_identification", "Error setting paramater: no netlist loaded.");
            return;
        }

        if (GuiExtensionModuleIdentification::s_progress_indicator_function)
        {
            GuiExtensionModuleIdentification::s_progress_indicator_function(0, "module identification running ...");
        }

        module_identification::GuiLayoutLocker gll;

        auto config = module_identification::Configuration(nl)
                          .with_max_thread_count(m_max_thread_count)
                          .with_max_control_signals(m_max_control_signals)
                          .with_multithreading_priority(module_identification::MultithreadingPriority::memory_priority)
                          .with_types_to_check(types_to_check);
        // .with_progress_printer(GuiExtensionModuleIdentification::s_progress_indicator_function);

        if (m_check_selector == 0)
        {
            auto execution_res = module_identification::execute(config);
            if (execution_res.is_error())
            {
                log_error("module_identification", "{}", execution_res.get_error().get());
                return;
            }

            const auto& creation_res = execution_res.get().create_modules_in_netlist();
            if (creation_res.is_error())
            {
                log_error("module_identification", "{}", creation_res.get_error().get());
            }
        }
        else if (m_check_selector == 1)
        {
            std::vector<Gate*> gates;
            for (u32 gate_id : gate_ids)
            {
                gates.push_back(nl->get_gate_by_id(gate_id));
            }
            auto execution_res = module_identification::execute_on_gates(gates, config);
            if (execution_res.is_error())
            {
                log_error("module_identification", "{}", execution_res.get_error().get());
                return;
            }

            const auto& creation_res = execution_res.get().create_modules_in_netlist();
            if (creation_res.is_error())
            {
                log_error("module_identification", "{}", creation_res.get_error().get());
            }
        }
        else
        {
            for (u32 module_id : module_ids)
            {
                auto cur_module                = nl->get_module_by_id(module_id);
                const std::vector<Gate*> gates = cur_module->get_gates();
                auto execution_res             = module_identification::execute_on_gates(gates, config);
                if (execution_res.is_error())
                {
                    log_error("module_identification", "{}", execution_res.get_error().get());
                    return;
                }

                const auto& creation_res = execution_res.get().create_modules_in_netlist();
                if (creation_res.is_error())
                {
                    log_error("module_identification", "{}", creation_res.get_error().get());
                }
            }
        }

        // if (GuiExtensionModuleIdentification::s_progress_indicator_function)
        // {
        //     GuiExtensionModuleIdentification::s_progress_indicator_function(100, "module identification finished");
        // }
    }

    std::function<void(int, const std::string&)> GuiExtensionModuleIdentification::s_progress_indicator_function = nullptr;

    void GuiExtensionModuleIdentification::register_progress_indicator(std::function<void(int, const std::string&)> pif)
    {
        s_progress_indicator_function = pif;
    }

    std::vector<ContextMenuContribution> GuiExtensionModuleIdentification::get_context_contribution(const Netlist*, const std::vector<u32>&, const std::vector<u32>&, const std::vector<u32>&)
    {
        std::vector<ContextMenuContribution> retval;
        auto context         = ContextMenuContribution();
        context.mContributer = this;
        context.mTagname     = "exec_gates";
        context.mEntry       = "run module identification on selected gates";
        retval.push_back(context);
        auto context2         = ContextMenuContribution();
        context2.mContributer = this;
        context2.mTagname     = "exec_module";
        context2.mEntry       = "run module identification on selected modules";
        retval.push_back(context2);
        auto context3         = ContextMenuContribution();
        context3.mContributer = this;
        context3.mTagname     = "exec_nl";
        context3.mEntry       = "run module identification on netlist";
        retval.push_back(context3);
        return retval;
    }

}    // namespace hal