
for m in netlist.get_modules():
    for net in m.get_output_nets():
        pin = m.get_pin_by_net(net)
        pg = pin.get_group()[0]
        
        index = net.name.split('(', 1)[1].split(')')[0]
        
        print(net.name, index)
        
        m.set_pin_group_name(pg, "O({})".format(index))
        m.set_pin_name(pin, "O({})".format(index))