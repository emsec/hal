# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [3.1.5] - 2020-11-16 14:30:00+02:00 (urgency: medium)
* fixed and expanded Python decorators for logging in study environment to additionally cover the GUI
* fixed infinite loop when renaming groupings using the GUI

## [3.1.4] - 2020-11-15 14:00:00+02:00 (urgency: medium)
* added tests for `netlist_utils`
* fixed crash when creating invalid module when using `netlist::create_module`
* fixed assigning nets instead of gates when copying netlist using `netlist_utils::copy_netlist`
* fixed `netlist_utils::copy_netlist` not copying port names
* fixed `netlist_utils::get_subgraph_function` crashing on unconnected input pin
* fixed infinite loop within `netlist_utils::get_subgraph_function` if the combinational logic contains a feedback path

## [3.1.3] - 2020-11-12 17:15:00+02:00 (urgency: medium)
* GUI code refactoring
* fix crash which occurs sometimes when invoking context menu from gate
	
## [3.1.2] - 2020-11-09 20:00:00+02:00 (urgency: medium)
* updated existing Python decorators and added new ones for the GUI API
* fix `get_subgraph_function` crashing when `output_net` is a nullptr

## [3.1.1] - 2020-11-05 16:18:00+02:00 (urgency: medium)
* fix bug that preventes module widget from showing the full tree in some instances
* fix missing cleaup of groupings when deleting gates, nets, or modules
* display Python Editor Widget by default, but hide its navigation bar
* do not display Log Widget by default anymore

## [3.1.0] - 2020-11-04 14:00:00+02:00 (urgency: medium)
* new (deterministic) layouter
  * tries to minimize wire length by placing connected gates closer to each other
  * combines multiple in-/outputs into one port if driven by the same net
  * cleaner overall appearance of the graph
* added Python tool to convert SkyWater gate libraries into a Liberty file that can be read by HAL
* fix bug with selection within grouping widget
* fix return code of GUI
* fixed problems when dragging around the docking bars of widgets
* Fix detect Library Path

## [3.0.2] - 2020-10-29 16:00:00+02:00 (urgency: medium)
* Focal Test Release 2

## [3.0.1] - 2020-10-29 13:00:00+02:00 (urgency: medium)
* Focal Test Release

## [3.0.0] - 2020-10-29 12:00:00+02:00 (urgency: medium)

### Added
* new selections details widget
  * hierarchical view of all items within the current selection
    * a single item can be selected within that view
    * selected item will be highlighted in a different color than the rest of the graph view selection
    * current selection can be moved to a module or grouping by using the respective buttons
  * details section
    * shows additional details for the item selected in the hierarchical view
    * sections can be collapsed and expanded
    * right-click context menu allows to copy strings or Python code to the clipboard and change module name, type, and port names
    * added number of gates, nets, and submodules to module details widget  
* new view manager widget
  * now presented in a table view additionally containing information about the time of creation of the view
  * views can be sorted by name and date
* gate library improvements
  * support for gate types with multi-bit pins
  * clear identification of clock pins
* new netlist features
  * module types and named module ports (parsed from netlist or set by user)
  * support for multi-driven nets (i.e., nets with more than one source)
* groupings
  * an interactive container for gates, nets, and modules
  * new groupings widget within the GUI to control groupings
    * allows to be filtered and sorted
    * groupings can be added to the current selection
  * members of a grouping are shown in a dedicated color within the graph view
* new graph view features
  * Python GUI API to control the graph view
  * zoom level of graph view can now be controlled by shortcuts
* netlist utilities 
  * added function to deep copy a netlist
  * added function to get the Boolean function of a subgraph
* state-of-the-art suite of benchmark netlists
* new simulator plugin
  * simulates (parts of) a netlist cycle accurate
  * can export to VCD
* added `clear_caches` to `Netlist`
* added `get_fan_in_endpoint`, `get_fan_out_endpoint`, `get_fan_in_endpoints` and `get_fan_out_endpoints` to class `Gate`

### CHANGED

