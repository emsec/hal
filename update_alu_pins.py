mod = netlist.get_module_by_id(10)

for pg_name in ["A", "B", "O"]:
    group = mod.get_pin_group_by_name(pg_name)
    for pin in group.get_pins():
        mod.set_pin_name(pin, pg_name + "(" + str(group.get_index(pin)) + ")")