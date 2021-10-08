# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
* **WARNING:** this release partially breaks the `GateType` API, please make sure to adjust your code accordingly.
* added user action system to enable recording and reverting actions within the GUI
  * moved most GUI actions to the new user action system, including interactions with the graph view and view management
  * user actions can be recorded and exported as a macro file allowing easier debugging and crash reporting
  * recording of the user actions is automatically dumped on crash
  * users can now revert actions executed within the GUI
* rebuild the settings system from scratch to allow for easier integration of new settings
  * theme setting (and many others) can now be changed during the session
  * added a warning when attempting to assign a keybinding that is already in use
  * added incremental search for settings
  * added setting to enable/disable extra window to list all executed instances from UserAction
* added new `Move to module ...` dialog 
  * allows to create new module or select from existing modules
  * existing modules can be selected from a table, tree-view, or using a module picker within the graph view
  * enables searching for existing modules
  * added cursor to indicate that user is in module pick mode
* added successor / predecessor utilities to gate and module context menu
  * shortest path between two gates can be highlighted or added to current view 
  * predecessors or successors can be highlighted or added to the current view up to a user-specified depth
  * different grouping colors can be assigned depending on the distance from the origin
  * common predecessors or successors can be found and added to view
* improved layouter
  * show progress bar during layouting
  * allow user to abort layouting
  * preserve graph view location of gate when moving it to new module
  * fixed routing errors for complex cable swaps
  * fixed multiple connections of a single net to the same gate not being shown properly
  * fixed incorrect placement of new gates and modules in cone view when navigating starting from a net
* added generic SMT solver interface.
  * added translation from `BooleanFunction` to SMT-LIB.
  * added `BooleanFunction::Node` data structure to extend functionality to generic ASTs.
  * added support for z3 and boolector SMT solvers.
  * added cpp-subprocess library to handle communication with SMT solver.
  * added Boost Spirit x3 library to generate grammar-based parser from SMT-LIB models to C++ data structures.
* improved handling of properties for special gate types such as LUTs and FFs.
  * properties that only apply to special gate types have been moved out of the `GateType` class and into a designated `GateTypeComponent`
  * added functions to retrieve a gate type's components based on some filter condition
  * added special components dealing with RAM properties 
* improved netlist parsers
  * split VHDL and Verilog parsers into two independent plugins
  * netlist parsers now take the path to the netlist file as input instead of a `std::stringstream`
  * added support for `Z` and `X` assignments to Verilog and VHDL parsers
  * added `tri` as a synonym for `wire` to the Verilog parser
  * fixed netlist parsers assigning wrong order of inputs for some multi-bit signals
* improved netlist writers
  * netlist writers now take the output path as input instead of a `std::stringstream`
  * removed broken VHDL writer (will not be supported until further notice)
  * entirely new Verilog writer that respects module hierarchies
  * added GEXF netlist writer, e.g., for netlist analysis within Gephi
* expanded `netlist_utils`
  * added function `get_common_inputs` to get inputs that are common across multiple gates
  * added function `replace_gate` to replace a gate with an instance of another gate type
  * added function `get_gate_chain` and `get_complex_gate_chain` to find gates that are arranged in a chain
  * added function `get_shortest_path` to compute the shortest path between two gates
  * added function `get_next_gates` to get the predecessors or successors of a gate up to a user-specified depth
* `dataflow_analysis` plugin
  * can now take groups of flip-flops as input that should not be touched during analysis
  * this is meant to aid the dataflow analysis by passing control registeres identified beforehand, which prevents them from being merged into the datapath
* new internal event system 
  * binds event handlers to a netlist instance
  * facilitates listening to the events of selected netlists only
* improved search
  * all searchbars now come with options for "Exact Match" and "Case Sensitive" search, as well as a "Clear" button
  * added search icons to the Python editor and the module widget
  * disabled the search filter whenever the searchbar is not visible within a widget
