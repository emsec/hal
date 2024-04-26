#include "netlist_modifier/netlist_modifier.h"

#include "boost/functional/hash.hpp"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/netlist_writer/netlist_writer_manager.h"

#include <deque>

#include "hal_core/netlist/netlist_factory.h"

namespace hal
{

    extern Netlist* gNetlist;

    Netlist* NetlistModifierPlugin::modified_netlist_pointer = nullptr;

    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<NetlistModifierPlugin>();
    }

    NetlistModifierPlugin::NetlistModifierPlugin(){
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
        m_gui_extension = new GuiExtensionNetlistModifier;
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

    bool NetlistModifierPlugin::replace_gates(){
        auto gates = NetlistModifierPlugin::modified_netlist_pointer->get_gates();

        for(Gate* gate: gates){
            // std::cout << gate->get_type()->to_string() << std::endl;

            if(gate->get_type()->to_string() == "AND"){
                GateType* new_gate_type = NetlistModifierPlugin::modified_netlist_pointer->get_gate_library()->get_gate_type_by_name("UNKNOWN_2IN");
                std::string gate_name = gate->get_name();
                u32 gate_id = gate->get_id();

                std::map<std::string, Net*> predecessors;
                std::map<std::string, Net*> successors;

                // gate->get_successor() results in errors????
                for(Endpoint* endpoint: gate->get_fan_in_endpoints()){
                    predecessors[endpoint->get_pin()->get_name()] = endpoint->get_net();
                }

                for(Endpoint* endpoint: gate->get_fan_out_endpoints()){
                    successors[endpoint->get_pin()->get_name()] = endpoint->get_net();
                }

                NetlistModifierPlugin::modified_netlist_pointer->delete_gate(gate);

                Gate* new_gate = NetlistModifierPlugin::modified_netlist_pointer->create_gate(gate_id, new_gate_type, gate_name);

                for(auto entry: predecessors){
                    entry.second->add_destination(new_gate, entry.first);
                }

                for(auto entry: successors){
                    entry.second->add_source(new_gate, entry.first);
                }
            }
        }

        return true;
    }

    bool NetlistModifierPlugin::create_modified_netlist(){
        std::cout << gNetlist->get_gates().size() << std::endl;

        auto tmp = gNetlist->copy();
        NetlistModifierPlugin::modified_netlist_pointer = std::move(tmp.get().get());


        replace_gates();

        replace_gates();

        // no writer for .v ????
        netlist_writer_manager::write(modified_netlist_pointer, "/home/ole/Documents/MPI/hal_project_for_testing/modified_netlist_pointer.v");

        // crashes when executed
        // gNetlist = (NetlistModifierPlugin::modified_netlist_pointer->copy()).get().get();

        // std::cout << gNetlist->get_gates().size() << std::endl;

        return true;
    }













    GuiExtensionNetlistModifier::GuiExtensionNetlistModifier()
         {
             m_parameter.push_back(PluginParameter(PluginParameter::PushButton,"modify","Modify and save"));
         }

         std::vector<PluginParameter> GuiExtensionNetlistModifier::get_parameter() const
         {
             return m_parameter;
         }

         void GuiExtensionNetlistModifier::set_parameter(const std::vector<PluginParameter>& params)
         {
             m_parameter = params;
             bool modify_save = false;
             for (PluginParameter par : m_parameter)
             {
                 if(par.get_tagname()=="modify" && par.get_value() == "clicked")
                     modify_save = true;
             }
             if (modify_save && m_parent )
             {
                m_parent->create_modified_netlist();
             }
         }


} // namespace hal