[![pipeline status](https://gitlab.com/swallat/hal/badges/master/pipeline.svg)](https://gitlab.com/swallat/hal/commits/master) [![coverage report](https://gitlab.com/swallat/hal/badges/master/coverage.svg)](https://gitlab.com/swallat/hal/commits/master) [![Gitter chat](https://badges.gitter.im/gitterHQ/gitter.png)](https://gitter.im/emsec-hal)

# Welcome to HAL!

[HAL](http://eprint.iacr.org/2017/783) [/hel/] is a comprehensive reverse engineering and manipulation framework for gate-level netlists focusing on efficiency, extendability and portability. HAL comes with a fully-fledged plugin system, allowing to introduce arbitrary functionalities to the core.

## Features

HAL transforms a netlist from VHDL or Verilog format into a graph representation and vice versa. HAL provides...

- Natural graphical representation of netlist elements and their connections
- Support for custom gate libraries
- Superior performance thanks to our optimized C++ graph library
- Modularity: write your own C++ Plugins for efficient netlist analysis and manipulation (e.g. via graph algorithms)
- A feature-rich GUI allowing for visual netlist inspection and interactive analysis
- An integrated Python shell to exploratively interact with netlist elements and to interface plugins from the GUI

## Citation

If you use HAL, please cite the original [paper](http://eprint.iacr.org/2017/783) using the reference below:

```
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

We plan to publish releases via it's own ppa. You will be able to find it here: [ppa:sebastian-wallat/hal](https://launchpad.net/~sebastian-wallat/+archive/ubuntu/hal)

### macOS

We plan to provide a [homebrew](https://brew.sh/index_de) tap.

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

