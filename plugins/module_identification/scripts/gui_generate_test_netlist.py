test_mod = netlist.get_module_by_id(2)
test_nl = hal_py.SubgraphNetlistDecorator(netlist).copy_subgraph_netlist(test_mod, False)

hal_py.NetlistWriterManager.write(test_nl, "/home/simon/projects/hal/plugins/module_identification/test/test_lib_xilinx/CONST_MUL_2.v")