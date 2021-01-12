#!/usr/bin/env python3
import sys, os

#some necessary configuration:
sys.path.append("/home/simon/projects/hal/build/lib/") #this is where your hal python lib is located
os.environ["HAL_BASE_PATH"] = "/home/simon/projects/hal/build" # hal base path
import hal_py

netlist_to_read = "netlist.v"
gate_library_path = "gate_library.lib"

#initialize HAL
hal_py.plugin_manager.load_all_plugins()

#read netlist
netlist = hal_py.NetlistFactory.load_netlist(netlist_to_read, gate_library_path)

from hal_plugins import solve_fsm

pl_fsm = hal_py.plugin_manager.get_plugin_instance("solve_fsm")

# UPDATE THE MODULE IDS OR CREATE YOUR OWN LIST OF GATES
state_mod = netlist.get_module_by_id(0)
transition_mod = netlist.get_module_by_id(0)

transition_gates = transition_mod.gates
state_gates = state_mod.gates

initial_state = {}
timeout = 600000

g = pl_fsm.solve_fsm(netlist, state_gates, transition_gates, initial_state, timeout)

#unload everything hal related
hal_py.plugin_manager.unload_all_plugins()