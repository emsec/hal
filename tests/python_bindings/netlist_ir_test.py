#!/usr/bin/env python3
"""Build a small hierarchical design through the netlist_ir bindings, the way a netlist parser written in Python
would, and check the helpers and the validation. Needs PYTHONPATH to point at the built hal_py module."""

import os
import sys

import hal_py

ir = hal_py.netlist_ir

# keep the interpreter from unloading the extension module at exit, which the plugin registry does not survive
import atexit

atexit.register(os._exit, 0)


def fail(message):
    print("FAIL:", message)
    os._exit(1)


def check(condition, message):
    if not condition:
        fail(message)


design = ir.Design("test.py")

sub = design.add_module("sub")
i = sub.add_port("i", hal_py.PinDirection.input, [ir.Range(1, 0)])
o = sub.add_port("o", hal_py.PinDirection.output)
g = sub.add_instance("g", "AND2", ir.InstanceKind.Gate)
g.add_connection("I0", [i.bits[1]])
g.add_connection("I1", [i.bits[0]])
g.add_connection("O", [o.bits[0]])

top = design.add_module("top")
a = top.add_port("a", hal_py.PinDirection.input, [ir.Range(3, 0)])
y = top.add_port("y", hal_py.PinDirection.output)
m = top.add_signal("m", [ir.Range(1, 0)])
n = top.add_signal("n")

s0 = top.add_instance("s0", "sub", ir.InstanceKind.Module)
s0.add_connection("i", a.slice(ir.Range(3, 2)))
s0.add_connection("o", [m.bit_at([0])])

s1 = top.add_instance("s1", "sub", ir.InstanceKind.Module)
s1.add_connection("i", [a.bit_at([1]), ir.ZERO])
s1.add_connection("o", [m.bit_at([1])])

g2 = top.add_instance("g2", "AND2", ir.InstanceKind.Gate)
g2.add_connection("I0", [m.bit_at([0])])
g2.add_connection("I1", [n.bits[0]])
g2.add_connection("O", y.bits)
top.add_alias(n.bits[0], m.bit_at([1]))

# constants and handles
check(ir.ZERO == 0 and ir.ONE == 1 and ir.FIRST_USER_BIT == 2, "reserved handles")
check(i.bits == [2, 3], f"port bits are allocated from FIRST_USER_BIT: {i.bits}")
check(a.width == 4 and a.bit_name(0) == "a(3)" and a.bit_name(3) == "a(0)", "bit naming")
check(n.bit_name(0) == "n", "scalar bit naming")
check(a.bit_at([5]) is None, "an index outside the range gives None")
check(ir.Range(7, 0).size == 8 and ir.Range(7, 0).offset_of(0) == 7 and ir.Range(7, 0).offset_of(9) is None, "range helpers")

# the builders hand out the objects inside the design, not copies
check(top.find_port("a") is not None and top.find_port("a").width == 4, "find_port")
check(top.find_signal("a") is not None and top.find_signal("m").width == 2, "find_signal covers ports and signals")
check(top.find_instance("s0").find_connection("i").bits == a.bits[0:2], "connection bits")
check(top.find_instance("s1").find_connection("i").bits[-1] == ir.ZERO, "constant connection")
check(len(top.ports) == 2 and len(top.signals) == 2 and len(top.instances) == 3, "container sizes")
check(top.aliases == [(n.bits[0], m.bits[0])], f"aliases: {top.aliases}")
check([mod.name for mod in design.modules] == ["sub", "top"], "modules")

# parameters and attributes
s0.parameters = [ir.TypedValue(hal_py.Parameter.Integer("W", "8"), "8")]
check(top.find_instance("s0").parameters[0].declaration.get_name() == "W", "parameter value round trip")
g2.parameters = [ir.TypedValue(hal_py.Parameter.Boolean("keep", "false", hal_py.Parameter.Source.Attribute), "true")]
check(top.find_instance("g2").parameters[0].declaration.get_type() == hal_py.Parameter.Type.Boolean, "attribute type")
check(top.find_instance("g2").parameters[0].declaration.source == hal_py.Parameter.Source.Attribute, "attribute source")

# top detection and validation
check(design.find_top() == "top", "find_top")
check(design.validate(), "a well-formed design validates")

design.top = "sub"
check(design.find_top() == "sub", "explicit top wins")
design.top = None

# instantiation against the test gate library, the way a Python parser would finish
base = os.environ["HAL_BASE_PATH"]
hal_py.plugin_manager.load("hgl_parser", os.path.join(base, "lib", "hal_plugins", "hgl_parser.so"))
lib = hal_py.GateLibraryManager.load(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "test_utils", "gate_libraries", "test.hgl"))
check(lib is not None, "test gate library loads")
options = ir.InstantiationOptions()
options.top_module_name = "top_module"
nl = ir.instantiate(design, lib, options)
check(nl is not None, "instantiation succeeds")
check(nl.get_design_name() == "top", "design name")
check(len(nl.get_gates()) == 4, f"three AND2 gates plus the GND gate: {[g.get_name() for g in nl.get_gates()]}")
g_top = [g for g in nl.get_gates() if g.get_name() == "g2"]
check(len(g_top) == 1, "top-level gate exists")
check(g_top[0].get_parameter_value("W") is None and len(nl.get_top_module().get_submodules()) == 2, "hierarchy")
s0_mod = [m for m in nl.get_modules() if m.get_name() == "s0"][0]
check(s0_mod.get_parameter_value("W") == "8", "instance parameter on the module")
check(g_top[0].get_parameter_value("keep", hal_py.Parameter.Source.Attribute) == "true", "attribute on the gate")
check(not g_top[0].has_parameter("keep"), "no generic of that name")
check(list(g_top[0].get_parameters(hal_py.Parameter.Source.Attribute).keys()) == ["keep"], "attributes by source")
check((hal_py.Parameter.Source.Attribute, "keep") in g_top[0].get_parameters(), "the store is keyed by source and name")
check(any(n.is_gnd_net() for n in nl.get_nets()), "the constant got a net")

top.add_instance("s0", "sub", ir.InstanceKind.Module)
check(not design.validate(), "a duplicate instance name is rejected")
check(ir.instantiate(design, lib) is None, "instantiating an invalid design fails")

print("netlist_ir bindings: all checks passed")
