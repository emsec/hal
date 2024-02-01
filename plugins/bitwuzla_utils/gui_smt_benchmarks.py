from hal_plugins import bitwuzla_utils

net = netlist.get_net_by_id(23708)

#bitwuzla_utils.BitwuzlaUtilsPlugin.benchmark(netlist, list([net]))
#bitwuzla_utils.BitwuzlaUtilsPlugin.benchmark(netlist, netlist.get_nets()[:10])
#bitwuzla_utils.BitwuzlaUtilsPlugin.benchmark_threaded_creation(netlist, netlist.get_nets()[:10000], 11)
#bitwuzla_utils.BitwuzlaUtilsPlugin.benchmark_threaded_solving(netlist, netlist.get_nets()[:10000], 16)


#bitwuzla_utils.BitwuzlaUtilsPlugin.benchmark_simplification(netlist, list([net]))

bitwuzla_utils.BitwuzlaUtilsPlugin.minimal_substitution()