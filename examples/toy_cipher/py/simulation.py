import time

sim_gates = netlist.get_gates()

from hal_plugins import netlist_simulator_controller

pl_sim_ctrl = hal_py.plugin_manager.get_plugin_instance("netlist_simulator_controller")

ctrl_sim = pl_sim_ctrl.create_simulator_controller() 
eng = ctrl_sim.create_simulation_engine("verilator")


print(len(sim_gates))
ctrl_sim.add_gates(sim_gates)


# standard init
for net in ctrl_sim.get_input_nets():
    ctrl_sim.set_input(net, hal_py.BooleanFunction.Value.ZERO)

# get start net
start = netlist.get_nets(lambda g : g.get_name() == "START")[0]

# get start net
done = netlist.get_nets(lambda g : g.get_name() == "DONE")[0]

# add clock
clk = netlist.get_nets(lambda g : g.get_name() == "CLK")[0]
period = 1000
ctrl_sim.add_clock_period(clk, period)

# initialize GND and VCC
ctrl_sim.set_input(netlist.get_net_by_id(1), hal_py.BooleanFunction.Value.ZERO)
ctrl_sim.set_input(netlist.get_net_by_id(2), hal_py.BooleanFunction.Value.ONE)



# get in-/outputs
key = list()
for i in range(0,16):
    key.append(netlist.get_nets(lambda n : n.get_name() == ("KEY_" + str(i)))[0])
print("found {} key bits".format(len(key)))

plaintext = list()
for i in range(0,16):
    plaintext.append(netlist.get_nets(lambda n : n.get_name() == ("PLAINTEXT_" + str(i)))[0])
print("found {} plaintext bits".format(len(plaintext)))

ciphertext = list()
for i in range(0,16):
    ciphertext.append(netlist.get_nets(lambda n : n.get_name() == ("OUTPUT_" + str(i)))[0])
print("found {} ciphertext bits".format(len(ciphertext)))


# start simulation
for net in plaintext:
    ctrl_sim.set_input(net, hal_py.BooleanFunction.Value.ZERO)

for net in key:
    ctrl_sim.set_input(net, hal_py.BooleanFunction.Value.ZERO)


ctrl_sim.set_input(start, hal_py.BooleanFunction.Value.ZERO)
ctrl_sim.simulate(30 * period)


ctrl_sim.set_input(start, hal_py.BooleanFunction.Value.ONE)
ctrl_sim.simulate(30 * period)


ctrl_sim.set_input(start, hal_py.BooleanFunction.Value.ZERO)
ctrl_sim.simulate(30 * period)


for net in key:
    ctrl_sim.set_input(net, hal_py.BooleanFunction.Value.ONE)

ctrl_sim.set_input(start, hal_py.BooleanFunction.Value.ONE)
ctrl_sim.simulate(30 * period)


ctrl_sim.set_input(start, hal_py.BooleanFunction.Value.ZERO)
ctrl_sim.simulate(30 * period)

# simulation propertys
eng.set_engine_property("num_of_threads", "4")


ctrl_sim.initialize()
ctrl_sim.run_simulation()
   
     
while eng.get_state() > 0 and eng.get_state() != 2:
    print ("eng state running %d" % (eng.get_state())) # Output: 1
    time.sleep(1)


    
print ("final state %d" % (eng.get_state())) # Output: 0 on success, -1 on error
ctrl_sim.get_results()


ctrl_sim.add_waveform_group("KEY", key)
ctrl_sim.add_waveform_group("PLAINTEXT", plaintext)
ctrl_sim.add_waveform_group("CIPHERTEXT", ciphertext)
ctrl_sim.add_waveform_group("START", [start])
ctrl_sim.add_waveform_group("CLK", [clk])
ctrl_sim.add_waveform_group("DONE", [done])