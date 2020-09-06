# Dataflow Analysis (DANA)

This plugin is our implementation of our technique DANA, published in TCHES'20 (https://eprint.iacr.org/2020/751)
```
DANA - Universal Dataflow Analysis for Gate-Level Netlist Reverse Engineering
Nils Albartus, Max Hoffmann, Sebastian Temme, Leonid Azriel and Christof Paar
```

## WARNING
The code is currently in the process of being cleaned up.
It is provided as-is and has received several updates since publication of the paper.

## Build Instructions
### Adding a new gate library
For the `XILINX_UNISIM`, `NANGATE` and `LSI_10K` gate library `DANA` should work right away. If the netlist is based on a different gate library you need to create a utility file, with additional information about the sequential gate types. This is necessary since gate-libraries do not provide distinct flags for control and data signals.
Therefore create the regarding `utils_<gate_lib>.h` in `include/utils` and `utils_<gate_lib>.cpp` in `src/utils` and implement the respective function for each port of the gate. Check the existing utils files for an example. Once this is done  the new gate library has to be registered in `src/common/netlist_abstraction.cpp`. Even though the implementation is straightforward and should not take long, we plan to automatically gather these information from the gate library itself in the future.

### Building the plugin
To build the plugin you need to enable it by adding the flag `-DPL_DATAFLOW=ON` to the cmake command. You can use the exemplary commands to build `hal` including the GUI and the dataflow plugin.

```
git clone git@github.com:emsec/hal.git
cd hal
mkdir build
cd build
cmake .. -DPL_DATAFLOW=ON
make
```

## Running the plugin
Currently there are no pybinds available for this plugin, meaning it can only be called via the CLI or be executed from another plugin written in C++.
You have to set the following options, when executing the plugin:
```
  --dataflow                     executes the plugin dataflow
  --path arg                     provide path where the result should be stored
  --layer arg                    (optional) layers per pipeline (default = 1)
  --sizes arg                    (optional) sizes of registers, which are prioritized in the majority voting
```

An exemplary call looks as follows:

```
hal -i hal-benchmarks/crypto/rsa/rsa_lsi_10k_synopsys.v --dataflow --layer 1 --sizes "512,514" --path /home/user/dataflow_out --gate-library lsi_10k.lib
```

## Recreating the paper results
All designs that were used as benchmarks are available in the hal-benchmarks git (https://github.com/emsec/hal-benchmarks).
Clone the repository, adjust the paths at the top of `configuration.py`, and run `control.py` to start the analysis.


## Want to create your own pass?
Todo


## Future Plans
In the long run we plan on integrating the dataflow plugin in a more sophisticated way, especially with the GUI.

* Create pybinds so DANA can be easily used via Python
* GUI integration
  * Make DANA accessible directly in the GUI, e.g., right click on a module (or the entire netlist) and execute DANA.
  * Make DANAs output visible in the GUI, e.g.,  create the register as HAL modules.
* Gather necessary information from the gate lib directly instead of creating the utils first.


## Get in touch
If you have any questions or are looking to cooperate with us, get in touch: <hal@csp.mpg.de>


## Directory Overview

```
hal/plugins/dataflow
├── CMakeLists.txt
├── include
│   ├── common
│   │   ├── grouping.h
│   │   └── netlist_abstraction.h
│   ├── evaluation
│   │   ├── configuration.h
│   │   ├── context.h
│   │   ├── evaluation.h
│   │   ├── result.h
│   │   ├── scoring.h
│   ├── output_generation
│   │   ├── dot_graph.h
│   │   ├── json.hpp
│   │   ├── json_output.h
│   │   ├── state_to_module.h
│   │   ├── svg_output.h
│   │   └── textual_output.h
│   ├── plugin_dataflow.h
│   ├── pre_processing
│   │   ├── counter_identification.h
│   │   ├── pre_processing.h
│   │   └── register_stage_identification.h
│   ├── processing
│   │   ├── configuration.h
│   │   ├── context.h
│   │   ├── pass_collection.h
│   │   ├── passes
│   │   │   ├── group_by_control_signals.h
│   │   │   ├── group_by_input_output_size.h
│   │   │   ├── group_by_successors_predecessors.h
│   │   │   ├── group_by_successors_predecessors_iteratively.h
│   │   │   ├── merge_states.h
│   │   │   ├── merge_successor_predecessor_groupings.h
│   │   │   ├── remove_duplicates.h
│   │   │   └── split_by_successors_predecessors.h
│   │   ├── processing.h
│   │   └── result.h
│   └── utils
│       ├── parallel_for_each.h
│       ├── progress_printer.h
│       ├── timing_utils.h
│       ├── utils.h
│       ├── utils_lsi_10k.h
│       ├── utils_nangate.h
│       └── utils_xilinx_unisim.h
├── README.md
└── src
    ├── common
    │   ├── grouping.cpp
    │   └── netlist_abstraction.cpp
    ├── evaluation
    │   ├── evaluation.cpp
    ├── output_generation
    │   ├── dot_graph.cpp
    │   ├── json_output.cpp
    │   ├── state_to_module.cpp
    │   ├── svg_output.cpp
    │   └── textual_output.cpp
    ├── plugin_dataflow.cpp
    ├── pre_processing
    │   ├── counter_identification.cpp
    │   ├── pre_processing.cpp
    │   └── register_stage_identification.cpp
    ├── processing
    │   ├── pass_collection.cpp
    │   ├── passes
    │   │   ├── group_by_control_signals.cpp
    │   │   ├── group_by_input_output_size.cpp
    │   │   ├── group_by_successors_predecessors.cpp
    │   │   ├── group_by_successors_predecessors_iteratively.cpp
    │   │   ├── merge_states.cpp
    │   │   ├── merge_successor_predecessor_groupings.cpp
    │   │   ├── remove_duplicates.cpp
    │   │   └── split_by_successors_predecessors.cpp
    │   └── processing.cpp
    └── utils
        ├── utils.cpp
        ├── utils_lsi_10k.cpp
        ├── utils_nangate.cpp
        └── utils_xilinx_unisim.cpp
```
