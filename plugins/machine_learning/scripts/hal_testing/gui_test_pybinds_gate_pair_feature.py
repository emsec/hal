# Example of using build_feature_vec for gate_pair_feature with all available features

from hal_plugins import machine_learning

# Create the feature context with the netlist
fc = machine_learning.gate_pair_feature.FeatureContext(netlist)

# Instantiate all available gate pair features
logical_distance = machine_learning.gate_pair_feature.LogicalDistance(direction=hal_py.PinDirection.output)
sequential_distance = machine_learning.gate_pair_feature.SequentialDistance(direction=hal_py.PinDirection.output)
physical_distance = machine_learning.gate_pair_feature.PhysicalDistance()
shared_control_signals = machine_learning.gate_pair_feature.SharedControlSignals()
shared_sequential_neighbors = machine_learning.gate_pair_feature.SharedSequentialNeighbors(depth=2, direction=hal_py.PinDirection.output)
shared_neighbors = machine_learning.gate_pair_feature.SharedNeighbors(depth=2, direction=hal_py.PinDirection.output)

# Collect all features into a list
features = [
    logical_distance,
    sequential_distance,
    #physical_distance,
    shared_control_signals,
    shared_sequential_neighbors,
    shared_neighbors
]

gate_a = netlist.get_gate_by_id(21)
gate_b = netlist.get_gate_by_id(151)


# Build the feature vector for the pair of gates
feature_vector = machine_learning.gate_pair_feature.build_feature_vec(fc, features, gate_a, gate_b)

print("Feature vector:", feature_vector)