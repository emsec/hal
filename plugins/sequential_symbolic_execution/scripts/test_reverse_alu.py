import sys, os

hal_base_path = "/home/simon/projects/hal/"
netlist_path = "/home/simon/workspaces/2022_06_08/mips_16_core_netlist"
#gate_library_path = "/home/simon/projects/hal_4_0/hal/plugins/gate_libraries/definitions/lsi_10k.hgl"

sys.path.append(hal_base_path + "build/lib/") #this is where your hal python lib is located
os.environ["HAL_BASE_PATH"] = hal_base_path + "build" # hal base path

import hal_py

#initialize HAL
hal_py.plugin_manager.load_all_plugins()

netlist = hal_py.NetlistFactory.load_hal_project(netlist_path)

from hal_plugins import iphone_tools

iphone_tools.IPhoneTools.reverse_alu(netlist, 6)

#unload everything hal related
hal_py.plugin_manager.unload_all_plugins()


#time /home/simon/projects/hal/build/bin/hal -p /home/simon/workspaces/2022_06_08/mips_16_core_netlist --reverse_alu 1 > /home/simon/workspaces/2022_06_08/out.txt