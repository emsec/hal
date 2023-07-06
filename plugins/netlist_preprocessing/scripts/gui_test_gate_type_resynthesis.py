from hal_plugins import netlist_preprocessing


gate_types = netlist.gate_library.get_gate_types(lambda gt : gt.has_property(hal_py.GateTypeProperty.c_lut)) 
target_gl_path = "/home/simon/projects/hal/plugins/gate_libraries/definitions/helper_libs/aoixm_hal_i4.hgl"

target_gl = hal_py.GateLibraryManager.load(target_gl_path)

netlist_preprocessing.NetlistPreprocessingPlugin.resynthesize_gates_of_type(netlist, list(gate_types.values()), target_gl)