* miscellaneous API changes and additions
  * added function `is_top_module` to class `Module` to determine whether a module is the top module
  * added function `get_nets` to class `Module` to get all nets that are connected to any of the gates or submodules of a module
  * added functions `is_gnd_net` and `is_vcc_net` to class `Net` to determine whether a net is connected to GND or VCC
  * added functions `operator==` and `operator!=` to classes `Netlist`, `Gate`, `Net`, `Module`, and `Endpoint`
  * added Python bindings for `netlist_serializer`
* miscellaneous GUI changes and additions
  * added `Save As...` option to save `.hal` files under a different name
  * added `Export ...` menu to export the netlist using any of the registered netlist writers
  * added `Remove from view` action to context menu for gates and modules
  * added context menu options to close multiple view tabs at once
  * added an indicator showing whether views have been modified
  * added HAL version number to the info shown in `About`
  * added `Fold parent module` option to module context menu
  * when trying to create a view for a module that is already associated with an (unchanged) view, the existing view is activated instead of creating a new view
* bugfixes
  * fixed selection details not being updated immediately when renaming or changing a type
  * fixed navigation bug where ports and nets did not match for modules
  * fixed list of navigation targets containing duplicates and/or loops
  * fixed drag'n'drop bug related to negative coordinates
  * fixed liberty parser aborting on unknown `pg_type`
  * fixed stylesheets
  * fixed improper handling of GND and VCC nets within the `solve_fsm` plugin
  * fixed module port names not being freed when reassigned a new name
  * fixed segfault when no VCC or GND gate is present within a netlist

## [3.2.6] - 2021-03-03 09:30:00+02:00 (urgency: medium)
* added support for multiple properties (formerly refered to as "base type") for a single instance of class `GateType`
  * renamed enum `GateType::BaseType` to `GateTypeProperty` and moved it out of class `GateType`
  * added function `has_property` to class `GateType`
  * changed function `get_base_type` of class `GateType` to `get_properties`
  * changed HGL gate libraries to support multiple properties
  * changed function `create_gate_type` of class `GateLibrary` to support multiple properties
* added `sequential`, `power`, `ground`, `buffer`, `mux`, and `carry` gate type properties to enum `GateTypeProperty`
* moved enums `PinType` and `PinDirection` from class `GateType` into global scope
* added `get_path` to `netlist_utils` to retrieve all gates on the predecessor/successor path from a start gate/net to gates of a specified property
* made `optimize_constants` of class `BooleanFunction` publicly accessible
* refined buffer removal in `netlist_utils::remove_buffers` to take constant `0` and `1` inputs into account
* added high-impedance state `Z` to class `BooleanFunction` and added basic support to `evaluate`
* cleaned up and refined some logger outputs and streamlined log channel names
* disabled extended logging again
* changes to `z3_utils` (WIP)
* fixed crash related to GraphicsScene destructor
* fixed overlapping gates in cone view (and subsequent segfault) by suppressing gate coordinates when adding to cone view
* fixed `get_gate_by_id` and `get_gates` of class `Netlist` returning only gates contained within one of its modules (causing a GUI crash upon deleting gates from a module)
* fixed nets of old module not updating when moving gate from one module to another

## [3.2.5] - 2021-01-29 13:15:00+02:00 (urgency: medium)
* **WARNING:** temporarily enabled extended logging (includes taking screenshots) for university course purposes. Note that no data leaves your machine unless you actively provide it to us.
* views get persisted to .halv file and are restored if the file is found on disk
* fixed bug in `boolean_influence` plugin causing problems on global inputs
* fixed gate and net details widget not showing full list of pins for large gates

## [3.2.4] - 2021-01-23 15:30:00+02:00 (urgency: medium)
* added plugin `boolean_influence` that enables calculation of the boolean influence for each FF depending on the predecessing FFs
* extended the `z3_utils` plugin with a `z3Wrapper` class, which holds exactly one `z3::expr` and the corresponding `z3::context`
* removed the code coverage checks from the macOS pipeline and added test command, so the macOS pipeline will work again properly
* fixed a bug in DANA, where sometimes the net names were output in the DANA results instead of the gate names

