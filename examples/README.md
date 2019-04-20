# Example Gate-Level Netlists

We included some example gate-level netlists that work with our example gate-library. These can be loaded into HAL if you just want a quick demo in HAL.

## FSM
This is a simple FSM with 4 states. The FSM has 5 external input signals and 2 outputs. In the paper [On the Difficulty of FSM-based Hardware Obfuscation](https://tches.iacr.org/index.php/TCHES/article/view/7277) we introduced strategies for finding FSM circuits in a gate level netlist as well as extracting the state transition graph. One could implement the algorithms and metrics in ones own HAL plugin in C++ or use the python shell/editor incorporated in the GUI.

In total the netlist consists of 21 gates and 35 nets.

## UART
A UART core that waits until 64 bit have been received and simply sends the exact 64 bit back. The UART core has a baud rate set 9600, uses one stop bit and works at 100Mhz. In total the netlist has 407 gates and 412 nets.