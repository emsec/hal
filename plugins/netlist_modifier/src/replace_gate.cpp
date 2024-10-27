#include "gui/gui_api/gui_api.h"
#include "netlist_modifier/netlist_modifier.h"

namespace hal
{
    /**
     * @brief This function replaces a given gate with a gate of an unknown type with the same input and output pin size
     * 
     * @param netlist A pointer to the current netlist
     * @param gate The gate that should be replaced
     * @return bool true on success
     */
    bool NetlistModifierPlugin::replace_gate_in_netlist(Netlist* netlist, Gate* gate)
    {
        // get the number of input pins
        int in_pins     = gate->get_type()->get_pins([](const GatePin* gp) { return gp->get_direction() == PinDirection::input; }).size();
        int out_pins    = gate->get_type()->get_pins([](const GatePin* gp) { return gp->get_direction() == PinDirection::output; }).size();
        int in_out_pins = gate->get_type()->get_pins([](const GatePin* gp) { return gp->get_direction() == PinDirection::inout; }).size();

        // get the obfuscated gate type from the gate lib
        GateType* new_gate_type = netlist->get_gate_library()->get_gate_type_by_name(obfuscated_gate_name(in_pins, out_pins, in_out_pins));

        if (!new_gate_type)
        {
            log_error("netlist_modifier", "No gatetype called '{}' in gatelib", obfuscated_gate_name(in_pins, out_pins, in_out_pins));
            return false;
        }

        std::string gate_name = "UNKNOWN_" + std::to_string(gate->get_id());
        u32 gate_id           = gate->get_id();

        // save the grid positions for all the views where this gate is included
        std::vector<std::tuple<u32, int, int>> grid_positions;

        for (u32 viewID : GuiApiClasses::View::getIds({}, {gate}))
        {
            GridPlacement* gp = GuiApiClasses::View::getGridPlacement(viewID);

            int x = gp->gatePosition(gate->get_id())->first;
            int y = gp->gatePosition(gate->get_id())->second;
            grid_positions.emplace_back(viewID, x, y);
        }

        // remember the module assignment
        Module* module = gate->get_module();

        std::vector<Net*> in_nets;
        std::vector<Net*> out_nets;

        // save the input and output nets
        for (GatePin* pin: gate->get_type()->get_input_pins())
        {
            if(gate->get_fan_in_endpoint(pin->get_name())){
                in_nets.push_back(gate->get_fan_in_endpoint(pin->get_name())->get_net());
            }else{
                in_nets.push_back(NULL);
            }
        }

        for (GatePin* pin: gate->get_type()->get_output_pins())
        {
            if(gate->get_fan_out_endpoint(pin->get_name())){
                out_nets.push_back(gate->get_fan_out_endpoint(pin->get_name())->get_net());
            }else{
                out_nets.push_back(NULL);
            }
        }

        // delete old gate and add new one
        netlist->delete_gate(gate);
        Gate* new_gate = netlist->create_gate(gate_id, new_gate_type, gate_name);

        // add old connections
        int counter = 0;
        for (Net* net : in_nets)
        {
            if(net){
                net->add_destination(new_gate, "I" + std::to_string(counter));
            }
            counter++;
        }

        counter = 0;
        for (Net* net : out_nets)
        {
            if(net){
                net->add_source(new_gate, "O" + std::to_string(counter));
            }
            counter++;
        }

        // assign the new gate to the same module
        if (module != new_gate->get_module())
            module->assign_gate(new_gate);

        // update all the grid positions in all the views
        for (std::tuple<u32, int, int> placement_item : grid_positions)
        {
            u32 viewID = std::get<0>(placement_item);
            int x      = std::get<1>(placement_item);
            int y      = std::get<2>(placement_item);

            GuiApiClasses::View::addTo(viewID, {}, {new_gate});

            GridPlacement* gp = GuiApiClasses::View::getGridPlacement(viewID);

            gp->setGatePosition(new_gate->get_id(), {x, y});

            GuiApiClasses::View::setGridPlacement(viewID, gp);
        }

        return true;
    }
}    // namespace hal