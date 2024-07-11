
import sys, os

try:
  user = os.getlogin()
  if user == "simon":
    hal_base_path = "/home/simon/projects/hal/"
except:
  hal_base_path = "/home/anna/hal/"
  pass



#some necessary configuration:
sys.path.append(hal_base_path + "build_dbg/lib/") #this is where your hal python lib is located
os.environ["HAL_BASE_PATH"] = hal_base_path + "build_dbg/" # hal base path

import hal_py
gate_library_path = hal_base_path + "plugins/gate_libraries/definitions/XILINX_UNISIM.hgl"


#### CPU ####
netlist_to_read = hal_base_path + "/plugins/bachelorarbeit/xilinx_netlists/cpu/edge/edge/netlist_flatten_full_fsm_auto_dsp_none/netlist_flatten_full_fsm_auto_dsp_none.v"

#initialize HAL
hal_py.plugin_manager.load_all_plugins()

#read netlist
netlist = hal_py.NetlistFactory.load_netlist(netlist_to_read, gate_library_path)

from hal_plugins import module_identification

pl = hal_py.plugin_manager.get_plugin_instance("module_identification")


def detect_carry_chains(nl: netlist):
    #nl_info = NetlistInformation.instance()
    carry_chains = []

    # retrieve all carry gates
    carry_gates = [g for g in nl.get_gates() if g.get_type().has_property(hal_py.GateTypeProperty.c_carry)]
    carry_gates_set = set(carry_gates)

    # collect carry chains until all carry gates have been analyzed
    while len(carry_gates_set) > 0:
        current_gate = carry_gates_set.pop()
        carry_type = current_gate.get_type()
        
        #print("current_gate",current_gate.get_name())
        #print("carry_type",carry_type)
        
        # get carry chains by defining appropriate filter function
        carry_chain = hal_py.NetlistUtils.get_gate_chain(current_gate, [carry_type.get_pin_by_name("CI")], [carry_type.get_pin_by_name("CO(3)")])

        # remove shift register gates from candidate set
        carry_gates_set -= set(carry_chain)
        #print("carry_chain", carry_chain)
        
        # only consider carry chains with more than 3 gates for now
        #print("carry_chain length:", len(carry_chain))

        if len(carry_chain) >= 3:
            #print(f"carry_gate: {carry_chain[0].get_name()}")
            carry_chains.append(carry_chain)

            #nl_info.add_structure(NetlistInformation.StructureType.CARRY_CHAIN, carry_chain)

    return carry_chains
    
chains = detect_carry_chains(netlist)
print(len(chains), "Carry chains found.")

#for c in chains:
#    pl.test_plugin(c)

#pl.test_plugin(chains[0])
#pl.test_plugin(netlist.get_top_module().get_gates())
#unload everything hal related
hal_py.plugin_manager.unload_all_plugins()