## [3.2.3] - 2021-01-18 18:30:00+02:00 (urgency: medium)
* fixed `z3_utils` plugin being disabled by default causing linking errors
* fixed `load_initial_values` and `load_initial_values_from_netlist` assigning values to potentially non-existing nets

## [3.2.2] - 2021-01-16 14:40:00+02:00 (urgency: medium)
* refactored gate library handling
  * separated gate library manager from gate library parser interface
  * added gate library writer interface to enable writing out gate library files
* extended and refactored gate library functionality
  * added `create_gate_type` to class `GateLibrary` to enable gate type creation from Python
  * added `mark_vcc_gate_type` and `mark_gnd_gate_type` to class `GateLibrary` to enable marking gate types as power or ground connections
  * added `get_gate_type_by_name` and `contains_gate_type_by_name` to class `GateLibrary`
  * added pin types and respective functions to `GateType` to enable assigning special-purpose pins
  * added `get_gate_library` to class `GateType`
  * added base types `ram`, `dsp`, and `io`
  * merged input and output pin groups to simplify pin group handling
  * removed `add_gate_type` function from class `GateLibrary`
  * removed `GateTypeSequential` and `GateTypeLut` classes and moved their functionality into class `GateType`
  * renamed some functions to have shorter and more understandable names
* added new gate library format: "HAL Gate Library" (HGL)
  * supports assignment of pin types to gate types
  * added parser for HGL (`.hgl`) files
  * added writer for HGL (`.hgl`) files
* refactored liberty gate library parser
  * added parsing of power and ground pins (`pg_pin`) to Liberty parser
* added more netlist utility functions
  * added `get_nets_at_pins` to retrieve nets that are connected to a vector of pins
  * added `remove_buffers` to remove buffer gates from a netlist
  * added `remove_unused_lut_endpoints` to remove unused LUT fan-in endpoints
  * added `rename_luts_according_to_function` to rename LUTs depending on the Boolean function they implement
* added `to_z3` to class `BooleanFunction` to translate a Boolean function into a z3 expression
* added **highly experimental** `solve_fsm` plugin for FSM verification using z3
* added `z3_utils` plugin to provide common z3 functions to all other plugins
* improved layouter uses location information from gate API
* switched from float gate coordinates to integer ones
* the netlist simulator VCD writer now optionally takes a set of target nets to write to VCD
* fixed `add_boolean_function` of class `Gate` assigning wrong functions to LUTs
* fixed wrong Python binding for property `gate_library` of class `Netlist`
* fixed netlist simulator segfaulting when an output pin of a FF remains unconnected
* fixed optimization of Boolean functions sometimes producing wrong or non-optimal results
* fixed `netlist_utils::get_subgraph_function` returning wrong results if input pins without relevance for the Boolean function remained unconnected
* fixed layouter not showing connections if things change within submodules by adding additional test whether removing or adding a gate/module requires context update

## [3.1.11] - 2021-01-03 11:35:00+02:00 (urgency: medium)
* added **highly experimental** way to close and reopen netlists at runtime
  * known issue: the Python context does not change properly between netlists
* added creation of backups for Python files created within the editor outside of the `.hal` file
* added user prompt when detecting Python file backups after a crash
* fixed crash when moving top module upward
* fixed module cache inconsistencies causing bad memory allocs in the GUI when deleting nets or endpoints

## [3.1.10] - 2020-12-18 14:00:00+02:00 (urgency: medium)
* added `SB_GB_IO`, `SB_GB`, `SB_I2C`, `SB_SPI`, `SB_HFOSC`, and `SB_LFOSC` gate types to `ICE40ULTRA` gate library
* added jumping to gates, nets, and modules from the selection details widget when double-clicking the respective item in the list or using the context menu
* added isolating gates and modules in new views from the selection details widget using the conext menu
* changed layouter to only be called after fully executing a Python script
* improved performance of selection details widget
* fixed layouter flaw that junctions could not be drawn if in- and outgoing wires were swapped
* fixed parsing of bit order for INIT strings in FPGA gate libraries
* fixed GUI segfaulting when deleting wires without unfolding the containing module
* fixed occasional null pointer exceptions on GUI startup
* fixed GUI listening to signals from netlists other than the one loaded within the GUI
* fixed graph view resizing when navigating using the arrow keys or the GUI API

