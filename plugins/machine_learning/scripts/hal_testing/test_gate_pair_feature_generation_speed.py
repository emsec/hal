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

to_test = list()
for i in range(1000):
    ri_a = random.randint(0, len(seq_gates) - 1)
    ri_b = random.randint(0, len(seq_gates) - 1)
    g_a = seq_gates[ri_a]
    g_b = seq_gates[ri_b]
    to_test.append((g_a, g_b))

ep_filter = lambda _ep, d : d <= 2

# TEST
ctx = machine_learning.Context(netlist, 1)

features = [
    machine_learning.gate_pair_feature.LogicalDistance(direction=hal_py.PinDirection.input, directed= True),
    machine_learning.gate_pair_feature.LogicalDistance(direction=hal_py.PinDirection.input, directed= False),

    machine_learning.gate_pair_feature.LogicalDistance(direction=hal_py.PinDirection.output, directed= True),
    machine_learning.gate_pair_feature.LogicalDistance(direction=hal_py.PinDirection.output, directed= False),

    machine_learning.gate_pair_feature.SequentialDistance(direction=hal_py.PinDirection.input, directed= True),
    machine_learning.gate_pair_feature.SequentialDistance(direction=hal_py.PinDirection.input, directed= False),
    
    machine_learning.gate_pair_feature.SequentialDistance(direction=hal_py.PinDirection.output, directed= True),
    machine_learning.gate_pair_feature.SequentialDistance(direction=hal_py.PinDirection.output, directed= False),

    #machine_learning.gate_pair_feature.PhysicalDistance(),

    machine_learning.gate_pair_feature.SharedControlSignals(),

    # directed features 
    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=1, direction=hal_py.PinDirection.output, directed=True, starting_pin_types=[hal_py.PinType.state, hal_py.PinType.neg_state], forbidden_pin_types=[hal_py.PinType.clock]),
    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=2, direction=hal_py.PinDirection.output, directed=True, starting_pin_types=[hal_py.PinType.state, hal_py.PinType.neg_state], forbidden_pin_types=[hal_py.PinType.clock]),

    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=1, direction=hal_py.PinDirection.input, directed=True, starting_pin_types=[hal_py.PinType.data], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset]),
    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=1, direction=hal_py.PinDirection.input, directed=True, starting_pin_types=[hal_py.PinType.data], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.enable]),
    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=2, direction=hal_py.PinDirection.input, directed=True, starting_pin_types=[hal_py.PinType.data], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset]),
    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=2, direction=hal_py.PinDirection.input, directed=True, starting_pin_types=[hal_py.PinType.data], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.enable]),

    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=1, direction=hal_py.PinDirection.input, directed=True, starting_pin_types=[hal_py.PinType.enable], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset]),
    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=1, direction=hal_py.PinDirection.input, directed=True, starting_pin_types=[hal_py.PinType.enable], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.enable]),
    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=2, direction=hal_py.PinDirection.input, directed=True, starting_pin_types=[hal_py.PinType.enable], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset]),
    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=2, direction=hal_py.PinDirection.input, directed=True, starting_pin_types=[hal_py.PinType.enable], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.enable]),

    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=1, direction=hal_py.PinDirection.input, directed=True, starting_pin_types=[hal_py.PinType.reset], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset]),
    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=1, direction=hal_py.PinDirection.input, directed=True, starting_pin_types=[hal_py.PinType.reset], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.enable]),
    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=2, direction=hal_py.PinDirection.input, directed=True, starting_pin_types=[hal_py.PinType.reset], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset]),
    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=2, direction=hal_py.PinDirection.input, directed=True, starting_pin_types=[hal_py.PinType.reset], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.enable]),

    machine_learning.gate_pair_feature.SharedNeighbors(depth=1, direction=hal_py.PinDirection.output, directed=True, starting_pin_types=[hal_py.PinType.state, hal_py.PinType.neg_state], forbidden_pin_types=[hal_py.PinType.clock]),
    machine_learning.gate_pair_feature.SharedNeighbors(depth=2, direction=hal_py.PinDirection.output, directed=True, starting_pin_types=[hal_py.PinType.state, hal_py.PinType.neg_state], forbidden_pin_types=[hal_py.PinType.clock]),

    machine_learning.gate_pair_feature.SharedNeighbors(depth=1, direction=hal_py.PinDirection.input, directed=True, starting_pin_types=[hal_py.PinType.data], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset]),
    machine_learning.gate_pair_feature.SharedNeighbors(depth=1, direction=hal_py.PinDirection.input, directed=True, starting_pin_types=[hal_py.PinType.data], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.enable]),
    machine_learning.gate_pair_feature.SharedNeighbors(depth=2, direction=hal_py.PinDirection.input, directed=True, starting_pin_types=[hal_py.PinType.data], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset]),
    machine_learning.gate_pair_feature.SharedNeighbors(depth=2, direction=hal_py.PinDirection.input, directed=True, starting_pin_types=[hal_py.PinType.data], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.enable]),

    machine_learning.gate_pair_feature.SharedNeighbors(depth=1, direction=hal_py.PinDirection.input, directed=True, starting_pin_types=[hal_py.PinType.enable], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset]),
    machine_learning.gate_pair_feature.SharedNeighbors(depth=1, direction=hal_py.PinDirection.input, directed=True, starting_pin_types=[hal_py.PinType.enable], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.enable]),
    machine_learning.gate_pair_feature.SharedNeighbors(depth=2, direction=hal_py.PinDirection.input, directed=True, starting_pin_types=[hal_py.PinType.enable], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset]),
    machine_learning.gate_pair_feature.SharedNeighbors(depth=2, direction=hal_py.PinDirection.input, directed=True, starting_pin_types=[hal_py.PinType.enable], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.enable]),

    machine_learning.gate_pair_feature.SharedNeighbors(depth=1, direction=hal_py.PinDirection.input, directed=True, starting_pin_types=[hal_py.PinType.reset], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset]),
    machine_learning.gate_pair_feature.SharedNeighbors(depth=1, direction=hal_py.PinDirection.input, directed=True, starting_pin_types=[hal_py.PinType.reset], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.enable]),
    machine_learning.gate_pair_feature.SharedNeighbors(depth=2, direction=hal_py.PinDirection.input, directed=True, starting_pin_types=[hal_py.PinType.reset], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset]),
    machine_learning.gate_pair_feature.SharedNeighbors(depth=2, direction=hal_py.PinDirection.input, directed=True, starting_pin_types=[hal_py.PinType.reset], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.enable]),

    # undirected features 
    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=1, direction=hal_py.PinDirection.output, directed=False, starting_pin_types=[hal_py.PinType.state, hal_py.PinType.neg_state], forbidden_pin_types=[hal_py.PinType.clock]),
    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=2, direction=hal_py.PinDirection.output, directed=False, starting_pin_types=[hal_py.PinType.state, hal_py.PinType.neg_state], forbidden_pin_types=[hal_py.PinType.clock]),

    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=1, direction=hal_py.PinDirection.input, directed=False, starting_pin_types=[hal_py.PinType.data], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset]),
    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=1, direction=hal_py.PinDirection.input, directed=False, starting_pin_types=[hal_py.PinType.data], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.enable]),
    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=2, direction=hal_py.PinDirection.input, directed=False, starting_pin_types=[hal_py.PinType.data], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset]),
    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=2, direction=hal_py.PinDirection.input, directed=False, starting_pin_types=[hal_py.PinType.data], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.enable]),

    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=1, direction=hal_py.PinDirection.input, directed=False, starting_pin_types=[hal_py.PinType.enable], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset]),
    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=1, direction=hal_py.PinDirection.input, directed=False, starting_pin_types=[hal_py.PinType.enable], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.enable]),
    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=2, direction=hal_py.PinDirection.input, directed=False, starting_pin_types=[hal_py.PinType.enable], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset]),
    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=2, direction=hal_py.PinDirection.input, directed=False, starting_pin_types=[hal_py.PinType.enable], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.enable]),

    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=1, direction=hal_py.PinDirection.input, directed=False, starting_pin_types=[hal_py.PinType.reset], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset]),
    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=1, direction=hal_py.PinDirection.input, directed=False, starting_pin_types=[hal_py.PinType.reset], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.enable]),
    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=2, direction=hal_py.PinDirection.input, directed=False, starting_pin_types=[hal_py.PinType.reset], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset]),
    machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=2, direction=hal_py.PinDirection.input, directed=False, starting_pin_types=[hal_py.PinType.reset], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.enable]),

    machine_learning.gate_pair_feature.SharedNeighbors(depth=1, direction=hal_py.PinDirection.output, directed=False, starting_pin_types=[hal_py.PinType.state, hal_py.PinType.neg_state], forbidden_pin_types=[hal_py.PinType.clock]),
    machine_learning.gate_pair_feature.SharedNeighbors(depth=2, direction=hal_py.PinDirection.output, directed=False, starting_pin_types=[hal_py.PinType.state, hal_py.PinType.neg_state], forbidden_pin_types=[hal_py.PinType.clock]),

    machine_learning.gate_pair_feature.SharedNeighbors(depth=1, direction=hal_py.PinDirection.input, directed=False, starting_pin_types=[hal_py.PinType.data], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset]),
    machine_learning.gate_pair_feature.SharedNeighbors(depth=1, direction=hal_py.PinDirection.input, directed=False, starting_pin_types=[hal_py.PinType.data], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.enable]),
    machine_learning.gate_pair_feature.SharedNeighbors(depth=2, direction=hal_py.PinDirection.input, directed=False, starting_pin_types=[hal_py.PinType.data], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset]),
    machine_learning.gate_pair_feature.SharedNeighbors(depth=2, direction=hal_py.PinDirection.input, directed=False, starting_pin_types=[hal_py.PinType.data], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.enable]),

    machine_learning.gate_pair_feature.SharedNeighbors(depth=1, direction=hal_py.PinDirection.input, directed=False, starting_pin_types=[hal_py.PinType.enable], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset]),
    machine_learning.gate_pair_feature.SharedNeighbors(depth=1, direction=hal_py.PinDirection.input, directed=False, starting_pin_types=[hal_py.PinType.enable], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.enable]),
    machine_learning.gate_pair_feature.SharedNeighbors(depth=2, direction=hal_py.PinDirection.input, directed=False, starting_pin_types=[hal_py.PinType.enable], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset]),
    machine_learning.gate_pair_feature.SharedNeighbors(depth=2, direction=hal_py.PinDirection.input, directed=False, starting_pin_types=[hal_py.PinType.enable], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.enable]),

    machine_learning.gate_pair_feature.SharedNeighbors(depth=1, direction=hal_py.PinDirection.input, directed=False, starting_pin_types=[hal_py.PinType.reset], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset]),
    machine_learning.gate_pair_feature.SharedNeighbors(depth=1, direction=hal_py.PinDirection.input, directed=False, starting_pin_types=[hal_py.PinType.reset], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.enable]),
    machine_learning.gate_pair_feature.SharedNeighbors(depth=2, direction=hal_py.PinDirection.input, directed=False, starting_pin_types=[hal_py.PinType.reset], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset]),
    machine_learning.gate_pair_feature.SharedNeighbors(depth=2, direction=hal_py.PinDirection.input, directed=False, starting_pin_types=[hal_py.PinType.reset], forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.enable]),
]

# this is just to prime the abstraction cache
calculated_features = machine_learning.gate_pair_feature.build_feature_vecs(ctx, features, to_test[0:1])

feature_durations = list()

for feature in features:
    start = time.time()
    calculated_features = machine_learning.gate_pair_feature.build_feature_vecs(ctx, [feature], to_test)
    end = time.time()

    duration = end - start
    feature_durations.append((feature, duration))

feature_durations = sorted(feature_durations, key=lambda x: x[1])
    
for feature, duration in feature_durations:
    print("{} : {}".format(duration, feature.to_string()))
    

hal_py.plugin_manager.unload_all_plugins()
