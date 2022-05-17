#!/usr/bin/env python3
import sys, os

#some necessary configuration:
# 
# sys.path.append("/Users/eve/Documents/Uni/Projekte/hal/build/lib/") #this is where your hal python lib is located
# os.environ["HAL_BASE_PATH"] = "/Users/eve/Documents/Uni/Projekte/hal/build" # hal base path
sys.path.append("/Users/eve/hal/build/lib/") #this is where your hal python lib is located
os.environ["HAL_BASE_PATH"] = "/Users/eve/hal/build" # hal base path
import hal_py

netlist_to_read = "/Users/eve/hal/plugins/simulator/verilator/test_files/netlists/counter/counter.v"
gate_library_path = "/Users/eve/hal/plugins/gate_libraries/definitions/XILINX_UNISIM.hgl"

#initialize HAL
hal_py.plugin_manager.load_all_plugins()

#read netlist
netlist = hal_py.NetlistFactory.load_netlist(netlist_to_read, gate_library_path)
from hal_plugins import netlist_simulator_controller

pl_sim_ctrl = hal_py.plugin_manager.get_plugin_instance("netlist_simulator_controller")
sim = pl_sim_ctrl.create_simulator_controller()

sim.add_gates(netlist.get_gates())
sim.create_simulation_engine("verilator")

#sim.load_initial_values()

Clock_enable_B = netlist.get_net_by_id(8)
clock = netlist.get_net_by_id(5)
reset = netlist.get_net_by_id(6)
output_0 = netlist.get_net_by_id(9)
output_1 = netlist.get_net_by_id(3)
output_2 = netlist.get_net_by_id(4)
output_3 = netlist.get_net_by_id(7)

clock_period = 10000

sim.add_clock_period(clock, clock_period)

sim.set_input(reset, hal_py.BooleanFunction.Value.ONE)
sim.simulate(1 * clock_period)

sim.set_input(reset, hal_py.BooleanFunction.Value.ZERO)
sim.simulate(1 * clock_period)

sim.set_input(Clock_enable_B, hal_py.BooleanFunction.Value.ONE)
sim.simulate(5 * clock_period)

sim.set_input(Clock_enable_B, hal_py.BooleanFunction.Value.ZERO)
sim.simulate(2 * clock_period)

sim.set_input(reset, hal_py.BooleanFunction.Value.ONE)
sim.simulate(2 * clock_period)

sim.set_input(reset, hal_py.BooleanFunction.Value.ZERO)
sim.simulate(1 * clock_period)

sim.set_input(Clock_enable_B, hal_py.BooleanFunction.Value.ONE)
sim.simulate(20 * clock_period)


sim.run_simulation()
sleep(30)



#sim.generate_vcd("counter.vcd", 0, 300000)

#unload everything hal related
hal_py.plugin_manager.unload_all_plugins()