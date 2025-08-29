tm = netlist.get_top_module()

grp_a = netlist.get_top_module().get_pin_group_by_name("A")
grp_a_pins = grp_a.get_pins()
grp_a_pins.reverse()

print([pin.name for pin in grp_a_pins])

tm.create_pin_group("tmp", grp_a_pins, ascending=False)