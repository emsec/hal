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

    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<NetlistModifierPlugin>();
    }

    std::string NetlistModifierPlugin::get_name() const
    {
        return std::string("netlist-modifier");
    }

    std::string NetlistModifierPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void NetlistModifierPlugin::initialize()
    {
    }

    bool NetlistModifierPlugin::replace_gates(){
        auto gates = modified_netlist_pointer->get_gates();

        for(Gate* gate: gates){
            // std::cout << gate->get_type()->to_string() << std::endl;

            if(gate->get_type()->to_string() == "AND"){
                GateType* new_gate_type = modified_netlist_pointer->get_gate_library()->get_gate_type_by_name("OR");
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

                modified_netlist_pointer->delete_gate(gate);

                Gate* new_gate = modified_netlist_pointer->create_gate(gate_id, new_gate_type, gate_name);

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
        auto tmp = gNetlist->copy();
        modified_netlist_pointer = tmp.get().get();


        replace_gates();

        replace_gates();

        // no writer for .v ????
        // netlist_writer_manager::write(modified_netlist_pointer, "~/Documents/MPI/hal_project_for_testing/modified_netlist_pointer.v");

        // gNetlist = modified_netlist_pointer;

        return true;
    }

} // namespace hal