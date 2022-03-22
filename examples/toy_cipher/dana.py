from hal_plugins import dataflow
pl = hal_py.plugin_manager.get_plugin_instance("dataflow")

# params: netlist, folder to write to, prefered sizes, dataflow graph generation = true/false
# TODO check params in master, I am currently on the dev branch and we adjusted DANA a bit...
# only generate dataflow graphs for small netlists
dana_registers = pl.execute(netlist, "/tmp/dataflow_out/", [16], False, set(), 3)

for module in netlist.get_modules():
    netlist.delete_module(module)

counter = 0
for dana_register in dana_registers:
    register_module = netlist.create_module("register_" + str(counter), netlist.get_top_module())
    counter += 1
    for ff in dana_register:
        register_module.assign_gate(ff)