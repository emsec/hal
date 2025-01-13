
# Example of using build_feature_vec for gate_feature with all available features

from hal_plugins import machine_learning

# Create the feature context with the netlist
fc = machine_learning.Context(netlist)


features = [
    #machine_learning.gate_feature.ConnectedGlobalIOs(),

    machine_learning.gate_feature.DistanceGlobalIO(hal_py.PinDirection.output, directed=True, forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.PinType.enable]),
    machine_learning.gate_feature.DistanceGlobalIO(hal_py.PinDirection.output, directed=False, forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.PinType.enable]),
    machine_learning.gate_feature.DistanceGlobalIO(hal_py.PinDirection.input, directed=True, forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.PinType.enable]),
    machine_learning.gate_feature.DistanceGlobalIO(hal_py.PinDirection.input, directed=False, forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.PinType.enable]),

    # machine_learning.gate_feature.SequentialDistanceGlobalIO(hal_py.PinDirection.output, directed=True, forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.PinType.enable]),
    # machine_learning.gate_feature.SequentialDistanceGlobalIO(hal_py.PinDirection.output, directed=False, forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.PinType.enable]),
    # machine_learning.gate_feature.SequentialDistanceGlobalIO(hal_py.PinDirection.input, directed=True, forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.PinType.enable]),
    # machine_learning.gate_feature.SequentialDistanceGlobalIO(hal_py.PinDirection.input, directed=False, forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.PinType.enable]),

    # machine_learning.gate_feature.IODegrees(),

    # machine_learning.gate_feature.GateTypeOneHot(),

    # machine_learning.gate_feature.NeighboringGateTypes(1, hal_py.PinDirection.output, directed=True),
    # machine_learning.gate_feature.NeighboringGateTypes(2, hal_py.PinDirection.output, directed=True),
    # machine_learning.gate_feature.NeighboringGateTypes(3, hal_py.PinDirection.output, directed=True),

    # machine_learning.gate_feature.NeighboringGateTypes(1, hal_py.PinDirection.input, directed=True),
    # machine_learning.gate_feature.NeighboringGateTypes(2, hal_py.PinDirection.input, directed=True),
    # machine_learning.gate_feature.NeighboringGateTypes(3, hal_py.PinDirection.input, directed=True),

    # machine_learning.gate_feature.BetweennessCentrality(directed = True, cutoff=-1),
    # machine_learning.gate_feature.BetweennessCentrality(directed = True, cutoff=16),
    # machine_learning.gate_feature.BetweennessCentrality(directed = False, cutoff=-1),
    # machine_learning.gate_feature.BetweennessCentrality(directed = False, cutoff=16),
    # machine_learning.gate_feature.SequentialBetweennessCentrality(directed = True, cutoff=-1),
    # machine_learning.gate_feature.SequentialBetweennessCentrality(directed = True, cutoff=16),
    # machine_learning.gate_feature.SequentialBetweennessCentrality(directed = False, cutoff=-1),
    # machine_learning.gate_feature.SequentialBetweennessCentrality(directed = False, cutoff=16),

    # machine_learning.gate_feature.HarmonicCentrality(direction=hal_py.PinDirection.output, cutoff=-1),
    # machine_learning.gate_feature.HarmonicCentrality(direction=hal_py.PinDirection.output, cutoff=16),
    # machine_learning.gate_feature.HarmonicCentrality(direction=hal_py.PinDirection.inout, cutoff=-1),
    # machine_learning.gate_feature.HarmonicCentrality(direction=hal_py.PinDirection.inout, cutoff=16),
    # machine_learning.gate_feature.SequentialHarmonicCentrality(direction=hal_py.PinDirection.output, cutoff=-1),
    # machine_learning.gate_feature.SequentialHarmonicCentrality(direction=hal_py.PinDirection.output, cutoff=16),
    # machine_learning.gate_feature.SequentialHarmonicCentrality(direction=hal_py.PinDirection.inout, cutoff=-1),
    # machine_learning.gate_feature.SequentialHarmonicCentrality(direction=hal_py.PinDirection.inout, cutoff=16),
]

gates = [netlist.get_gate_by_id(3)]

# Build the feature vector for the pair of gates
feature_vector = machine_learning.gate_feature.build_feature_vecs(fc, features, gates)

print("Feature vector:", feature_vector)
