#!/usr/bin/env python3
import sys, os

#some necessary configuration:
# 
# sys.path.append("/Users/eve/Documents/Uni/Projekte/hal/build/lib/") #this is where your hal python lib is located
# os.environ["HAL_BASE_PATH"] = "/Users/eve/Documents/Uni/Projekte/hal/build" # hal base path
sys.path.append("/Users/eve/hal/build/lib/") #this is where your hal python lib is located
os.environ["HAL_BASE_PATH"] = "/Users/eve/hal/build" # hal base path
import hal_py

#netlist_to_read = "/Users/eve/hal/plugins/simulator/verilator_simulator/test_files/netlists/weird_latch/weird_latch.v"
netlist_to_read = "/Users/eve/hal/plugins/simulator/verilator_simulator/test_files/netlists/normal_latch/normal_latch.v"
#netlist_to_read = "/Users/eve/hal/plugins/simulator/verilator_simulator/test_files/netlists/complex_ff/complex_ff.v"
gate_library_path = "/Users/eve/hal/plugins/gate_libraries/definitions/lsi_10k.hgl"

#initialize HAL
hal_py.plugin_manager.load_all_plugins()

#read netlist
netlist = hal_py.NetlistFactory.load_netlist(netlist_to_read, gate_library_path)

from hal_plugins import verilator_simulator

verilator_simulator.VerilatorSimulator.convert_gate_library_to_verilog(netlist)

#unload everything hal related
hal_py.plugin_manager.unload_all_plugins()