* double-clicking module in module widget will open it in new view
* gate library manager now operates on file paths instead of library names
* liberty gate library parser now supports inout ports, clock statements and bus groups
* netlist parsers
  * VHDL and Verilog parser now use common intermediate structure for parsing
  * attribute parsing for Verilog parser
  * support for inout ports
* all parsers and writers have been moved into plugins
* code refactoring
  * "hal" namespace
  * TitleCase for classes
  * moved to std::filesystem internally
  * split Python API into multiple files
* massively expanded testing coverage
* the project is now affiliated with the Max Planck Institute for Security and Privacy
* reworked core ownership model
  * no more `shared_ptr`
  * clear ownership management via `unique_ptr` at necessary locations
  * instances passed via non-owning raw pointers
* updated/fixed the graph algorithms plugin by switching entirely to igraph
* Endpoints are now managed classes as well, owned by the respective nets and passed via pointers
* cleaned up the documentation in many parts of the core
* inverting a Boolean function is now done using `~` instead of `!`

### FIXED

* cone-view now works correctly for modules
* module widget selection is now more consistent with graph view
* inconsistencies with case insensitivity of the VHDL parser fixed
* delete option no longer shown for top module in right-click context menu
* some minor memory leaks in plugin management

### REMOVED

* list of nets and gates no longer present in module details widget

### DEPRECATED

* accessing the single source of a net is superseeded by accessing its source list to support multi-driven nets
* using `!`to invert a Boolean function

## [2.0.0] - 2019-12-19 22:00:00+02:00 (urgency: medium)
Note: This is an API breaking release.

* Heavily improved VHDL and Verilog parsers
* Updated CMake build system to use target-based configurations
* Changes to gate library system
  * Replaced BDDs with Boolean functions
  * Major changes to internal representation of gate types
  * Allows for differentiation between LUTs, flip-flops, latches and combinational gate types
  * Flip-flops and latches may now specify special sequential inputs such as enable, clock, set, and reset
  * Replaced JSON gate libraries with liberty files
* Simplified plugin system
* Included igraph library
* Major GUI revision
  * Added isolation view/cone view feature
  * New layouting system
  * Added support for hierarchization/modularization
* Tons of bug fixes and smaller issues ...

## [1.1.8] - 2019-09-19 14:23:59+02:00 (urgency: medium)

* Fix command line parsing
* Fix parse all_options from main
* Fixed set log option as command line parameter without stopping execution
* Fixed assign parsing for yosys verilog output

## [1.1.7] - 2019-08-26 10:41:58-04:00 (urgency: medium)

