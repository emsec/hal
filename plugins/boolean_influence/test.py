from hal_plugins import boolean_influence

pl = hal_py.plugin_manager.get_plugin_instance("boolean_influence")

g = netlist.get_gate_by_id(4)
inf = pl.get_boolean_influences_of_gate(g)

#n = netlist.get_net_by_id(11)
#inf = pl.get_boolean_influences_of_subcircuit(netlist.get_gates(), n)

for net in inf:
    print(net.name, inf[net])
    
import time
print("Finished at {}".format(time.time()))