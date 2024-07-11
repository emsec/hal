from hal_plugins import module_identification

for mod in netlist.get_modules():
    netlist.delete_module(mod)

pl = hal_py.plugin_manager.get_plugin_instance("module_identification")

pl.run_plugin(netlist)
