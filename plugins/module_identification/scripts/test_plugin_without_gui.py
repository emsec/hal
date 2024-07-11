#!/usr/bin/env python3
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
#gate_library_path = hal_base_path + "plugins/gate_libraries/definitions/NangateOpenCellLibrary.hgl"
gate_library_path = hal_base_path + "plugins/gate_libraries/definitions/XILINX_UNISIM.hgl"

#ADDER#
#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/pattern_lib/addition_32bit/addition_32bit_netlist.v"
#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/pattern_lib/addition_carry_32bit/addition_carry_32bit_netlist.v"
#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/pattern_lib/addition_carry_enable_32bit/addition_carry_enable_32bit_netlist.v"
#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/pattern_lib/addition_enable_32bit/addition_enable_32bit_netlist.v"
#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/pattern_lib/addition_two_enable_32bit/addition_two_enable_32bit_netlist.v"
#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/pattern_lib/addition_sign_extened_32bit/addition_sign_extened_32bit_netlist.v"

#----
#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/pattern_lib/addition_carry_sign_extened_32bit/addition_carry_sign_extened_32bit_netlist.v"
#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/pattern_lib/addition_carry_sign_extened_enable_32bit/addition_carry_sign_extened_enable_32bit_netlist.v"
#----

#COUNTER
#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/pattern_lib/counter_32bit/counter_32bit_netlist.v"
#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/pattern_lib/counter_enable_32bit/counter_enable_32bit_netlist.v" 
#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/pattern_lib/counter_reset_32bit/counter_reset_32bit_netlist.v"
#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/pattern_lib/counter_enable_reset_32bit/counter_enable_reset_32bit_netlist.v" 

#SUBSTRACTION
#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/pattern_lib/subtraction_32bit/subtraction_32bit_netlist.v"
#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/pattern_lib/subtraction_enable_32bit/subtraction_enable_32bit_netlist.v"

#XILINX Arithmetic
# ADDER
#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/xilinx_netlists/arithmetic/addition/addition/addition/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32.v"
#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/xilinx_netlists/arithmetic/addition/addition_carry/addition_carry/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32.v"
#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/xilinx_netlists/arithmetic/addition/addition_carry_enable/addition_carry_enable/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32.v"
#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/xilinx_netlists/arithmetic/addition/addition_carry_sign_extended/addition_carry_sign_extended/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32_top_WIDTH_HALF_16/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32_top_WIDTH_HALF_16.v"
#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/xilinx_netlists/arithmetic/addition/addition_carry_sign_extended_enable/addition_carry_sign_extended_enable/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32_top_WIDTH_HALF_16/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32_top_WIDTH_HALF_16.v"
#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/xilinx_netlists/arithmetic/addition/addition_carry_sign_extended_two_enable/addition_carry_sign_extended_two_enable/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32_top_WIDTH_HALF_16/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32_top_WIDTH_HALF_16.v"


#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/xilinx_netlists/arithmetic/addition/addition_enable/addition_enable/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32.v"
#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/xilinx_netlists/arithmetic/addition/addition_sign_extened/addition_sign_extended/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32_top_WIDTH_HALF_16/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32_top_WIDTH_HALF_16.v"
#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/xilinx_netlists/arithmetic/addition/addition_two_enable/addition_two_enable/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32.v"

#Increment
#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/xilinx_netlists/arithmetic/increment/increment_enable/increment_enable/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32_top_INC_1/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32_top_INC_1.v"
#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/xilinx_netlists/arithmetic/increment/increment_enable_reset/increment_enable_reset/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32_top_INC_1/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32_top_INC_1.v"
#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/xilinx_netlists/arithmetic/increment/increment_reset/increment_reset/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32_top_INC_1/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32_top_INC_1.v"

#Substraction
netlist_to_read = hal_base_path + "plugins/bachelorarbeit/xilinx_netlists/arithmetic/subtraction/subtraction/subtraction/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32.v"

#### CPU ####
#netlist_to_read = hal_base_path + "/plugins/bachelorarbeit/xilinx_netlists/cpu/edge/edge/netlist_flatten_full_fsm_auto_dsp_none/netlist_flatten_full_fsm_auto_dsp_none.v"

#initialize HAL
hal_py.plugin_manager.load_all_plugins()

#read netlist
netlist = hal_py.NetlistFactory.load_netlist(netlist_to_read, gate_library_path)
from hal_plugins import module_identification


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

        if len(carry_chain) >= 2:
            #print(f"carry_gate: {carry_chain[0].get_name()}")
            carry_chains.append(carry_chain)

            #nl_info.add_structure(NetlistInformation.StructureType.CARRY_CHAIN, carry_chain)

    return carry_chains
    

pl = hal_py.plugin_manager.get_plugin_instance("module_identification")
pl.test_plugin(netlist.get_top_module().get_gates())

 
'''
test_gates=[]
test_ids = [6794, 6743, 6863, 6559, 6677, 6670, 6665, 6662, 6659, 8816, 8815, 8814, 7401, 7400, 7399, 7396, 8158, 8157, 8156, 8155, 8137, 8136, 8135, 8134, 8117, 8116, 8115, 5802, 5797]

for i in test_ids:
   test_gates.append(netlist.get_gate_by_id(i))

pl.test_plugin(test_gates)

'''

'''
chains = detect_carry_chains(netlist)
print(len(chains), "Carry chains found.")

def get_s_predecessors(gate):
    result = []
    eps = []
    endpoints = g.get_fan_in_endpoints()
    for e in endpoints:
        if e.get_pin().get_name().startswith("S"):
            src = e.get_net().get_sources()
            eps.extend(src)
            #print("src: ",src)
    for ep in eps:
        if ep.get_gate().get_type().has_property(hal_py.GateTypeProperty.c_lut):
            result.append(ep.get_gate())
    return result

with open("/home/anna/hal/plugins/bachelorarbeit/scripts/results.txt", "a") as myfile:
    myfile.write("Modules identified: \n")




for c in chains:
    for g in c:
        if g.get_name() == "GND":
            c.remove(g) 
        #else:
        #    c.extend(get_s_predecessors(g))
    tmp = ""
    for g in c:
        tmp += '{}, '.format(g.get_id())
    with open("/home/anna/hal/plugins/bachelorarbeit/scripts/results.txt", "a") as myfile:
        myfile.write(tmp)
        myfile.write("\n\n")      
    pl.test_plugin(c)  
        
'''  







#unload everything hal related
hal_py.plugin_manager.unload_all_plugins()
