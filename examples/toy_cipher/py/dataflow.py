from hal_plugins import dataflow
pl = hal_py.plugin_manager.get_plugin_instance("dataflow")

dana_registers = pl.execute(netlist, "/tmp/dataflow_out/", [16], False, True, False, [], 3)
