#!/usr/bin/env python3
import sys, os

#some necessary configuration:
base_path = "/home/simon/projects/hal/"

sys.path.append(base_path + "build/lib/") #this is where your hal python lib is located
os.environ["HAL_BASE_PATH"] = base_path + "build" # hal base path
import hal_py

#initialize HAL
hal_py.plugin_manager.load_all_plugins()

#read netlist

netlist = hal_py.NetlistFactory.load_netlist(base_path + "examples/fsm/fsm.v", base_path + "examples/fsm/example_library.hgl")

from hal_plugins import solve_fsm

pl_fsm = hal_py.plugin_manager.get_plugin_instance("solve_fsm")

# UPDATE THE MODULE IDS OR CREATE YOUR OWN LIST OF GATES
fsm_mod = netlist.get_module_by_id(1)

transition_gates = fsm_mod.get_gates(lambda g : g.type.has_property(hal_py.GateTypeProperty.combinational))
state_gates = fsm_mod.get_gates(lambda g : g.type.has_property(hal_py.GateTypeProperty.sequential))

initial_state = {}
graph_path = base_path + "examples/fsm/graph.dot"
timeout = 600000

g = pl_fsm.solve_fsm(netlist, state_gates, transition_gates, initial_state, graph_path, timeout)

#unload everything hal related
hal_py.plugin_manager.unload_all_plugins()