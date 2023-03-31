from hal_plugins import boolean_influence

pl = hal_py.plugin_manager.get_plugin_instance("boolean_influence")

#g = netlist.get_gate_by_id(4)
#inf = pl.get_boolean_influences_of_gate(g)

<<<<<<< HEAD
n = netlist.get_net_by_id(67)
inf = pl.get_boolean_influences_of_subcircuit(netlist.get_gates(), n, 32000)
=======
n = netlist.get_net_by_id(98)
inf = pl.get_boolean_influences_of_subcircuit(netlist.get_gates(), n, 64000)
>>>>>>> master

print("Probabilistic: ")
for net in inf:
    print(net.name, inf[net])

<<<<<<< HEAD
#n = netlist.get_net_by_id(58)
#inf = pl.get_boolean_influences_of_subcircuit_deterministic(netlist.get_gates(), n)

#print("Deterministic: ")
#for net in inf:
#    print(net.name, inf[net])
=======
n = netlist.get_net_by_id(74)
inf = pl.get_boolean_influences_of_subcircuit_deterministic(netlist.get_gates(), n)

print("Deterministic: ")
for net in inf:
    print(net.name, inf[net])
>>>>>>> master
    