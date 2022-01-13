
import csv
import time
sim_module = netlist.get_top_module()
sim_gates = sim_module.get_gates(lambda g : g.get_type().get_name() not in ["SB_IO", "SB_GB_IO", "HAL_GND", "HAL_VDD"], recursive=True)
path_to_vcd = "/Users/eve/Uni/Projekte/hal/plugins/simulator/netlist_simulator_controller/test/netlists/counter/dump.vcd"
path_to_vcd = "/Users/eve/Downloads/created_waveform.vcd"



from hal_plugins import netlist_simulator_controller
start = time.time()
pl_sim_ctrl = hal_py.plugin_manager.get_plugin_instance("netlist_simulator_controller")
ctrl_sim = pl_sim_ctrl.create_simulator_controller() 


print(len(sim_gates))
ctrl_sim.add_gates(sim_gates)


ctrl_sim.parse_vcd(path_to_vcd, netlist_simulator_controller.CompleteNetlist, silent=True)