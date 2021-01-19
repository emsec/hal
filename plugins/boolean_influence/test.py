from hal_plugins import boolean_influence

pl = hal_py.plugin_manager.get_plugin_instance("boolean_influence")

g = netlist.get_gate_by_id(1547)
inf = pl.get_boolean_influences_of_gate(g)

print(inf)