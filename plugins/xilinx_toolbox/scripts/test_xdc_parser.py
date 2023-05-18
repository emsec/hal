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

nl_path = "/home/simon/projects/hal/plugins/xilinx_toolbox/scripts/netlist.v"
gl_path = "/home/simon/projects/hal/plugins/gate_libraries/definitions/XILINX_UNISIM.hgl"

xdc_path = "/home/simon/projects/hwio/soc_with_aes_uart/vivado_project/test_location.xdc"

netlist = hal_py.NetlistFactory.load_netlist(nl_path, gl_path)

print(len(netlist.gates))

from hal_plugins import xilinx_toolbox

xilinx_toolbox.XilinxToolboxPlugin.parse_xdc_file(netlist, xdc_path)

#unload everything hal related
hal_py.plugin_manager.unload_all_plugins()