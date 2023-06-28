from hal_plugins import machine_learning

g = machine_learning.MachineLearning.Graph.test_construct_netlist_graph(netlist)
l = machine_learning.MachineLearning.Graph.test_construct_node_labels(netlist)

print(g)
print(g.node_features)
print(g.edge_list)

print(l)

machine_learning.MachineLearning.Graph.annotate_netlist_graph(netlist, g)