#!/usr/bin/env python3
import sys, os

user = os.getlogin()
print("Found user: {}".format(user)); 
if user == "simon":
    hal_base_path = "/home/simon/projects/hal/"


#some necessary configuration:
build_dir = "build_perf"
sys.path.append(hal_base_path + build_dir + "/lib/") #this is where your hal python lib is located
os.environ["HAL_BASE_PATH"] = hal_base_path + build_dir + "/" # hal base path

import hal_py

netlist_to_read = "/home/simon/workspaces/2023_10_24/canny_edge_detector/canny_edge_detector.hal"
gate_library_path = hal_base_path + "plugins/gate_libraries/definitions/XILINX_UNISIM_hal.hgl"

module_id = 246

#initialize HAL
hal_py.plugin_manager.load_all_plugins()

#read netlist
netlist = hal_py.NetlistFactory.load_netlist(netlist_to_read, gate_library_path)

print("{} - {} - {}".format(netlist.design_name, netlist.device_name, netlist.input_filename))

module = netlist.get_module_by_id(module_id)
gates = module.get_gates(recursive = True)

from hal_plugins import module_identification

#pl = hal_py.plugin_manager.get_plugin_instance("module_identification")

config = module_identification.module_identification.Configuration(netlist)
config.maxControlSignals = 4
#config.known_dana_groupings = initial_dana_regs
config.to_check_types = [
    module_identification.CandidateType.adder, 
    # module_identification.CandidateType.constant_multiplication_left_shift, 
    module_identification.CandidateType.constant_multiplication_right_shift, 
    module_identification.CandidateType.absolute, 
    module_identification.CandidateType.counter, 
    module_identification.CandidateType.equal, 
    module_identification.CandidateType.less_equal, 
    # module_identification.CandidateType.less_than, 
    # module_identification.CandidateType.negation, 
    module_identification.CandidateType.subtraction, 
    module_identification.CandidateType.value_check
]

config.multithreading_handling = module_identification.MultithreadingHandling.memory_priority
config.maxThreadCount = 16


res = module_identification.module_identification.execute_on_gates(gates, config)

#unload everything hal related
hal_py.plugin_manager.unload_all_plugins()