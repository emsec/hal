# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.1.8] - 2019-09-19 14:23:59+02:00

* Fix command line parsing
* Fix parse all_options from main
* Fixed set log option as command line parameter without stopping execution
* Fixed assign parsing for yosys verilog output

## [1.1.7] - 2019-08-26 10:41:58-04:00

* Changed myself library to synopsys_nand_nor
* Fixed bug in bdd gen
* Added new gate library!
* Added python decorator to log function calls
* Changed question when hal file exists for better clarity
* Fixes #179. Parse yosys netlist correct. (#181)
* Set new vhdl parser as default, keep old one as fallback (#180)

## [1.1.6] - 2019-08-05 15:55:18+02:00

* Fixed ubuntu ppa build

## [1.1.5] - 2019-08-05 15:14:35+02:00

* Added uninstall target and fixed make/ninja install on linux with correct ldconfig settings
* Test/test update (#176)
* Fixed OpenMP build for macOS 2nd try
* Fixed cmake on macOS
* resetted utils to master
* resetted some files to master
* Update Dockerfile
* Update install_dependencies.sh
* Update Dockerfile
* modified dependencies to include igraph
* reset files from vhdl_modules_feature branch
* Update scan_ff.py
* added missing line in python script
* added new gate_library for scan_ff      - added gate decorator  - added json for library
* cleaned community_detection
* community_detection working with igraph_community_fastgreedy + printing and plotting results
* added nangate gate library
* minor fixes
* Fix documentation error
* removed bug in split function in utils; added file exists function
* fixed some shadow warnings
* added file_exists function to core::utils
* implemented graph plot + community fastgreedy algorithm
* openmp
* prepared community detection with igraph
* Updated brew file to include graph
* added igraph to cmake
* added igraph
* bug in hdl writer - backslashes are being removed
* fixxed naming for modules

## [1.1.4] - 2019-07-15 14:53:49+02:00

* Remember py path for last 'saved as'
* Fix several bugs regarding file modified state
* Add file modified indicator
* Fix function call
* Remove unnecessary function parameters
* added missing includes
* Remove malformed whitespaces
* Remove global content manager artifacts
* Add python file modified indicator
* Deglobalize hal content manager
* Connect netlist watcher and file status manager
* Integrate global file status manager
* Add Quuid to python code editor
* Add global class to track modified files
* recent-files-items now keep track of their files and are removeable. This fixes issue #136
* Fix content manager integration
* Integrate content manager to previous solution
* Fix errors after master merge
* Add ability to reload pyscript if modified on disk

## [1.1.3] - 2019-07-03 10:10:12+02:00

* Remember last opened path for python scripts
* Add multi file selection for python scripts

## [1.1.2] - 2019-06-27 14:21:03+02:00

* Add hint to gate_libraries to README.md
* Changes to the global state can be handled via hal_content_manager

## [1.1.1] - 2019-06-27 14:21:03+02:00

* Quickfix for Xilinx Simprim gate library

## [1.1.0] - 2019-06-27 12:28:36+02:00

* Added Support for Xilinx Unisim gate library
* Added Support for Xilinx Simprim gate library
* Added Support for Synopsys 90nm gate library
* Added Support for GSCLIB 3.0 gate library
* Added Support for UMC 0.18 um gate library
* Added VHDL and Verilog parser fixes
* Fixeds issue #143
* Fixed static lint issues

## [1.0.8] - 2019-04-30 19:46:40+02:00 (urgency: high)

* Added plugin directory to include_directories

## [1.0.7] - 2019-04-30 16:12:10+02:00 (urgency: high)

* Added missing python defs
* Fixed misleading error message, fixed wrong module signal
* Fix graph_algorithm include

## [1.0.6] - 2019-04-29 18:52:10+02:00 (urgency: high)

* Added sunny theme
* Fixes plugin name for python import

## [1.0.5] - 2019-04-26 16:55:18+02:00 (urgency: high)

* GIT repo cleanup

## [1.0.4] - 2019-04-26 16:55:18+02:00 (urgency: high)

* Fixes GUI not start from package install again

## [1.0.3] - 2019-04-26 15:20:17+02:00 (urgency: high)

* Fixes GUI not start from package install

## [1.0.2] - 2019-04-26 01:38:17+02:00 (urgency: high)

* Fix in install buddy header again

## [1.0.1] - 2019-04-25 20:44:35+02:00 (urgency: high)

* Fix in install buddy header

## [1.0.0] - 2019-04-25 16:32:50+02:00 (urgency: low)

* Initial Release

[//]: # (Hyperlink section)
[Unreleased]: https://github.com/emsec/hal/compare/v1.1.8...HEAD
[1.1.8]: https://github.com/emsec/hal/compare/v1.1.7...v1.1.8
[1.1.7]: https://github.com/emsec/hal/compare/v1.1.6...v1.1.7
[1.1.6]: https://github.com/emsec/hal/compare/v1.1.5...v1.1.6
[1.1.5]: https://github.com/emsec/hal/compare/v1.1.4...v1.1.5
[1.1.4]: https://github.com/emsec/hal/compare/v1.1.3...v1.1.4
[1.1.3]: https://github.com/emsec/hal/compare/v1.1.2...v1.1.3
[1.1.2]: https://github.com/emsec/hal/compare/v1.1.1...v1.1.2
[1.1.1]: https://github.com/emsec/hal/compare/v1.1.0...v1.1.1
[1.1.0]: https://github.com/emsec/hal/compare/v1.0.8...v1.1.0
[1.0.8]: https://github.com/emsec/hal/compare/v1.0.7...v1.0.8
[1.0.7]: https://github.com/emsec/hal/compare/v1.0.6...v1.0.7
[1.0.6]: https://github.com/emsec/hal/compare/v1.0.5...v1.0.6
[1.0.5]: https://github.com/emsec/hal/compare/v1.0.4...v1.0.5
[1.0.4]: https://github.com/emsec/hal/compare/v1.0.3...v1.0.4
[1.0.3]: https://github.com/emsec/hal/compare/v1.0.2...v1.0.3
[1.0.2]: https://github.com/emsec/hal/compare/v1.0.1...v1.0.2
[1.0.1]: https://github.com/emsec/hal/compare/v1.0.0...v1.0.1
[1.0.0]: https://github.com/emsec/hal/releases/tag/v1.0.0