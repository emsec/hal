#!/usr/bin/env python3
import sys, os

user = os.getlogin()
print("Found user: {}".format(user)); 
if user == "simon":
    hal_base_path = "/home/simon/projects/hal/"
else:
    hal_base_path = "/home/anna/hal/"


#some necessary configuration:
sys.path.append(hal_base_path + "build/lib/") #this is where your hal python lib is located
os.environ["HAL_BASE_PATH"] = hal_base_path + "build/" # hal base path

import hal_py

netlist_to_read = hal_base_path + "plugins/bachelorarbeit/netlists/adder.v"
gate_library_path = hal_base_path + "plugins/gate_libraries/definitions/NangateOpenCellLibrary.hgl"

#initialize HAL
hal_py.plugin_manager.load_all_plugins()

#read netlist
netlist = hal_py.NetlistFactory.load_netlist(netlist_to_read, gate_library_path)
from hal_plugins import module_identification

pl = hal_py.plugin_manager.get_plugin_instance("module_identification")

pl.test_adder(netlist.get_top_module())

#unload everything hal related
hal_py.plugin_manager.unload_all_plugins()