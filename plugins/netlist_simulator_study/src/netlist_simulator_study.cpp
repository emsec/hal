#include "netlist_simulator_study/netlist_simulator_study.h"
#include "hal_core/netlist/netlist_parser/netlist_parser_manager.h"
#include "netlist_simulator_controller/plugin_netlist_simulator_controller.h"
#include "netlist_simulator_controller/netlist_simulator_controller.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/netlist/netlist_factory.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<NetlistSimulatorStudyPlugin>();
    }

    NetlistSimulatorStudyPlugin::NetlistSimulatorStudyPlugin(){
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
        m_original_netlist = netlist_factory::load_netlist("/home/ole/Documents/MPI/hal_project_for_testing/simple.v", "/home/ole/Documents/MPI/hal_project_for_testing/unknown_design.lib");
        std::cout << m_original_netlist.get() << std::endl;


        m_gui_extension = new GuiExtensionNetlistSimulatorStudy;
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

     bool NetlistSimulatorStudyPlugin::simulate(std::filesystem::path sim_input)
     {
         enum {VCD, CSV, SAL} input_file_format;
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
             if (!m_simul_controller.get()->import_vcd(sim_input.string(),NetlistSimulatorController::FilterInputFlag::CompleteNetlist))
             {
                 log_warning("netlist_simulator_study", "Cannot parse simulation input file '{}'.", sim_input.string());
                 return false;
             }
             break;
         case CSV:
             m_simul_controller.get()->import_csv(sim_input.string(),NetlistSimulatorController::FilterInputFlag::CompleteNetlist);
             break;
         case SAL:
             m_simul_controller.get()->import_simulation(sim_input.parent_path().string(),NetlistSimulatorController::FilterInputFlag::CompleteNetlist);
             break;
         }

         m_simul_controller.get()->emit_run_simulation();
     }

     //----------------------

         GuiExtensionNetlistSimulatorStudy::GuiExtensionNetlistSimulatorStudy()
         {
             m_parameter.push_back(PluginParameter(PluginParameter::ExistingFile,"sim_input","Simulation input file"));
             m_parameter.push_back(PluginParameter(PluginParameter::PushButton,"simulate","Start Simulation"));
         }

         std::vector<PluginParameter> GuiExtensionNetlistSimulatorStudy::get_parameter() const
         {
             return m_parameter;
         }

         void GuiExtensionNetlistSimulatorStudy::set_parameter(const std::vector<PluginParameter>& params)
         {
             m_parameter = params;
             bool simulate = false;
             std::filesystem::path sim_input;
             for (PluginParameter par : m_parameter)
             {
                 if(par.get_tagname()=="simulate" && par.get_value() == "clicked")
                     simulate = true;
                 else if (par.get_tagname()=="sim_input")
                     sim_input = par.get_value();
             }
             if (simulate && m_parent )
             {
                m_parent->simulate(sim_input);
             }
         }


}    // namespace hal