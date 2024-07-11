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
gate_library_path = hal_base_path + "plugins/gate_libraries/definitions/NangateOpenCellLibrary.hgl"

#Example1 Adder
#ADDER 
#netlist_to_read = hal_base_path + "plugins/bachelorarbeit/pattern_lib/addition_32bit/addition_32bit_netlist.v"

# example 2 addition carry
netlist_to_read = hal_base_path + "plugins/bachelorarbeit/xilinx_netlists/arithmetic/addition/addition_carry/addition_carry/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32/netlist_flatten_full_fsm_auto_dsp_none_top_WIDTH_32.v"

#initialize HAL
hal_py.plugin_manager.load_all_plugins()

#read netlist
netlist = hal_py.NetlistFactory.load_netlist(netlist_to_read, gate_library_path)
from hal_plugins import module_identification

# example1
# ORIGINAL
#out_ids = [287,298,309,312,313,314,315,316,317,318,288,289,290,291,292,293,294,295,296,297,299,300,301,302,303,304,305,306,307,308,310,311]
#a_ids = []
#b_ids = []
#a_ids.extend(range(3,35))
#b_ids.extend(range(35,67))


#Testing Substraction candidate:
#Operand A: 3 4 37 38 7 8 41 42 43 44 45 46 15 16 17 18 51 52 53 54 55 56 57 26 27 28 29 62 63 64 33 34 
#a_ids = [3, 4, 37, 38, 7, 8, 41, 42, 43, 44, 45, 46, 15, 16, 17, 18, 51, 52, 53, 54, 55, 56, 57, 26, 27, 28, 29, 62, 63, 64, 33, 34]
#Operand B: 35 36 5 6 39 40 9 10 11 12 13 14 47 48 49 50 19 20 21 22 23 24 25 58 59 60 61 30 31 32 65 66 
#b_ids = [35, 36, 5, 6, 39, 40, 9, 10, 11, 12, 13, 14, 47, 48, 49, 50, 19, 20, 21, 22, 23, 24, 25, 58, 59, 60, 61, 30, 31, 32, 65, 66]
#Output: 289 300 311 314 315 316 317 318 319 320 290 291 292 293 294 295 296 297 298 299 301 302 303 304 305 306 307 308 309 310 312 313 
#out_ids = [289, 300, 311, 314, 315, 316, 317, 318, 319, 320, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 312, 313]
#Successful check for SUBSTRACTION

# Example 2
a_ids = []
#Operand A: a(0) a(1) b(2) b(3) a(4) a(5) b(6) b(7) b(8) b(9) b(10) b(11) a(12) a(13) a(14) a(15) b(16) b(17) b(18) b(19) b(20) b(21) b(22) a(23) a(24) a(25) a(26) b(27) b(28) b(29) a(30) a(31) 
#Operand B: b(0) b(1) a(2) a(3) b(4) b(5) a(6) a(7) a(8) a(9) a(10) a(11) b(12) b(13) b(14) b(15) a(16) a(17) a(18) a(19) a(20) a(21) a(22) b(23) b(24) b(25) b(26) a(27) a(28) a(29) b(30) b(31) 
#Output: out(0) out(1) out(2) out(3) out(4) out(5) out(6) out(7) out(8) out(9) out(10) out(11) out(12) out(13) out(14) out(15) out(16) out(17) out(18) out(19) out(20) out(21) out(22) out(23) out(24) out(25) out(26) out(27) out(28) out(29) out(30) out(32) out(31) 

print(len(a_ids),":", a_ids)
print(len(b_ids),":",b_ids)
print(len(out_ids),":",out_ids)

subgraph = netlist.gates

bf_out = hal_py.NetlistUtils.get_subgraph_function(netlist.get_net_by_id(out_ids[0]), subgraph)
print(bf_out)
for i in range(1, len(out_ids)):
    bf_i = hal_py.NetlistUtils.get_subgraph_function(netlist.get_net_by_id(out_ids[i]), subgraph)
    bf_out = hal_py.BooleanFunction.Concat(bf_i, bf_out, i+1)
    
bf_a = hal_py.BooleanFunctionNetDecorator(netlist.get_net_by_id(a_ids[0])).get_boolean_variable()
for i in range(1, len(a_ids)):
    bf_i = hal_py.BooleanFunctionNetDecorator(netlist.get_net_by_id(a_ids[i])).get_boolean_variable()
    bf_a = hal_py.BooleanFunction.Concat(bf_i, bf_a, i+1)
    
bf_b = hal_py.BooleanFunctionNetDecorator(netlist.get_net_by_id(b_ids[0])).get_boolean_variable()
for i in range(1, len(b_ids)):
    bf_i = hal_py.BooleanFunctionNetDecorator(netlist.get_net_by_id(b_ids[i])).get_boolean_variable()
    bf_b = hal_py.BooleanFunction.Concat(bf_i, bf_b, i+1)
    

print("OUT: ", bf_out)
print("A: ", bf_a)
print("B: ", bf_b)

bf_sub = hal_py.BooleanFunction.Sub(bf_a, bf_b, 32)
#print("Sub: ", bf_sub)

bf_eq = hal_py.BooleanFunction.Eq(bf_sub, bf_out, 1)
#print("Eq: ", bf_eq)
#print("Out: ", bf_out)

s = hal_py.SMT.Solver()

c = hal_py.SMT.Constraint(hal_py.BooleanFunction.Not(bf_eq, 1))

res = s.with_constraint(c).query(hal_py.SMT.QueryConfig())

print(res.is_sat())
m = res.model
print("A: ", m.evaluate(bf_a))
print("B: ", m.evaluate(bf_b))
print("SUB: ", m.evaluate(bf_out))



#unload everything hal related
hal_py.plugin_manager.unload_all_plugins()