import sys, os

hal_base_path = "/home/simon/projects/hal/"
netlist_path = "/home/simon/data/iphone/simulation_2022_05_13/maggie_netlist_28_04_2022_with_manual_DSP_views_added_fsms.hal"
#gate_library_path = "/home/simon/projects/hal_4_0/hal/plugins/gate_libraries/definitions/lsi_10k.hgl"

sys.path.append(hal_base_path + "build/lib/") #this is where your hal python lib is located
os.environ["HAL_BASE_PATH"] = hal_base_path + "build" # hal base path

import hal_py

#initialize HAL
hal_py.plugin_manager.load_all_plugins()

netlist = hal_py.NetlistFactory.load_netlist(netlist_path)

from hal_plugins import iphone_tools

iphone_tools.IPhoneTools.reverse_dsps(netlist)


#unload everything hal related
hal_py.plugin_manager.unload_all_plugins()
