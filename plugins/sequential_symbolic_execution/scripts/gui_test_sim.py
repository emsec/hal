
test_time = 12353277945
test_net = netlist.get_net_by_id(2634)
test_waveform = ctrl_sim.get_waveform_by_net(test_net)
test_val = test_waveform.get_value_at(test_time)

print(test_val)