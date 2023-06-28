from hal_plugins import netlist_preprocessing

gate_id = 102
target_gl_path = "/home/simon/projects/hal/plugins/netlist_preprocessing/gate_libs/basic_hal.hgl"
genlib_path = "/tmp/resynthesize_boolean_functions_with_abc/new_gate_library.genlib"

g = netlist.get_gate_by_id(gate_id)
target_gl = hal_py.GateLibraryManager.load(target_gl_path)

netlist_preprocessing.NetlistPreprocessingPlugin.resynthesize_gate(netlist, g, target_gl, genlib_path, True)