
# Example of using build_feature_vec for gate_feature with all available features

from hal_plugins import machine_learning

# Create the feature context with the netlist
fc = machine_learning.gate_feature.FeatureContext(netlist)

# Instantiate all available gate pair features
connected_global_ios = machine_learning.gate_feature.ConnectedGlobalIOs()
distance_global_io = machine_learning.gate_feature.DistanceGlobalIO(hal_py.PinDirection.output)
sequnetial_distance_global_io = machine_learning.gate_feature.SequentialDistanceGlobalIO(hal_py.PinDirection.output)
io_degrees = machine_learning.gate_feature.IODegrees()
gate_type_one_hot = machine_learning.gate_feature.GateTypeOneHot()
neighboring_gate_types = machine_learning.gate_feature.NeighboringGateTypes(2, hal_py.PinDirection.output)

# Collect all features into a list
features = [
    connected_global_ios,
    distance_global_io,
    #sequnetial_distance_global_io,
    io_degrees,
    #gate_type_one_hot,
    #neighboring_gate_types,
]

gate_a = netlist.get_gate_by_id(21)

# Build the feature vector for the pair of gates
feature_vector = machine_learning.gate_feature.build_feature_vec(fc, features, gate_a)

print("Feature vector:", feature_vector)
