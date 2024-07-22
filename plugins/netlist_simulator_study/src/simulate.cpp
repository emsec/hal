#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/netlist/project_manager.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "netlist_simulator_controller/plugin_netlist_simulator_controller.h"
#include "netlist_simulator_study/netlist_simulator_study.h"

namespace hal
{
    bool NetlistSimulatorStudyPlugin::simulate(std::filesystem::path sim_input, std::vector<const Net*> probes)
    {
        if (!std::filesystem::exists(sim_input))
        {
            log_error("netlist_simulator_study", "No valid input file!");
            return false;
        }

        // read the original netlist
        ProjectManager* pm = ProjectManager::instance();
        std::filesystem::path project_dir_path(pm->get_project_directory().string());

        std::string salt = read_named_zip_file_decrypted(project_dir_path / "original/original.zip", SECRET_PASSWORD, "salt.encrypt");

        std::string netlist_data = read_named_zip_file_decrypted(project_dir_path / "original/original.zip", gen_salted_password(SECRET_PASSWORD, salt), "original.hal");

        if (netlist_data == "")
        {
            log_error("netlist_simulator_study", "No valid netlist file in zip!");
            return false;
        }

        m_original_netlist = netlist_factory::load_netlist_from_string(netlist_data, gNetlist->get_gate_library()->get_path());

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

        int simulProgress = 0;
        if (GuiExtensionNetlistSimulatorStudy::s_progress_indicator_function)
            GuiExtensionNetlistSimulatorStudy::s_progress_indicator_function(simulProgress++, "Wait for simulation to finish...");

        m_simul_controller.get()->simulate_only_probes(probes);

        m_simul_controller.get()->run_simulation();

        if (GuiExtensionNetlistSimulatorStudy::s_progress_indicator_function)
            GuiExtensionNetlistSimulatorStudy::s_progress_indicator_function(simulProgress++, "Wait for simulation to finish...");

        while (m_simul_controller.get()->get_simulation_engine()->get_state() > 0)
        {
            if (GuiExtensionNetlistSimulatorStudy::s_progress_indicator_function)
                GuiExtensionNetlistSimulatorStudy::s_progress_indicator_function(simulProgress, "Wait for simulation to finish...");
            if (simulProgress % 10 == 0)
                log_info("netlist_simulator_study", "Waiting for simulation to finish...");
            if (++simulProgress >= 100)
                simulProgress = 0;
        }

        if (GuiExtensionNetlistSimulatorStudy::s_progress_indicator_function)
            GuiExtensionNetlistSimulatorStudy::s_progress_indicator_function(100, "Simulation process ended.");

        if (m_simul_controller.get()->get_simulation_engine()->get_state() == SimulationEngine::Done)
        {
            log_info("netlist_simulator_study", "Simulation successful.");
            m_simul_controller.get()->get_results();

            for (const Net* n : probes)
                m_simul_controller.get()->get_waveform_by_net(n);
        }
        else
        {
            log_info("netlist_simulator_study", "Simulation failed, please check engine log in directory '{}'.", m_simul_controller.get()->get_working_directory());
            return false;
        }
        return true;
    }
}    // namespace hal