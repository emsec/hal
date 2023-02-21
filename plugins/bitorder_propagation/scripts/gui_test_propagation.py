from hal_plugins import bitorder_propagation

res_1 = bitorder_propagation.BitorderPropagationPlugin.propagate_bitorder(netlist, (125, "B"), (83, "Q"))

res_2 = bitorder_propagation.BitorderPropagationPlugin.propagate_bitorder(netlist, (83, "Q"), (83, "D"))

res_3 = bitorder_propagation.BitorderPropagationPlugin.propagate_bitorder(netlist, (83, "D"), (16, "O"))

res_4 = bitorder_propagation.BitorderPropagationPlugin.propagate_bitorder(netlist, list([(16, "A"), (133, "IN"), (124, "B")]), list([(78, "Q")]))

print("Done")

print("{} {} {} {}".format(res_1, res_2, res_3, res_4))
