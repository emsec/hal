#!/usr/bin/env python3
import sys, os

#some necessary configuration:
base_path = "/home/simon/projects/hal/"

sys.path.append(base_path + "build/lib/") #this is where your hal python lib is located
os.environ["HAL_BASE_PATH"] = base_path + "build" # hal base path
import hal_py

#initialize HAL
hal_py.plugin_manager.load_all_plugins()

from hal_plugins import bitwuzla_utils


netlist_path = "/home/simon/workspaces/2024_01_31/hal_project_ibex/ibex/ibex.hal"
gate_lib_path = "/home/simon/workspaces/2024_01_31/hal_project_ibex/ibex/XILINX_UNISIM_hal.hgl"

netlist = hal_py.NetlistFactory.load_netlist(netlist_path, gate_lib_path)

bitwuzla_utils.BitwuzlaUtilsPlugin.benchmark(netlist, netlist.get_nets())

#unload everything hal related
hal_py.plugin_manager.unload_all_plugins()



