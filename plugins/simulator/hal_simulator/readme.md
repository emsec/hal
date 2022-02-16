# Netlist Simulator Plugin
A plugin for simple simulation of (parts of) a HAL netlist.

## Quickstart Guide
Use the plugin manager to get the plugin instance.
Via this instance you can create new simulator instances via `create_simulator`.

Given a simulator, follow these steps:
1. add all gates you want to simulate via `add_gates`
2. add a clock to the main clock net(s) of your design via one of the `add_clock_*` functions
3. in case you use an FPGA netlist where FFs can have initial values, use `load_initial_values` to load those
4. set the state of specific (input) nets via `set_input`
5. simulate for a specific duration via `simulate`
6. obtain the simulation trace via `get_simulation_state`

An exemplary simulation run may look as follows:
```
auto simulator = plugin->create_simulator();
simulator->add_gates(all_gates_to_simulate);
simulator->add_clock_hertz(clock_net, frequency);
simulator->set_input(input_net_1, SignalValue::ONE);
simulator->set_input(input_net_2, SignalValue::ZERO);
simulator->simulate(5000);
simulator->set_input(input_net_2, SignalValue::ONE);
simulator->simulate(4300);
auto simulation = simulator->get_simulation_state();
```

The simulation obtained via `get_simulation_state` contains all events, i.e., signal changes that occured since the start of the simulation.

The events can be obtained via `get_events` and the value of a specific signal at a specific point in time can be obtained via `get_net_value`.

## Known Issues / TODOs
* Tri-State Z-value not supported
* Propagation delays not supported (in theory delays ARE already supported, but at the current time HAL does not support parsing of gate delays)
