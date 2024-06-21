#include "netlist_simulator_study/netlist_simulator_study.h"

#include "gui/gui_api/gui_api.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/netlist/netlist_parser/netlist_parser_manager.h"
#include "hal_core/netlist/project_manager.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "netlist_simulator_controller/netlist_simulator_controller.h"
#include "netlist_simulator_controller/plugin_netlist_simulator_controller.h"

#include <QSettings>
#include <algorithm>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <random>
#include <vector>

namespace hal
{
    extern Netlist* gNetlist;

    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<NetlistSimulatorStudyPlugin>();
    }

    NetlistSimulatorStudyPlugin::NetlistSimulatorStudyPlugin()
    {
        m_gui_extension = nullptr;
    }

    std::string NetlistSimulatorStudyPlugin::get_name() const
    {
        return std::string("netlist_simulator_study");
    }

    std::string NetlistSimulatorStudyPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void NetlistSimulatorStudyPlugin::on_load()
    {
        m_gui_extension           = new GuiExtensionNetlistSimulatorStudy;
        m_gui_extension->m_parent = this;
        m_extensions.push_back(m_gui_extension);
    }

    void NetlistSimulatorStudyPlugin::on_unload()
    {
        delete_extension(m_gui_extension);
    }

    /**
      * Returns plugin dependencies. Depends on GUI, simulation controller, verilator, waveform viewer
      * @return
      */
    std::set<std::string> NetlistSimulatorStudyPlugin::get_dependencies() const
    {
        std::set<std::string> retval;
        retval.insert("netlist_simulator_controller");
        retval.insert("hal_gui");
        retval.insert("waveform_viewer");
        retval.insert("verilator");
        retval.insert("liberty_parser");
        retval.insert("verilog_parser");
        return retval;
    }

    bool NetlistSimulatorStudyPlugin::simulate(std::filesystem::path sim_input, std::vector<const Net*> probes)
    {
        // read the original netlist
        ProjectManager* pm = ProjectManager::instance();
        std::filesystem::path project_dir_path(pm->get_project_directory().string());

        if (!std::filesystem::exists(project_dir_path / "original/original.v"))
        {
            log_error("netlist_simulator_study", "Original netlist file is missing!");
            return false;
        }

        m_original_netlist = netlist_factory::load_netlist(project_dir_path / "original/original.v", gNetlist->get_gate_library()->get_path());

        if (!std::filesystem::exists(sim_input))
        {
            return false;
        }

        enum
        {
            VCD,
            CSV,
            SAL
        } input_file_format;

        if (sim_input.extension() == ".vcd")
            input_file_format = VCD;
        else if (sim_input.extension() == ".csv")
            input_file_format = CSV;
        else if (sim_input.extension() == ".json")
            input_file_format = SAL;
        else
        {
            log_warning("netlist_simulator_study", "Simulation input file '{}’ has unknown extension.", sim_input.string());
            return false;
        }

        // create new simulation controller
        NetlistSimulatorControllerPlugin* ctrlPlug = static_cast<NetlistSimulatorControllerPlugin*>(plugin_manager::get_plugin_instance("netlist_simulator_controller"));
        if (!ctrlPlug)
        {
            log_warning("netlist_simulator_study", "Plugin 'netlist_simulator_controller' not found");
            return false;
        }
        m_simul_controller = ctrlPlug->create_simulator_controller();
        m_simul_controller.get()->add_gates(m_original_netlist.get()->get_gates());
        m_simul_controller.get()->set_no_clock_used();
        m_simul_controller.get()->create_simulation_engine("verilator");

        switch (input_file_format)
        {
            case VCD:
                if (!m_simul_controller.get()->import_vcd(sim_input.string(), NetlistSimulatorController::FilterInputFlag::CompleteNetlist))
                {
                    log_warning("netlist_simulator_study", "Cannot parse simulation input file '{}'.", sim_input.string());
                    return false;
                }
                break;
            case CSV:
                m_simul_controller.get()->import_csv(sim_input.string(), NetlistSimulatorController::FilterInputFlag::CompleteNetlist);
                break;
            case SAL:
                m_simul_controller.get()->import_simulation(sim_input.parent_path().string(), NetlistSimulatorController::FilterInputFlag::CompleteNetlist);
                break;
        }

        m_simul_controller.get()->simulate_only_probes(probes);

        m_simul_controller.get()->emit_run_simulation();

        return true;
    }

    //----------------------

    GuiExtensionNetlistSimulatorStudy::GuiExtensionNetlistSimulatorStudy()
    {
        m_parameter.push_back(PluginParameter(PluginParameter::ExistingFile, "sim_input", "Simulation input file"));
        m_parameter.push_back(PluginParameter(PluginParameter::Boolean,
                                              "net_picker",
                                              "You are only allowed to select 5 nets to probe.\nSelect up to 5 at random if more then 5 are selected?\nOtherwise the execution just stops.",
                                              "true"));
        m_parameter.push_back(PluginParameter(PluginParameter::PushButton, "simulate", "Start Simulation"));
    }

    std::vector<PluginParameter> GuiExtensionNetlistSimulatorStudy::get_parameter() const
    {
        return m_parameter;
    }

    void GuiExtensionNetlistSimulatorStudy::set_parameter(const std::vector<PluginParameter>& params)
    {
        m_parameter    = params;
        GuiApi* guiAPI = new GuiApi();

        std::vector<const Net*> probes;

        bool simulate            = false;
        bool net_picker_checkbox = false;
        std::filesystem::path sim_input;

        bool valid_inputs = true;

        // read settings file and values
        ProjectManager* pm = ProjectManager::instance();
        std::filesystem::path project_dir_path(pm->get_project_directory().string());

        if (!std::filesystem::exists(project_dir_path / "original/settings.ini"))
        {
            log_error("netlist_simulator_study", "Settings file is missing!");
            return;
        }

        QSettings my_settings(QString::fromStdString(project_dir_path / "original/settings.ini"), QSettings::IniFormat);

        if (!my_settings.contains("section1/t_probe"))
        {
            log_error("netlist_simulator_study", "Settings file is missing t_probe value!");
            return;
        }
        bool PROBE_TYPE = my_settings.value("section1/t_probe").toBool();

        if (!my_settings.contains("section1/max_probes"))
        {
            log_error("netlist_simulator_study", "Settings file is missing max_probes value!");
            return;
        }
        int MAX_PROBES = my_settings.value("section1/max_probes").toInt();

        for (PluginParameter par : m_parameter)
        {
            if (par.get_tagname() == "net_picker" && par.get_value() == "clicked")
            {
                net_picker_checkbox = true;
            }
            else if (par.get_tagname() == "simulate" && par.get_value() == "clicked")
                simulate = true;
            else if (par.get_tagname() == "sim_input")
                sim_input = par.get_value();
        }

        if (sim_input == "")
        {
            return;
        }

        std::vector<Net*> nets = guiAPI->getSelectedNets();

        // select only the allowed nets from the selected nets
        if (PROBE_TYPE)
        {
            // t probe model (n arbitrary probes)
            for (Net* net : nets)
            {
                if (net->is_global_input_net() || net->is_global_output_net())
                {
                    continue;
                }
                probes.push_back(net);
            }
        }
        else
        {
            // scan chain model (n probes at FF output)
            for (Net* net : nets)
            {
                if (net->is_global_input_net() || net->is_global_output_net())
                {
                    continue;
                }

                std::vector<hal::Endpoint*> sources = net->get_sources();
                bool dff_as_source                  = false;

                for (Endpoint* source : sources)
                {
                    if (source->get_gate()->get_type()->get_name().find("DFF") != std::string::npos)
                    {
                        dff_as_source = true;
                        break;
                    }
                }

                if (!dff_as_source)
                {
                    continue;
                }

                probes.push_back(net);
            }
        }

        std::vector<const Net*> probes_final_selection;

        // verify that only MAX_PROBES are selected or select a random subset if checkbox is checked
        if (probes.size() > MAX_PROBES)
        {
            if (!net_picker_checkbox)
            {
                log_error("netlist_simulator_study", "You selected more than " + std::to_string(MAX_PROBES) + " probes");
                return;
            }
            else
            {
                std::vector<int> indices(probes.size());

                std::iota(indices.begin(), indices.end(), 0);

                // Shuffle the indices
                std::random_device rd;
                std::mt19937 gen(rd());
                std::shuffle(indices.begin(), indices.end(), gen);

                // Pick the first subsetSize elements from the shuffled indices
                std::vector<int> subset;
                for (int i = 0; i < MAX_PROBES; ++i)
                {
                    probes_final_selection.push_back(probes[indices[i]]);
                }
            }
        }
        else
        {
            for (const Net* net : probes)
            {
                probes_final_selection.push_back(net);
            }
        }

        // add all input and output nets to the selection so the results are kept in the verilator output
        for (Net* net : gNetlist->get_nets())
        {
            if (net->is_global_input_net() || net->is_global_output_net())
            {
                probes_final_selection.push_back(net);
            }
        }

        if (simulate && m_parent && valid_inputs)
        {
            m_parent->simulate(sim_input, probes_final_selection);
        }
    }

}    // namespace hal