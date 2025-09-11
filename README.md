# Welcome to HAL! 
[![Ubuntu 22.04](https://github.com/emsec/hal/actions/workflows/ubuntu22.04.yml/badge.svg)](https://github.com/emsec/hal/actions/workflows/ubuntu22.04.yml)  [![Ubuntu 24.04](https://github.com/emsec/hal/actions/workflows/ubuntu24.04.yml/badge.svg)](https://github.com/emsec/hal/actions/workflows/ubuntu24.04.yml)  [![macOS](https://github.com/emsec/hal/actions/workflows/macOS.yml/badge.svg)](https://github.com/emsec/hal/actions/workflows/macOS.yml) [![Deploy Documentation](https://github.com/emsec/hal/actions/workflows/releaseDoc.yml/badge.svg)](https://github.com/emsec/hal/actions/workflows/releaseDoc.yml) [![Doc: C++](https://img.shields.io/badge/doc-c%2B%2B-orange)](https://emsec.github.io/hal/doc/) [![Doc: Python](https://img.shields.io/badge/doc-python-red)](https://emsec.github.io/hal/pydoc/)


HAL \[/hel/\] is a comprehensive netlist reverse engineering and manipulation framework.

| head |
|------|
| abc  |
| efg  |


| head |
|------|
| <img src="https://raw.githubusercontent.com/emsec/hal/refs/heads/feature/interactive_screenshot/wiki_images/screenshot/hal_screenshot_00.png" width="14.1%"><img src="https://raw.githubusercontent.com/emsec/hal/refs/heads/feature/interactive_screenshot/wiki_images/screenshot/hal_screenshot_01.png" width="56.9%"> |
| <img src="https://raw.githubusercontent.com/emsec/hal/refs/heads/feature/interactive_screenshot/wiki_images/screenshot/hal_screenshot_10.png" width="14.1%"><img src="https://raw.githubusercontent.com/emsec/hal/refs/heads/feature/interactive_screenshot/wiki_images/screenshot/hal_screenshot_11.png" width="56.9%"> |
| <img src="https://raw.githubusercontent.com/emsec/hal/refs/heads/feature/interactive_screenshot/wiki_images/screenshot/hal_screenshot_20.png" width="14.1%"><img src="https://raw.githubusercontent.com/emsec/hal/refs/heads/feature/interactive_screenshot/wiki_images/screenshot/hal_screenshot_21.png" width="56.9%"> |

HAL GUI screenshot
1. [Graph View](#gui-graph-view)
2. [Modules Hierarchy](#gui-modules)
3. [View Management](#gui-views)
4. [Groupings](#gui-groupins)
5. [Selection Details](#gui-selection-details)
6. [Python Editor](#gui-python-editor)
7. [Waveform Viewer](#gui-waveform-viewer)
8. [Python Console](#gui-python-console)
9. [Log Messages](#gui-log-messages)


# Navigation
1. [Introduction](#introduction)
2. [Build Instructions](#build-instructions)
3. [Quickstart Guide](#quickstart)
4. [Academic Context](#academic-context)

<a name="introduction"></a>
# Introduction

## What the hell is HAL?
Virtually all available research on netlist analysis operates on a graph-based representation of the netlist under inspection.
At its core, HAL provides exactly that: A framework to parse netlists of arbitrary sources, e.g., FPGAs or ASICs, into a graph-based netlist representation and to provide the necessary built-in tools for traversal and analysis of the included gates and nets.

Our vision is that HAL becomes the hardware-reverse-engineering-equivalent of tools like IDA or Ghidra.
We want HAL to enable a common baseline for researchers and analysts to improve reproducibility of research results and abstract away recurring basic tasks such as netlist parsing etc.
- **High performance** thanks to the optimized C++ core
- **Flexibility** through built-in Python bindings
- **Modularity** via a C++ plugin system
- **Stability** is ensured via a rich test suite

HAL is actively developed by the Embedded Security group of the [Max Planck Institute for Security and Privacy](https://www.mpi-sp.org).
Apart from multiple research projects, it is also used in our university lecture "Einführung ins Hardware Reverse Engineering" (Introduction to Hardware Reverse Engineering) at Ruhr University Bochum (RUB).

Note that we also have a set of **modern** state-of-the-art benchmark circuits for the evaluation of netlist reverse engineering techniques available in a seperate [repository](https://github.com/emsec/hal-benchmarks).

## Shipped Plugins
This repository contains a selection of curated plugins:
- **GUI:** A feature-rich GUI allowing for visual netlist inspection and interactive analysis
  - Native integration of a Python shell with access to the HAL Python bindings
  - Isolation of specific gates or modules for clutter-free inspection
  - Interactive traversal of netlists
  - Detailed widgets with information on all aspects of the inspected netlist
- **Netlist Simulator:** A simulator for arbitrary parts of a loaded netlist
- **Dataflow Analysis:** Our dataflow analysis plugin [DANA](https://eprint.iacr.org/2020/751.pdf) that recovers high-level registers in an unstructured netlist
- **Graph Algorithms:** [igraph](https://igraph.org) integration for direct access to common algorithms from graph-theory
- **Python Shell:** A command-line plugin to spawn a Python shell preloaded with the HAL Python bindings
- **VHDL & Verilog Parsers:** Adds support for parsing VHDL and Verilog files as netlist input formats
- **Liberty Parser:** Adds support for arbitrary gate libraries in the standard `liberty` gate library format
- **VHDL & Verilog Writers:** Adds support for serializing a (modified) netlist to synthesizable VHDL or Verilog files
- **Gate Libraries:** Adds support for the XILINX Unisim and Simprim gate libraries


## Documentation
A comprehensive documentation of HAL's features from a user perspective is available in our [Wiki](https://github.com/emsec/hal/wiki). In addition, we provide a full [C++ API](https://emsec.github.io/hal/doc/) and [Python API](https://emsec.github.io/hal/pydoc/) documentation.

<a name="build-instructions"></a>
# Build Instructions 

For instructions on how to build HAL, please refer to the dedicated page in our [Wiki](https://github.com/emsec/hal/wiki/Building-HAL).

<a name="quickstart"></a>
# Quickstart Guide 

Install HAL or build HAL and start the GUI via `hal -g`. You can list all available options via `hal [--help|-h]`.
We included some example netlists in `examples` together with the implementation of the respective example gate library in `plugins/example_gate_library`.
For instructions to create your own gate library and other useful tutorials, take a look at the [wiki](https://github.com/emsec/hal/wiki).

Load a library from the `examples` directory and start exploring the graphical representation.
Use the integrated Python shell or the Python script window to interact. Both feature (limited) autocomplete functionality.

Let's list all lookup tables and print their Boolean functions:
```python
for gate in netlist.get_gates():
    if "LUT" in gate.type.name:
        print("{} (id {}, type {})".format(gate.name, gate.id, gate.type.name))
        print("  {}-to-{} LUT".format(len(gate.type.input_pins), len(gate.type.output_pins)))
        boolean_functions = gate.boolean_functions
        for name in boolean_functions:
            print("  {}: {}".format(name, boolean_functions[name]))
        print("")
```
For the example netlist `fsm.vhd` this prints:
```text
FSM_sequential_STATE_REG_0_i_3_inst (id 4, type LUT6)
  6-to-1 LUT
  O: (!I1 & !I2 & I3 & !I4 & I5) | (I0 & !I2) | (I0 & I1) | (I0 & I3) | (I0 & I4) | (I0 & I5)

FSM_sequential_STATE_REG_0_i_2_inst (id 3, type LUT6)
  6-to-1 LUT
  O: (I2 & I3 & I4 & !I5) | (I1 & !I5) | (I1 & !I4) | (I1 & !I3) | (I0 & I1) | (I1 & I2)

FSM_sequential_STATE_REG_1_i_3_inst (id 6, type LUT6)
  6-to-1 LUT
  O: (!I1 & I4 & !I5) | (!I1 & !I3 & I4) | (I0 & I4 & !I5) | (I0 & !I3 & I4) | (!I1 & I2 & I4) | (I0 & I2 & I4) | (!I2 & !I5) | (!I2 & !I4) | (!I2 & !I3) | (!I0 & !I4) | (!I0 & !I2) | (!I0 & !I1) | (I1 & !I4) | (I1 & !I2) | (I0 & I1) | (I3 & !I5) | (I3 & !I4) | (!I0 & I3) | (I1 & I3) | (I2 & I3) | (!I4 & I5) | (!I3 & I5) | (!I0 & I5) | (I1 & I5) | (I2 & I5)

FSM_sequential_STATE_REG_1_i_2_inst (id 5, type LUT6)
  6-to-1 LUT
  O: (!I0 & I1 & !I2 & I3 & I4 & !I5) | (I0 & !I2 & I3 & I4 & I5)

OUTPUT_BUF_0_inst_i_1_inst (id 18, type LUT1)
  1-to-1 LUT
  O: !I0

OUTPUT_BUF_1_inst_i_1_inst (id 20, type LUT2)
  2-to-1 LUT
  O: (I0 & !I1) | (!I0 & I1)
```

# Contributing

You are welcome to contribute to the development of HAL. Feel free to submit a new pull request via github.
Please consider running the static checks + `clang format` before that.
You can also install these checks as git hooks before any commit.

## Run static checks and clang format locally
To install clang-format hook install [git-hooks](https://github.com/icefox/git-hooks) and run:

`git hooks --install`

Start Docker build via:
`docker-compose run --rm hal-build`

## Generate Changelog

`git log $(git describe --tags --abbrev=0)..HEAD --pretty=format:"%s" --no-merges`

<a name="academic-context"></a>
# Academic Context 

If you use HAL in an academic context, please cite the framework using the reference below:
```latex
@misc{hal,
    author = {{Embedded Security Group}},
    publisher = {{Max Planck Institute for Security and Privacy}},
    title = {{HAL - The Hardware Analyzer}},
    year = {2019},
    howpublished = {\url{https://github.com/emsec/hal}},
}
```

Feel free to also include the original [paper](http://eprint.iacr.org/2017/783). However, we note that HAL has massively changed since its original prototype that was described in the paper.
Hence, we prefer citing the above entry.
```latex
@article{2018:Fyrbiak:HAL,
    author = {Marc Fyrbiak and Sebastian Wallat and Pawel Swierczynski and Max Hoffmann and Sebastian Hoppach and Matthias Wilhelm and Tobias Weidlich and Russell Tessier and Christof Paar},
    title = {{HAL-} The Missing Piece of the Puzzle for Hardware Reverse Engineering, Trojan Detection and Insertion},
    journal = {IEEE Transactions on Dependable and Secure Computing},
    year = {2018},
    publisher = {IEEE},
    howpublished = {\url{https://github.com/emsec/hal}}
}
```

To get an overview on the challenges we set out to solve with HAL, feel free to watch our [talk](https://media.ccc.de/v/36c3-10879-hal_-_the_open-source_hardware_analyzer) at 36C3.


# Licensing
HAL is licensed under MIT License to encourage collaboration with other research groups and contributions from the industry. Please refer to the license file for further information.

# Disclaimer
HAL is at most alpha-quality software.
Use at your own risk.
We do not encourage any malicious use of our toolkit.
