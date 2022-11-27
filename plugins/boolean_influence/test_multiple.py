import sys, os

hal_base_path = "/home/simon/projects/hal/"
dir_path = "/home/simon/playground/boolean_inf_test/netlists"
gate_library_path = "/home/simon/projects/hal/plugins/gate_libraries/definitions/lsi_10k.hgl"

sys.path.append(hal_base_path + "build/lib/") #this is where your hal python lib is located
os.environ["HAL_BASE_PATH"] = hal_base_path + "build" # hal base path

import hal_py

#initialize HAL
hal_py.plugin_manager.load_all_plugins()

from hal_plugins import boolean_influence

pl = hal_py.plugin_manager.get_plugin_instance("boolean_influence")

import glob

files = glob.glob(dir_path + '/**/netlist/*.v', recursive=True)

print(files)
print(len(files))

for file in files:
    print("Getting influences in netlist {}".format(file))

    netlist = hal_py.NetlistFactory.load_netlist(file, gate_library_path)

    for gate in netlist.gates:
        if gate.type.has_property(hal_py.GateTypeProperty.ff):
            print("Gathering influences for gate {}".format(gate.id))   
            inf = pl.get_boolean_influences_of_gate(gate)


#unload everything hal related
hal_py.plugin_manager.unload_all_plugins()
