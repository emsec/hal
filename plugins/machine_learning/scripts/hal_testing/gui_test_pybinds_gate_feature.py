
# Example of using build_feature_vec for gate_feature with all available features

from hal_plugins import machine_learning

# Create the feature context with the netlist
fc = machine_learning.Context(netlist)


features = [
    machine_learning.GateFeature.GateFeatureSingle.ConnectedGlobalIOs(),

    machine_learning.GateFeature.GateFeatureSingle.DistanceGlobalIO(hal_py.PinDirection.output, directed=True, forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.PinType.enable]),
    machine_learning.GateFeature.GateFeatureSingle.DistanceGlobalIO(hal_py.PinDirection.output, directed=False, forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.PinType.enable]),
    machine_learning.GateFeature.GateFeatureSingle.DistanceGlobalIO(hal_py.PinDirection.input, directed=True, forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.PinType.enable]),
    machine_learning.GateFeature.GateFeatureSingle.DistanceGlobalIO(hal_py.PinDirection.input, directed=False, forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.PinType.enable]),

    machine_learning.GateFeature.GateFeatureSingle.SequentialDistanceGlobalIO(hal_py.PinDirection.output, directed=True, forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.PinType.enable]),
    machine_learning.GateFeature.GateFeatureSingle.SequentialDistanceGlobalIO(hal_py.PinDirection.output, directed=False, forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.PinType.enable]),
    machine_learning.GateFeature.GateFeatureSingle.SequentialDistanceGlobalIO(hal_py.PinDirection.input, directed=True, forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.PinType.enable]),
    machine_learning.GateFeature.GateFeatureSingle.SequentialDistanceGlobalIO(hal_py.PinDirection.input, directed=False, forbidden_pin_types=[hal_py.PinType.clock, hal_py.PinType.reset, hal_py.PinType.enable]),

    machine_learning.GateFeature.GateFeatureSingle.IODegrees(),

    machine_learning.GateFeature.GateFeatureSingle.GateTypeOneHot(),

    machine_learning.GateFeature.GateFeatureSingle.NeighboringGateTypes(1, hal_py.PinDirection.output, directed=True),
    machine_learning.GateFeature.GateFeatureSingle.NeighboringGateTypes(2, hal_py.PinDirection.output, directed=True),
    machine_learning.GateFeature.GateFeatureSingle.NeighboringGateTypes(3, hal_py.PinDirection.output, directed=True),

    machine_learning.GateFeature.GateFeatureSingle.NeighboringGateTypes(1, hal_py.PinDirection.input, directed=True),
    machine_learning.GateFeature.GateFeatureSingle.NeighboringGateTypes(2, hal_py.PinDirection.input, directed=True),
    machine_learning.GateFeature.GateFeatureSingle.NeighboringGateTypes(3, hal_py.PinDirection.input, directed=True),

    machine_learning.GateFeature.GateFeatureBulk.BetweennessCentrality(directed = True, cutoff=-1),
    machine_learning.GateFeature.GateFeatureBulk.BetweennessCentrality(directed = True, cutoff=16),
    machine_learning.GateFeature.GateFeatureBulk.BetweennessCentrality(directed = False, cutoff=-1),
    machine_learning.GateFeature.GateFeatureBulk.BetweennessCentrality(directed = False, cutoff=16),
    machine_learning.GateFeature.GateFeatureBulk.SequentialBetweennessCentrality(directed = True, cutoff=-1),
    machine_learning.GateFeature.GateFeatureBulk.SequentialBetweennessCentrality(directed = True, cutoff=16),
    machine_learning.GateFeature.GateFeatureBulk.SequentialBetweennessCentrality(directed = False, cutoff=-1),
    machine_learning.GateFeature.GateFeatureBulk.SequentialBetweennessCentrality(directed = False, cutoff=16),

    machine_learning.GateFeature.GateFeatureBulk.HarmonicCentrality(direction=hal_py.PinDirection.output, cutoff=-1),
    machine_learning.GateFeature.GateFeatureBulk.HarmonicCentrality(direction=hal_py.PinDirection.output, cutoff=16),
    machine_learning.GateFeature.GateFeatureBulk.HarmonicCentrality(direction=hal_py.PinDirection.inout, cutoff=-1),
    machine_learning.GateFeature.GateFeatureBulk.HarmonicCentrality(direction=hal_py.PinDirection.inout, cutoff=16),
    machine_learning.GateFeature.GateFeatureBulk.SequentialHarmonicCentrality(direction=hal_py.PinDirection.output, cutoff=-1),
    machine_learning.GateFeature.GateFeatureBulk.SequentialHarmonicCentrality(direction=hal_py.PinDirection.output, cutoff=16),
    machine_learning.GateFeature.GateFeatureBulk.SequentialHarmonicCentrality(direction=hal_py.PinDirection.inout, cutoff=-1),
    machine_learning.GateFeature.GateFeatureBulk.SequentialHarmonicCentrality(direction=hal_py.PinDirection.inout, cutoff=16),
]

#gates = [netlist.get_gate_by_id(21)]
gates = netlist.get_gates(lambda g: g.type.has_property(hal_py.sequential))

# Build the feature vector for the pair of gates
feature_vector = machine_learning.gate_feature.build_feature_vecs(fc, features, gates)

print("Feature vector:", feature_vector)
