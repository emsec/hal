#include "netlist_modifier/netlist_modifier.h"

#include "boost/functional/hash.hpp"
#include "gui/gui_api/gui_api.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/netlist/netlist_writer/netlist_writer_manager.h"
#include "hal_core/netlist/project_manager.h"

#include <deque>
#include <filesystem>
#include <iostream>

namespace hal
{

    extern Netlist* gNetlist;

    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<NetlistModifierPlugin>();
    }

    NetlistModifierPlugin::NetlistModifierPlugin()
    {
        m_gui_extension = nullptr;
    }

    std::string NetlistModifierPlugin::get_name() const
    {
        return std::string("netlist-modifier");
    }

    std::string NetlistModifierPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void NetlistModifierPlugin::on_load()
    {
        m_gui_extension           = new GuiExtensionNetlistModifier;
        m_gui_extension->m_parent = this;
        m_extensions.push_back(m_gui_extension);
    }

    void NetlistModifierPlugin::on_unload()
    {
        delete_extension(m_gui_extension);
    }

    std::set<std::string> NetlistModifierPlugin::get_dependencies() const
    {
        std::set<std::string> retval;
        retval.insert("hal_gui");
        retval.insert("verilog_writer");
        return retval;
    }

    void NetlistModifierPlugin::initialize()
    {
    }

    bool replace_gate_in_netlist(Netlist* netlist, Gate* gate)
    {
        // get the number of input pins
        int num_of_in  = gate->get_fan_in_endpoints().size();
        int num_of_out = gate->get_fan_out_endpoints().size();

        GateType* new_gate_type = netlist->get_gate_library()->get_gate_type_by_name("UNKNOWN_" + std::to_string(num_of_in) + "IN_" + std::to_string(num_of_out) + "OUT");

        if (!new_gate_type)
        {
            log_error("netlist_modifier", "No gatetype called 'UNKNOWN_" + std::to_string(num_of_in) + "IN_" + std::to_string(num_of_out) + "OUT' in gatelib");
            return false;
        }

        /*
        // dynamicly creating missing gatetype does not work as returned gatelib by netlist is const and adding type does not work
        if (!new_gate_type){
            // gate type does not exist in gatelib
            new_gate_type = netlist->get_gate_library()->create_gate_type("UNKNOWN_"+std::to_string(num_of_in)+"IN_"+std::to_string(num_of_out)+"OUT");

            for (int i = 0; i < num_of_in; i++)
            {
                new_gate_type->create_pin("IN_"+std::to_string(i), PinDirection::input);
            }

            for (int i = 0; i < num_of_out; i++)
            {
                new_gate_type->create_pin("OUT_"+std::to_string(i), PinDirection::output);
            }
        }*/

        std::string gate_name = "UNKNOWN_" + std::to_string(gate->get_id());
        u32 gate_id           = gate->get_id();

        std::vector<Net*> in_nets;
        std::vector<Net*> out_nets;

        // save the input and output nets
        for (Endpoint* ep : gate->get_fan_in_endpoints())
        {
            in_nets.push_back(ep->get_net());
        }
        for (Endpoint* ep : gate->get_fan_out_endpoints())
        {
            out_nets.push_back(ep->get_net());
        }

        // delete old gate and add new one
        netlist->delete_gate(gate);
        Gate* new_gate = netlist->create_gate(gate_id, new_gate_type, gate_name);

        // add old connections
        int counter = 1;
        for (Net* net : in_nets)
        {
            net->add_destination(new_gate, "IN_" + std::to_string(counter));
            counter++;
        }

        counter = 1;
        for (Net* net : out_nets)
        {
            net->add_source(new_gate, "OUT_" + std::to_string(counter));
            counter++;
        }

        return true;
    }

    bool NetlistModifierPlugin::modify_in_place()
    {
        GuiApi* guiAPI = new GuiApi();

        /*
        // loading copy does not work as one can not add a type to the second gatelib and adding a gate of this type to the netlist if the gatelib of the gui does not have this type

        ProjectManager* pm = ProjectManager::instance();

        std::filesystem::path project_dir_path(pm->get_project_directory().string());

        if (!std::filesystem::exists(project_dir_path/"generated/gatelib_obfuscated.hgl"))
        {
            if (std::filesystem::exists(project_dir_path/"generated") && !std::filesystem::is_directory(project_dir_path/"generated")){
                log_error("netlist_modifier", "A file called 'generated' in the project directory exists but it is expected to be a directory!");
                return false;
            }
            std::filesystem::create_directories(project_dir_path/"generated");

            std::filesystem::copy(gNetlist->get_gate_library()->get_path(), project_dir_path/"generated/gatelib_obfuscated.hgl");
        }

        
        GateLibrary* lib = gate_library_manager::load(project_dir_path/"generated/gatelib_obfuscated.hgl");*/

        std::vector<Gate*> gates = guiAPI->getSelectedGates();

        for (Gate* gate : gates)
        {
            if (!replace_gate_in_netlist(gNetlist, gate))
            {
                return false;
            }
        }

        return true;
    }

    bool NetlistModifierPlugin::save()
    {
        ProjectManager* pm = ProjectManager::instance();

        std::filesystem::path project_dir_path(pm->get_project_directory().string());

        if (std::filesystem::exists(project_dir_path / "generated") && !std::filesystem::is_directory(project_dir_path / "generated"))
        {
            log_error("netlist_modifier", "A file called 'generated' in the project directory exists but it is expected to be a directory!");
            return false;
        }

        if (!std::filesystem::exists(project_dir_path / "generated"))
        {
            std::filesystem::create_directories(project_dir_path / "generated");
        }

        netlist_writer_manager::write(gNetlist, project_dir_path / "generated/generated_netlist_obfuscated.v");

        std::filesystem::copy(gNetlist->get_gate_library()->get_path(), project_dir_path / "generated/generated_gatelib_obfuscated.hgl");

        return true;
    }

    GuiExtensionNetlistModifier::GuiExtensionNetlistModifier()
    {
        m_parameter.push_back(PluginParameter(PluginParameter::PushButton, "modify_in_place", "Modify selected gates in place"));
        m_parameter.push_back(PluginParameter(PluginParameter::PushButton, "save", "Save"));
    }

    std::vector<PluginParameter> GuiExtensionNetlistModifier::get_parameter() const
    {
        return m_parameter;
    }

    void GuiExtensionNetlistModifier::set_parameter(const std::vector<PluginParameter>& params)
    {
        m_parameter = params;

        bool modify_in_place = false;
        bool save            = false;

        for (PluginParameter par : m_parameter)
        {
            if (par.get_tagname() == "modify_in_place" && par.get_value() == "clicked")
            {
                modify_in_place = true;
            }
            else if (par.get_tagname() == "save" && par.get_value() == "clicked")
            {
                save = true;
            }
        }
        if (modify_in_place && m_parent)
        {
            m_parent->modify_in_place();
        }
        else if (save && m_parent)
        {
            m_parent->save();
        }
    }
}    // namespace hal