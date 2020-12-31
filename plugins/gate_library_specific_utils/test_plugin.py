#!/usr/bin/env python3
import sys, os

#some necessary configuration:
sys.path.append("/home/simon/projects/hal/build/lib/") #this is where your hal python lib is located
os.environ["HAL_BASE_PATH"] = "/home/simon/projects/hal/build" # hal base path
import hal_py

netlist_to_read = "/home/simon/projects/maggie/maggie.v"
gate_library_path = "/home/simon/projects/hal/plugins/gate_libraries/definitions/ice40ultra.lib"

#initialize HAL
hal_py.plugin_manager.load_all_plugins()

#read netlist
netlist = hal_py.NetlistFactory.load_netlist(netlist_to_read, gate_library_path)

from hal_plugins import solve_fsm

pl = hal_py.plugin_manager.get_plugin_instance("solve_fsm")

transition = [1869, 4061, 1868, 1875, 1872, 2178, 4420, 2177, 2967, 4065, 1963, 3968, 1867, 1882, 4457, 1871]
state = [1578, 1686, 1539, 1538, 1552, 1543, 1533, 1541, 1540, 1532, 1657, 1579]

graph = pl.solve_fsm(netlist, state, transition, {})
print(graph)

#unload everything hal related
hal_py.plugin_manager.unload_all_plugins()