# module_identification


## Install plugin

clone this repository into your plugins folder
```
cd hal base directory
cd build
```
remove your current build and remake it
```
rm -rf * && cmake .. -DBUILD_ALL_PLUGINS=1 -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=1
make
```


## Name
Module Identification plugin for hal.

## Function

This plugin tries to identify groups of gates inside a netlist fulfilling a function that this plugin can recognize.
The possible operations that this plugin can recognize and verify are mostly of arithmetic nature and include addition, subtraction, comparisons, multiplication with a constant and similar.

No previous information is necessary, however, providing knowledge like register groupings to the plugin can improve its result.

The plugin first identifies base candidates that serve as a starting point for its search.
Based on these base candidates it we build strucutral variants that consist of the base candidate and different gates surrounding it.
This step is architecture depentend and needs to be implemented for every target this plugin is applied to.
So far we support all netlists using the XILINX_UNISIM elements and the lattice ice40 fpga gate library.

After building structural candidates the plugin than tries to identify which inputs of the subgraph belong to different operands and in which order as well as which signals might be control signals.
Since some of the required information might not be reconstructable the plugin will make some guesses and create for each structural candidate multiple functional candidates that "guess" which function on which operands the subgraph might compute.

As a third step we check all functional candidates using an SMT solver and verify which functional candidate is actually correct.

In a final step we are left with a set of verified_candidates for each base candidate and have to decide which one we actually want to choose to annotate in the netlist as a module.
This is done by considering many metrics, like size of the candidate, input sizes, output sizes, amount of control inputs and similar.
