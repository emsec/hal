from hal_plugins import boolean_influence

pl = hal_py.plugin_manager.get_plugin_instance("boolean_influence")

g = netlist.get_gate_by_id(66)
inf = pl.get_boolean_influences_of_gate(g)

for net in inf:
    print(net.name, inf[net])
    
import time
print("Finished at {}".format(time.time()))