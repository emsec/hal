from hal_plugins import bitorder_propagation

src_mod_id = 10
dst_mod_id = 7

src_grp_name = "O"
dst_grp_name = "DI"

src_mod = netlist.get_module_by_id(src_mod_id)
dst_mod = netlist.get_module_by_id(dst_mod_id)
bitorder_propagation.BitorderPropagationPlugin.propagate_bitorder((src_mod, src_mod.get_pin_group_by_name(src_grp_name)), (dst_mod, dst_mod.get_pin_group_by_name(dst_grp_name)))