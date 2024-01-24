import json

from hal_plugins import bitorder_propagation
from hal_plugins import netlist_preprocessing


min_group_size = 8

# clean up all _ordered_suffixes in the netlist
for m in netlist.get_modules():
    for pg in m.get_pin_groups():
        new_name  = pg.name
        while new_name.endswith("_ordered"):
            new_name = new_name.removesuffix("_ordered")

        if new_name != pg.name:
            m.set_pin_group_name(pg, new_name)
            #print(pg.name, new_name)

# reconstruct top module pin groups
netlist_preprocessing.NetlistPreprocessingPlugin.reconstruct_top_module_pin_groups(netlist)

unknown_module_pingroups = list()
known_module_pingroups = list()

use_register_ground_truth = True

for m in netlist.get_modules():
    if "DANA" in m.name:
        for pg in m.get_pin_groups():
            if pg.size() > min_group_size:
                unknown_module_pingroups.append((m, pg))

        continue

    # value checks do not provide a bitorder
    if "VALUE_CHECK" in m.name:
        continue

    for pg in m.get_pin_groups():
        if pg.name not in ["I", "O"] and (pg.size() > min_group_size):
            known_module_pingroups.append((m, pg))   

# generate ground truth for register modules
bitorder_ground_truth = dict()
if use_register_ground_truth:
    for m, pg in unknown_module_pingroups:
        net_to_index = dict()
        for p in pg.pins:
            if p.direction == hal_py.PinDirection.output:
                gate = p.net.get_sources()[0].gate
            else:
                gate = p.net.get_destinations(lambda ep : ep.gate.module == m)[0].gate

            if ("preprocessing_information", "multi_bit_indexed_identifiers") in gate.data:
                _, reconstructed_identifiers_str = gate.get_data("preprocessing_information", "multi_bit_indexed_identifiers")

                reconstructed_identifiers = json.loads(reconstructed_identifiers_str)

                if len(reconstructed_identifiers) == 0:
                    group_name = gate.name
                    index = 0
                else:
                    identifier = reconstructed_identifiers[0]
                    group_name = identifier[0]
                    index = identifier[1]

                    # if (identifier[2] != "gate_name"):
                    #     print("Warning, found identifier that stems not from a gate name, this could lead to unwanted behavior")
                    #     print(reconstructed_identifiers)
            
    
            net_to_index[p.net] = index
            #print("Net {} / {} - Gate {} / {}: {} {}".format(p.net.id, p.net.name, gate.id, gate.name, group_name, index))

        bitorder_ground_truth[(m, pg)] = net_to_index

print("KNOWN MODULE BITORDER:")
for mpg in known_module_pingroups:
    m, pg = mpg
    print("\t{} - {}".format(m.name, pg.name))

print("UNKNOWN MODULE BITORDER:")
for mpg in unknown_module_pingroups:
    m, pg = mpg
    print("\t{} - {}".format(m.name, pg.name))

# 1) only registers
bitorder_propagation_result = bitorder_propagation.BitorderPropagationPlugin.propagate_bitorder(known_module_pingroups, unknown_module_pingroups)

relative_unknown = 1.0
if len(unknown_module_pingroups) != 0:
    relative_unknown = (len(bitorder_propagation_result) - len(known_module_pingroups)) / len(unknown_module_pingroups)

relative_total = 1.0
if (len(unknown_module_pingroups) + len(known_module_pingroups)) != 0:
    relative_total = len(bitorder_propagation_result) / (len(unknown_module_pingroups) + len(known_module_pingroups))


benchmark_results = dict()
benchmark_results["BITORDER_PROPAGATION"] = dict()
benchmark_results["BITORDER_PROPAGATION"]["register_only"] = dict()
benchmark_results["BITORDER_PROPAGATION"]["register_only"]["initial_known_pingroups"] = len(known_module_pingroups)
benchmark_results["BITORDER_PROPAGATION"]["register_only"]["unknown_pingroups"] = len(unknown_module_pingroups)
benchmark_results["BITORDER_PROPAGATION"]["register_only"]["final_known_pingroups"] = len(bitorder_propagation_result)
benchmark_results["BITORDER_PROPAGATION"]["register_only"]["relative_unknown"] = "{:.2}".format(relative_unknown)
benchmark_results["BITORDER_PROPAGATION"]["register_only"]["relative_total"] = "{:.2}".format(relative_total)

print(benchmark_results)