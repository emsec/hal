[![pipeline status](https://gitlab.com/swallat/hal/badges/master/pipeline.svg)](https://gitlab.com/swallat/hal/commits/master) [![coverage report](https://gitlab.com/swallat/hal/badges/master/coverage.svg)](https://gitlab.com/swallat/hal/commits/master) [![Gitter chat](https://badges.gitter.im/gitterHQ/gitter.png)](https://gitter.im/emsec-hal)

# Welcome to HAL!

[HAL](http://eprint.iacr.org/2017/783) [/hel/] is a comprehensive reverse engineering and manipulation framework for gate-level netlists focusing on efficiency, extendability and portability. HAL comes with a fully-fledged plugin system, allowing to introduce arbitrary functionalities to the core.

![HAL Screenshot](https://raw.githubusercontent.com/emsec/hal/master/hal_screenshot.png "HAL Screenshot")

## Features
- Natural directed graph representation of netlist elements and their connections
- Support for custom gate libraries
- High performance thanks to optimized C++ core
- Modularity: write your own C++ Plugins for efficient netlist analysis and manipulation (e.g. via graph algorithms)
- A feature-rich GUI allowing for visual netlist inspection and interactive analysis
- An integrated Python shell to exploratively interact with netlist elements and to interface plugins from the GUI

## API Documentation

The C++ documentation is available [here](https://doc.hal.emsec.rub.de/).
The Python documentation can be found [here](https://py-doc.hal.emsec.rub.de/).

## Quick Start

Install or build HAL and start the GUI via `hal -g`. You can list all available options via `hal [--help|-h]`.
We included some example netlists in `examples` together with the implementation of the respective example gate library in `plugins/example_gate_library`.
For instructions to create your own gate library and other useful tutorials, take a look at the [wiki](https://github.com/emsec/hal/wiki).

Load a library from the `examples` directory and start exploring the graphical representation.
Use the integrated Python shell or the Python script window to interact. Both feature (limited) autocomplete functionality.

Let's list all lookup tables and print their Boolean functions:
```python
from hal_plugins import libquine_mccluskey

qm_plugin = libquine_mccluskey.quine_mccluskey()

for gate in netlist.get_gates():
    if "LUT" in gate.type:
        print(gate.name + " (id "+str(gate.id) + ", type " + gate.type + ")")
        print("  " + str(len(gate.input_pin_types)) + "-to-" + str(len(gate.output_pin_types)) + " LUT")
        boolean_functions = qm_plugin.get_boolean_function_str(gate, False)
        for pin in boolean_functions:
            print("  " + pin + ": "+boolean_functions[pin])
        print("")
```
For the example netlist `fsm.vhd` this prints:
```
FSM_sequential_STATE_REG_1_i_2_inst (id 5, type LUT6)
  6-to-1 LUT
  O: (~I0 I1 ~I2 I3 I4 ~I5) + (I0 ~I2 I3 I4 I5)

FSM_sequential_STATE_REG_0_i_2_inst (id 3, type LUT6)
  6-to-1 LUT
  O: (I2 I3 I4 ~I5) + (I1 I2) + (I0 I1) + (I1 ~I3) + (I1 ~I4) + (I1 ~I5)

FSM_sequential_STATE_REG_0_i_3_inst (id 4, type LUT6)
  6-to-1 LUT
  O: (~I1 ~I2 I3 ~I4 I5) + (I0 I5) + (I0 I4) + (I0 I3) + (I0 I1) + (I0 ~I2)

OUTPUT_BUF_0_inst_i_1_inst (id 18, type LUT1)
  1-to-1 LUT
  O: (~I0)

OUTPUT_BUF_1_inst_i_1_inst (id 20, type LUT2)
  2-to-1 LUT
  O: (~I0 I1) + (I0 ~I1)

FSM_sequential_STATE_REG_1_i_3_inst (id 6, type LUT6)
  6-to-1 LUT
  O: (I0 I2 I4) + (~I1 I2 I4) + (I0 ~I3 I4) + (~I1 ~I3 I4) + (I0 I4 ~I5) + (~I1 I4 ~I5) + (I2 I5) + (I2 I3) + (I1 I5) + (I1 I3) + (I0 I1) + (~I0 I5) + (~I0 I3) + (~I0 ~I1) + (I1 ~I2) + (~I0 ~I2) + (~I3 I5) + (~I2 ~I3) + (~I4 I5) + (I3 ~I4) + (I1 ~I4)
```

## Citation

If you use HAL in an academic context, please cite the framework using the reference below:
```latex
@misc{hal,
    author = {Marc Fyrbiak and Sebastian Wallat and Max Hoffmann},
    title = {{HAL - The Hardware Analyzer}},
	year = {2019},
    howpublished = {\url{https://github.com/emsec/hal}},
}
```

Feel free to also include the original [paper](http://eprint.iacr.org/2017/783)
```latex
@article{2018:Fyrbiak:HAL,
      author    = {Marc Fyrbiak and
                   Sebastian Wallat and
                   Pawel Swierczynski and
                   Max Hoffmann and
                   Sebastian Hoppach and
                   Matthias Wilhelm and
                   Tobias Weidlich and
                   Russell Tessier and
                   Christof Paar},
  title     	= {{HAL-} The Missing Piece of the Puzzle for Hardware Reverse Engineering,
               	  Trojan Detection and Insertion},
  journal		= {IEEE Transactions on Dependable and Secure Computing},
  year			= {2018},
  publisher		= {IEEE},
  howpublished 	= {\url{https://github.com/emsec/hal}}
}
```

## Contact and Support

Please contact us via Gitter: https://gitter.im/emsec-hal

## Install Instructions

### Ubuntu

HAL releases are available via it's own ppa. You can find it here: [ppa:sebastian-wallat/hal](https://launchpad.net/~sebastian-wallat/+archive/ubuntu/hal)

### macOS

A [homebrew](https://brew.sh/index_de) tap is coming soon...

## Build Instructions

Run the following commands to download and install HAL.

1. `git clone https://github.com/emsec/hal.git && cd hal`
2. To install all neccessary dependencies execute `./install_dependencies.sh`
3. `mkdir build && cd build`
4. `cmake .. `
5. `make`

Optionally you can install HAL:

`make install`

## Contributing

You are very welcome to contribute to the development of HAL. Feel free to submit a new pull request via github. Please consider running the static checks + clang format before that. You can also install these checks as git hooks before any commit.

### Run static checks and clang format locally
To install clang-format hook install git-hooks (https://github.com/icefox/git-hooks) and run:

`git hooks --install`

Start Docker build via:
`docker-compose run --rm hal-build`

### Generate Changelog

`git log $(git describe --tags --abbrev=0)..HEAD --pretty=format:"%s" --no-merges`

## Licensing

HAL is licensed under MIT License to encourage collaboration with other research groups and contributions from the industry. Please refer to the license file for further information.

## Disclaimer

HAL is at most alpha-quality software. Use at your own risk. We do not encourage any malicious use of our toolkit.

