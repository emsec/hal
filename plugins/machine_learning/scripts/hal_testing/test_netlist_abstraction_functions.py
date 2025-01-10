#!/usr/bin/env python3
import sys, os
import pathlib

user_name = os.getlogin()

# some necessary configuration:
if user_name == "simon":
    base_path = "/home/simon/projects/hal/"
if user_name == "simon.klix":
    base_path = "/mnt/scratch/simon.klix/tools/hal/"
else:
    print("add base paths for user {} before executing...".format(user_name))
    exit()

sys.path.append(base_path + "build/lib/") #this is where your hal python lib is located
os.environ["HAL_BASE_PATH"] = base_path + "build" # hal base path

import hal_py
import random
import time

#initialize HAL
hal_py.plugin_manager.load_all_plugins()

from hal_plugins import machine_learning

netlist_path = "/home/nfs0/simon.klix/projects/benchmarks/netlists_preprocessed/yosys/NangateOpenCellLibrary/synthetic/arithmetic/synth_0/netlist_1559bb70c68caf4c_7403e6d54ad10560/netlist_1559bb70c68caf4c_7403e6d54ad10560.hal"
gate_lib_path = base_path + "/plugins/gate_libraries/definitions/NangateOpenCellLibrary.hgl"

netlist = hal_py.NetlistFactory.load_netlist(netlist_path, gate_lib_path)


seq_gates = netlist.get_gates(lambda g : g.type.has_property(hal_py.ff))
nl_seq_abstr = hal_py.NetlistAbstraction.create(netlist, seq_gates, False)

res = nl_seq_abstr.get_successors(seq_gates[0])
if not res:
    quit()

nl_seq_abstr_dec = hal_py.NetlistAbstractionDecorator(nl_seq_abstr)

to_test = list()
for i in range(10):
    ri = random.randint(0, len(seq_gates) - 1)
    g = seq_gates[ri]
    to_test.append(g)


ep_filter = lambda _ep, d : d <= 2

# TEST cached
start_cached = time.time()
cached_results = nl_seq_abstr_dec.get_next_matching_gates(to_test, lambda g : True, hal_py.PinDirection.output, True, True, ep_filter, ep_filter)
end_cached = time.time()

# TEST single
original_results = list()
start_original = time.time()
for g in to_test:
    res = nl_seq_abstr_dec.get_next_matching_gates(g, lambda g : True, hal_py.PinDirection.output, True, True, ep_filter, ep_filter)
    original_results.append(res)
end_original = time.time()

print(cached_results == original_results)
print(end_cached - start_cached)
print(end_original - start_original)

hal_py.plugin_manager.unload_all_plugins()
