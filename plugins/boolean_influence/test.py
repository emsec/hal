from hal_plugins import boolean_influence

pl = hal_py.plugin_manager.get_plugin_instance("boolean_influence")

# g = netlist.get_gate_by_id(4)
# inf = pl.get_boolean_influences_of_gate(g)

n = netlist.get_net_by_id(3849)
inf = pl.get_boolean_influences_of_subcircuit(netlist.get_module_by_id(35).get_gates(), n, 32000)

print("Probabilistic: ")
influences_list = []
for net in inf:
    #print(net.name, inf[net])
    influences_list.append((net, inf[net]))
    
influences_list = sorted(influences_list, key=lambda x: x[1])
for (net, inf) in influences_list:
    print(net.name, inf)

# n = netlist.get_net_by_id(58)
# inf = pl.get_boolean_influences_of_subcircuit_deterministic(netlist.get_gates(), n)

# print("Deterministic: ")
# for net in inf:
#    print(net.name, inf[net])