* Changed myself library to synopsys_nand_nor
* Fixed bug in bdd gen
* Added new gate library!
* Added python decorator to log function calls
* Changed question when hal file exists for better clarity
* Fixes #179. Parse yosys netlist correct. (#181)
* Set new vhdl parser as default, keep old one as fallback (#180)

## [1.1.6] - 2019-08-05 15:55:18+02:00 (urgency: medium)

* Fixed ubuntu ppa build

## [1.1.5] - 2019-08-05 15:14:35+02:00 (urgency: medium)

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

## [1.1.4] - 2019-07-15 14:53:49+02:00 (urgency: medium)

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

## [1.1.3] - 2019-07-03 10:10:12+02:00 (urgency: medium)

* Remember last opened path for python scripts
* Add multi file selection for python scripts

## [1.1.2] - 2019-06-27 14:21:03+02:00 (urgency: medium)

* Add hint to gate_libraries to README.md
* Changes to the global state can be handled via ContentManager

## [1.1.1] - 2019-06-27 14:21:03+02:00 (urgency: medium)

* Quickfix for Xilinx Simprim gate library

## [1.1.0] - 2019-06-27 12:28:36+02:00 (urgency: medium)

* Added Support for Xilinx Unisim gate library
* Added Support for Xilinx Simprim gate library
* Added Support for Synopsys 90nm gate library
* Added Support for GSCLIB 3.0 gate library
* Added Support for UMC 0.18 um gate library
* Added VHDL and Verilog parser fixes
* Fixeds issue #143
* Fixed static lint issues

## [1.0.24] - 2019-06-21 15:36:52+02:00 (urgency: medium)

* Testing new CI deployment

## [1.0.23] - 2019-06-21 10:36:36+02:00 (urgency: medium)

* Automated homebrew deployment
* Testing new CI deployment

## [1.0.22] - 2019-06-19 16:48:41+02:00 (urgency: medium)

* Fixed showing message box for error while opening file
* Fixed plugin generator for new EXPORT definition
* Fixed Visibility of shared Libraries (#121)
* Fixed bug in callback hook, optimized json output
* Added pybind for bdd evaluate
* Added python editor tabs in hal file manager
* Remove plugin manager callback hook while closing GUI. Fixes #88

## [1.0.21] - 2019-06-13 17:17:28+02:00 (urgency: medium)

* Fixes in python editor
* Fixes in core (Closes issue #98 and #96)

## [1.0.20] - 2019-06-06 11:56:36+02:00 (urgency: medium)

* Added python editor tabs to have multiple documents open at the same time

## [1.0.19] - 2019-05-30 21:45:18+02:00 (urgency: high)

* Remove dirty flag from genversion.py
* Remove kernel.h dependency in include/gate_decorator_system/decorators/gate_decorator_bdd.h for plugin repositoriese

## [1.0.18] - 2019-05-30 19:44:48+02:00 (urgency: high)

* Fix un-indent of single line failing on specific cursor positions
* fixed bug in parsers: added a GLOBAL_GND instead of a GLOBAL_VCC gate

## [1.0.17] - 2019-05-29 14:52:07+02:00 (urgency: medium)

* Fixed correct python binding of data container and added documentation for it. Closes #78
* Rename add_net and insert_gate to assign_net and assign_gate in module class. Closes #85
* Fix cursor position upon stdout and stderr text output
* Fix build instructions for macOS and import of OpenMP
* fixxed documentation
* added new features to gate_decorator_bdd:       - generate truth table  - evaluate bdd
* python context is no longer printed before each script execution
* Hardcode returns from python editor
* Display prompt before closing application
* Added first issue templates

## [1.0.16] - 2019-05-23 10:20:12+02:00 (urgency: high)

* Allow changing parent module (#84)
* navigation expands the file-name-index when gui starts (#83)
* Feature/selection history (#82)
* Move to spdlog 1.3.1 (#80)
* new tool for generating new plugins (#81)
* Fixed homebrew install command in README.md
* Fixes for out of source plugin builds

## [1.0.15] - 2019-05-16 14:42:58+02:00 (urgency: high)

* Fixed install include path of plugins
* Include information about homebrew tap in README.md

## [1.0.14] - 2019-05-16 10:41:27+02:00 (urgency: high)

* Fix install rpath on macOS
* Update genversion.py
* netlist creation now correctly fires module created event for topmodule
* Fix bad python binding for gates property of module
* Fix disabled arrow keys in navigation view

## [1.0.13] - 2019-05-13 12:53:46+02:00 (urgency: high)

* Fix auto-scrolling in the python editor
* Add indent/unindent for multi-line selections in python editor
* Fixed python shell to properly support global keyword

## [1.0.12] - 2019-05-13 12:53:46+02:00 (urgency: high)

* Fixed global and local in python context
* Cleanup .gitignore and .gitattributes
* Added .brew_home to .gitignore for homebrew builds
* Move from gitter to discord in README.md

## [1.0.11] - 2019-04-30 19:46:40+02:00 (urgency: high)

* Minor fixes

## [1.0.10] - 2019-04-30 19:46:40+02:00 (urgency: high)

* Add python editor save-as button and fix load/save logic (#55)
* Add shift-tab unindent in python editor (#51)
* fixed readme not showing screenshot
* updated readme, fixed gui log not showing multiple spaces, added python bindings for quine mccluskey plugin

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
[Unreleased]: https://github.com/emsec/hal/compare/v3.1.5...HEAD
[3.1.5]: https://github.com/emsec/hal/compare/v3.1.4...v3.1.5
[3.1.4]: https://github.com/emsec/hal/compare/v3.1.3...v3.1.4
[3.1.3]: https://github.com/emsec/hal/compare/v3.1.2...v3.1.3
[3.1.2]: https://github.com/emsec/hal/compare/v3.1.1...v3.1.2
[3.1.1]: https://github.com/emsec/hal/compare/v3.1.0...v3.1.1
[3.1.0]: https://github.com/emsec/hal/compare/v3.0.2...v3.1.0
[3.0.2]: https://github.com/emsec/hal/compare/v3.0.1...v3.0.2
[3.0.1]: https://github.com/emsec/hal/compare/v3.0.0...v3.0.1
[3.0.0]: https://github.com/emsec/hal/compare/v2.0.0...v3.0.0
[2.0.0]: https://github.com/emsec/hal/compare/v1.1.8...v2.0.0
[1.1.8]: https://github.com/emsec/hal/compare/v1.1.7...v1.1.8
[1.1.7]: https://github.com/emsec/hal/compare/v1.1.6...v1.1.7
[1.1.6]: https://github.com/emsec/hal/compare/v1.1.5...v1.1.6
[1.1.5]: https://github.com/emsec/hal/compare/v1.1.4...v1.1.5
[1.1.4]: https://github.com/emsec/hal/compare/v1.1.3...v1.1.4
[1.1.3]: https://github.com/emsec/hal/compare/v1.1.2...v1.1.3
[1.1.2]: https://github.com/emsec/hal/compare/v1.1.1...v1.1.2
[1.1.1]: https://github.com/emsec/hal/compare/v1.1.0...v1.1.1
[1.1.0]: https://github.com/emsec/hal/compare/v1.0.8...v1.1.0
[1.0.24]: https://github.com/emsec/hal/compare/v1.0.23...v1.0.24
[1.0.23]: https://github.com/emsec/hal/compare/v1.0.22...v1.0.23
[1.0.22]: https://github.com/emsec/hal/compare/v1.0.21...v1.0.22
[1.0.21]: https://github.com/emsec/hal/compare/v1.0.20...v1.0.21
[1.0.20]: https://github.com/emsec/hal/compare/v1.0.19...v1.0.20
[1.0.19]: https://github.com/emsec/hal/compare/v1.0.18...v1.0.19
[1.0.18]: https://github.com/emsec/hal/compare/v1.0.17...v1.0.18
[1.0.17]: https://github.com/emsec/hal/compare/v1.0.16...v1.0.17
[1.0.16]: https://github.com/emsec/hal/compare/v1.0.15...v1.0.16
[1.0.15]: https://github.com/emsec/hal/compare/v1.0.14...v1.0.15
[1.0.14]: https://github.com/emsec/hal/compare/v1.0.13...v1.0.14
[1.0.13]: https://github.com/emsec/hal/compare/v1.0.12...v1.0.13
[1.0.12]: https://github.com/emsec/hal/compare/v1.0.11...v1.0.12
[1.0.11]: https://github.com/emsec/hal/compare/v1.0.10...v1.0.11
[1.0.10]: https://github.com/emsec/hal/compare/v1.0.8...v1.0.10
[1.0.8]: https://github.com/emsec/hal/compare/v1.0.7...v1.0.8
[1.0.7]: https://github.com/emsec/hal/compare/v1.0.6...v1.0.7
[1.0.6]: https://github.com/emsec/hal/compare/v1.0.5...v1.0.6
[1.0.5]: https://github.com/emsec/hal/compare/v1.0.4...v1.0.5
[1.0.4]: https://github.com/emsec/hal/compare/v1.0.3...v1.0.4
[1.0.3]: https://github.com/emsec/hal/compare/v1.0.2...v1.0.3
[1.0.2]: https://github.com/emsec/hal/compare/v1.0.1...v1.0.2
[1.0.1]: https://github.com/emsec/hal/compare/v1.0.0...v1.0.1
[1.0.0]: https://github.com/emsec/hal/releases/tag/v1.0.0
