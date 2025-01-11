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

nl_path = "/home/simon/tools/OpenLane/designs/mem_1r1w/runs/full_guide/results/final/verilog/gl/mem_1r1w_nl.v"
gl_path = "/home/simon/projects/hal/plugins/gate_libraries/definitions/sky130_fd_sc_hd__ff_100C_1v65.hgl"

def_path = "/home/simon/tools/OpenLane/designs/mem_1r1w/runs/full_guide/results/final/def/mem_1r1w.def"

netlist = hal_py.NetlistFactory.load_netlist(nl_path, gl_path)

print(len(netlist.gates))

from hal_plugins import netlist_preprocessing

netlist_preprocessing.NetlistPreprocessingPlugin.parse_def_file(netlist, def_path)

#unload everything hal related
hal_py.plugin_manager.unload_all_plugins()