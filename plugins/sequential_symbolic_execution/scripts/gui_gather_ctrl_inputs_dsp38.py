
# first extract all ctrl nets

mod_id = 451

non_ctrl_input_mod_ids = [6, 8, 21, 22, 135, 136, 164]

mod = netlist.get_module_by_id(mod_id)

ctrl_nets = list()
ctrl_grouping = netlist.create_grouping("Ctrl Grouping")

for in_net in mod.get_input_nets():
    src = in_net.get_sources()[0]
    
    if src.gate.module.id not in non_ctrl_input_mod_ids:
        ctrl_grouping.assign_net(in_net)
        ctrl_nets.append(in_net)


# get time steps that we wanna pass to the symbolic execution (in this case all rising clock edges)

t_start = 12353277697
t_stop  = 12353279361
time_steps = list()

clk_id = 4254
clk_net = netlist.get_net_by_id(clk_id)

waveform = ctrl_sim.get_waveform_by_net(clk_net) 
t0 = t_start                                   # start time for requested event data buffer (list of tuples)
while (events := waveform.get_events(t0)):   # as long as there are events ...
    t0 = events[-1][0] + 1                   # next start time := maxtime + 1
    for tuple in events:                     # dump events in console
        if tuple[1] == 1:
            time_steps.append(tuple[0])

            if tuple[0] > t_stop:
                break

    if t0 > t_stop:
        break

print(len(time_steps))
print(time_steps)

ctrl_events = list()

for i in range(len(time_steps)):
    ctrl_events.append(dict())

for ctrl_net in ctrl_nets:
    ctrl_waveform = ctrl_sim.get_waveform_by_net(ctrl_net)

    for t_index in range(len(time_steps)):
        t = time_steps[t_index]
        if ctrl_net.id == clk_net.id:
            ctrl_val = ctrl_waveform.get_value_at(t)
        else:
            ctrl_val = ctrl_waveform.get_value_at(t-1)
        ctrl_events[t_index][ctrl_net.id] = ctrl_val
        
        #print("Got {} at {} / {} for {}".format(ctrl_val, t-1, t_index, ctrl_net.id))
        
        if ctrl_val not in [0, 1]:
            print("ERROR, got value {} at {} for {}!".format(ctrl_val, t, ctrl_net.id))

#print(ctrl_events[3])
print(ctrl_events)


