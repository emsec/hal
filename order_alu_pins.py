mod_id = 10
output_net_ids = [34, 35, 36, 37, 38, 39, 40, 41]
input_a_net_ids = [65, 66, 67, 68, 69, 70, 71, 72]
input_b_net_ids = [57, 58, 59, 60, 61, 62, 63, 64]

def reorder_pins_by_nets(net_ids):
    mod = netlist.get_module_by_id(mod_id)
    if mod == None:
        return False

    first_net = netlist.get_net_by_id(net_ids[0])
    if first_net == None:
        return False
        
    first_pin = mod.get_pin_by_net(first_net)
    if first_pin == None:
        return False
        
    grp = first_pin.get_group()[0]
    if grp == None:
        return False
    
    ctr = 0
    for nid in net_ids:
        net = netlist.get_net_by_id(nid)
        if net == None:
            return False
                
        mod.move_pin_within_group(grp, mod.get_pin_by_net(net), ctr)
        ctr += 1
        
reorder_pins_by_nets(output_net_ids)
reorder_pins_by_nets(input_a_net_ids)
reorder_pins_by_nets(input_b_net_ids)