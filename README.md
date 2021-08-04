<a name="introduction"></a>
# Welcome to HAL! 
[![Ubuntu 20.04](https://github.com/emsec/hal/actions/workflows/ubuntu20.04.yml/badge.svg)](https://github.com/emsec/hal/actions/workflows/ubuntu20.04.yml)  [![macOS](https://github.com/emsec/hal/actions/workflows/macOS.yml/badge.svg)](https://github.com/emsec/hal/actions/workflows/macOS.yml) [![Doc: C++](https://img.shields.io/badge/doc-c%2B%2B-orange)](https://emsec.github.io/hal/doc/) [![Doc: Python](https://img.shields.io/badge/doc-python-red)](https://emsec.github.io/hal/pydoc/) [![deployment](https://gitlab.com/swallat/hal/badges/master/pipeline.svg)](https://gitlab.com/swallat/hal/commits/master) 


HAL \[/hel/\] is a comprehensive netlist reverse engineering and manipulation framework.

![HAL Screenshot](https://raw.githubusercontent.com/emsec/hal/master/hal_screenshot.png "HAL Screenshot")




# Navigation
1. [Introduction](#introduction)
2. [Install Instructions](#install-instructions)
3. [Build Instructions](#build-instructions)
4. [Quickstart Guide](#quickstart)
5. [Academic Context](#academic-context)

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
Apart from multiple research projects, it is also used in our university lecture [Introduction to Hardware Reverse Engineering](https://www.ei.ruhr-uni-bochum.de/studium/lehrveranstaltungen/832/).

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

## Slack, Contact and Support
For all kinds of inquiries, please contact us using our dedicated e-mail address: [hal@csp.mpg.de](mailto:hal@csp.mpg.de). To receive an invite to our dedicated hal-support Slack workspace, please write us an e-mail as well.


<a name="install-instructions"></a>
# Install Instructions 

## Ubuntu 20.04

HAL releases are available via it's own ppa, which can be found here: [ppa:sebastian-wallat/hal](https://launchpad.net/~sebastian-wallat/+archive/ubuntu/hal). If you wish to always work with the most recent version of HAL, we recommend building HAL yourself.


<a name="build-instructions"></a>
# Build Instructions 

## Ubuntu 18.04

We do currently not support building HAL on Ubuntu 18.04. We are working towards a solution and will update this guide as soon as possible.

## Ubuntu 20.04

If you want to build HAL on Ubuntu 20.04, run the following commands:

1. `git clone https://github.com/emsec/hal.git && cd hal` to clone the Git repository
2. `./install_dependencies.sh` to install all required dependencies
3. `mkdir build && cd build` to create and move to the build folder
4. `cmake .. [OPTIONS]` to run cmake
5. `make` to compile HAL
6. `make install` (optionally) to install HAL

## macOS

**Warning:** Building on macOS is experimental and may not always work.

Please make sure to use a compiler that supports OpenMP. You can install one using, e.g., Homebrew via: `brew install llvm`.

To let cmake know of the custom compiler use following command.

`cmake .. -DCMAKE_C_COMPILER=/usr/local/opt/llvm/bin/clang -DCMAKE_CXX_COMPILER=/usr/local/opt/llvm/bin/clang++`

## CMake Options
Using the CMake build system, your HAL build can be configured quite easily (by adding `-D<OPTION>=1` to the cmake command).
Here is a selection of the most important options:
- `BUILD_TESTS`: builds all available tests which can be executed by running `ctest` in the build directory.
This also builds all tests of plugins that are built.
- `BUILD_DOCUMENTATION`: builds the C++ and Python documentation in the directory \<build directory\>/documentation/
- `PL_<plugin name>`: enable (or disable) building a specific plugin
- `BUILD_ALL_PLUGINS`: all-in-one option to build all available plugins, overrides the options for individual plugins
- `SANITIZE_ADDRESS`, `SANITIZE_MEMORY`, `SANITIZE_THREAD`, `SANITIZE_UNDEFINED `: builds with the respective sanitizers (recommended only for debug builds)

If you do not specify `CMAKE_BUILD_TYPE`, it defaults to `Release`.

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
        print("  {}-to-{} LUT".format(len(gate.input_pins), len(gate.output_pins)))
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
