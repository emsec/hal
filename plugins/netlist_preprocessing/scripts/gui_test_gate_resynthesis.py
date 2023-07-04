from hal_plugins import netlist_preprocessing

gate_id = 353
target_gl_path = "/home/simon/projects/hal/plugins/gate_libraries/definitions/aoim_hal_i4.hgl"
genlib_path = "/tmp/resynthesize_boolean_functions_with_abc/new_gate_library.genlib"

g = netlist.get_gate_by_id(gate_id)

target_gl = hal_py.GateLibraryManager.load(target_gl_path)
hal_py.GateLibraryManager.save(genlib_path, target_gl, True)


netlist_preprocessing.NetlistPreprocessingPlugin.resynthesize_gate(netlist, g, target_gl, genlib_path, True)