## [3.1.9] - 2020-12-02 13:00:00+02:00 (urgency: medium)
* added `NetlistSimulator::load_initial_value_from_netlist` to load the initial value from the netlist
* changed `NetlistSimulator::load_initial_values` to load a user-specified value instead of reading it from the netlist
* fixed layouter not showing gates within modules for netlists with coordinates

## [3.1.8] - 2020-12-02 10:00:00+02:00 (urgency: medium)
* added optional placement of gates and modules according to COORDINATE info parsed from netlist file
* fixed bug in recently introduced placement according to parsed coordinates, map needs clear if unused
* fixed crash caused by NodeBoxes hash not completely emptied on clear
* fixed mistakes in `module` Python documentation
* fixed missing include in `graph_navigation_widget.cpp`
* fixed missing initialization in `NetlistSimulator::generate_vcd`
* fixed missing input label when separated net is connected to more than one port

## [3.1.7] - 2020-11-26 14:30:00+02:00 (urgency: medium)
* **WARNING:** the function signature of `netlist_utilities::get_subgraph_function` has changed
* added cycle detection to `netlist_utilities::get_subgraph_function` again
* added Lattice ICE gate library
* improved access to layout class NodeBox for GUI C++ developer
* improved Python plugin, added command line option `--python-script` and `--python-args`
* improved appearance and programmcode for arrow-key navigation
* removed all Boost and Kyoto references
* fixed crashes due to missing entries in coordinate system in views with unconnected boxes
* fixed crashes when navigating from global inputs connected to multiple boxes
* fixed bad alignment between net endpoint and port label for gates with ports not connected to nets
* fixed log output got incorrectly supressed in case command line arguments were solely directed at plugins
* fixed wrong layouting of endpoints in GUI in the presence of unconnected pins/ports
* fixed wrong calculation of minimum size of the dockbars
* fixed bottom dockbar not reappearing when dragging removing every widget from it

## [3.1.6] - 2020-11-22 19:30:00+02:00 (urgency: medium)
* added `netlist_utilities::get_subgraph_function` variants with and without cache
* added `netlist_utilities::get_next_sequential_gates` variants with and without cache
* added tests for `netlist_utilities::get_next_sequential_gates`
* added python bindings for gate_library_manager
* cleaned up `DataContainer`
* removed hidden internal cache from `netlist_utilities::get_subgraph_function`
* removed `netlist_utilities::get_subgraph_function` cycle detection due to unintended behavior
* fixed netlist parsers wrongly handling escapings within strings
* fixed `netlist_utilities::copy_netlist` did not copy `DataContainer` contents
* fixed netlist pybind handling of netlists that occasionally led to double-free segfaults
* fixed segfault in `BooleanFunction::from_string` when providing partial variable names

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
[Unreleased]: https://github.com/emsec/hal/compare/v3.2.6...HEAD
[3.2.6]: https://github.com/emsec/hal/compare/v3.2.5...v3.2.6
[3.2.5]: https://github.com/emsec/hal/compare/v3.2.4...v3.2.5
[3.2.4]: https://github.com/emsec/hal/compare/v3.2.3...v3.2.4
[3.2.3]: https://github.com/emsec/hal/compare/v3.2.2...v3.2.3
[3.2.2]: https://github.com/emsec/hal/compare/v3.1.11...v3.2.2
[3.1.11]: https://github.com/emsec/hal/compare/v3.1.10...v3.1.11
[3.1.10]: https://github.com/emsec/hal/compare/v3.1.9...v3.1.10
[3.1.9]: https://github.com/emsec/hal/compare/v3.1.8...v3.1.9
[3.1.8]: https://github.com/emsec/hal/compare/v3.1.7...v3.1.8
[3.1.7]: https://github.com/emsec/hal/compare/v3.1.6...v3.1.7
[3.1.6]: https://github.com/emsec/hal/compare/v3.1.5...v3.1.6
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
