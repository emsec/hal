#include "netlist_modifier/netlist_modifier.h"

#include "gui/gui_api/gui_api.h"

namespace hal
{
    bool NetlistModifierPlugin::replace_gate_in_netlist(Netlist* netlist, Gate* gate)
    {
        // get the number of input pins
        int in_pins = gate->get_type()->get_pins([](const GatePin* gp) { return gp->get_direction() == PinDirection::input; }).size();
        int out_pins = gate->get_type()->get_pins([](const GatePin* gp) { return gp->get_direction() == PinDirection::output; }).size();
        int in_out_pins = gate->get_type()->get_pins([](const GatePin* gp) { return gp->get_direction() == PinDirection::inout; }).size();

        GateType* new_gate_type = netlist->get_gate_library()->get_gate_type_by_name(obfuscated_gate_name(in_pins,out_pins, in_out_pins));

        if (!new_gate_type)
        {
            log_error("netlist_modifier", "No gatetype called '{}' in gatelib", obfuscated_gate_name(in_pins,out_pins, in_out_pins));
            return false;
        }

        std::string gate_name = "UNKNOWN_" + std::to_string(gate->get_id());
        u32 gate_id           = gate->get_id();

        std::vector<u32> viewIDs;
        
        for(u32 viewID: GuiApiClasses::View::getIds({}, {gate})){
            viewIDs.push_back(viewID);
            // grid_positions.emplace_back(viewID, GuiApiClasses::View::getGridPlacement(viewID)->gatePosition(gate->get_id())->first, GuiApiClasses::View::getGridPlacement(viewID)->gatePosition(gate->get_id())->second);
        }

        Module* module = gate->get_module();

        std::vector<Net*> in_nets;
        std::vector<Net*> out_nets;

        // save the input and output nets
        for (Net* net : gate->get_fan_in_nets())
        {
            in_nets.push_back(net);
        }
        for (Net* net : gate->get_fan_out_nets())
        {
            out_nets.push_back(net);
        }

        // delete old gate and add new one
        netlist->delete_gate(gate);
        Gate* new_gate = netlist->create_gate(gate_id, new_gate_type, gate_name);

        // add old connections
        int counter = 0;
        for (Net* net : in_nets)
        {
            net->add_destination(new_gate, "I" + std::to_string(counter));
            counter++;
        }

        counter = 0;
        for (Net* net : out_nets)
        {
            net->add_source(new_gate, "O" + std::to_string(counter));
            counter++;
        }

        /*for(std::tuple<u32, int, int> placement_item: grid_positions){
            u32 viewID = std::get<0>(placement_item);
            int x = std::get<1>(placement_item);
            int y = std::get<2>(placement_item);

            std::cout << x << ":" << y << std::endl;

            GridPlacement* gp = GuiApiClasses::View::getGridPlacement(viewID);

            gp->setGatePosition(new_gate->get_id(), {x, y});

            GuiApiClasses::View::setGridPlacement(viewID, gp);

            int new_x = GuiApiClasses::View::getGridPlacement(viewID)->gatePosition(new_gate->get_id())->first;
            int new_y = GuiApiClasses::View::getGridPlacement(viewID)->gatePosition(new_gate->get_id())->second;

            std::cout << new_x << ":" << new_y << std::endl;
        }*/

        if (module != new_gate->get_module())
            module->assign_gate(new_gate);

        
        for(u32 viewID: viewIDs){
            continue;
            // GridPlacement* gp = GuiApiClasses::View::getGridPlacement(viewID);
            GuiApiClasses::View::addTo(viewID, {}, {new_gate});
            // GuiApiClasses::View::setGridPlacement(viewID, gp);
        }

        return true;
    }
}