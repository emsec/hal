#!/usr/bin/env python3
"""Call every no-argument binding once and check that it does not raise.

This exists because a binding can be broken in a way that only shows up when it is called, and
nothing in the test suite used to call one. Adding ``keep_alive<0, 1>`` to a function that returns a
list is accepted by the compiler and by every C++ test, and then fails on every single call with
``TypeError: cannot create weak reference to 'list' object``, because a Python list cannot be a
keep_alive nurse. Eighteen bindings were broken that way at once and ``ctest`` stayed green.

The test is deliberately shallow. It checks that a binding can be called and returns, not that the
answer is right -- the C++ tests cover behaviour. What it does cover is every accessor reachable
from a small netlist, so a new binding is covered the moment it is added, without anyone listing it
here.

Only no-argument bindings are called. Which those are is read from the pybind11 signature in the
docstring rather than guessed, because calling a binding that wants arguments raises a TypeError
that looks exactly like a real failure.
"""

import os
import re
import sys
import traceback
from pathlib import Path

# Names that change the netlist rather than report on it. A no-argument binding is almost always an
# accessor, but these prefixes are worth keeping away from regardless of arity, so that a failure
# later in the run cannot be blamed on an earlier call having mutated the netlist out from under it.
MUTATING_PREFIXES = (
    "create_", "delete_", "remove_", "add_", "set_", "assign_", "mark_", "unmark_",
    "clear_", "reset_", "enable_", "disable_", "unassign_", "move_", "rename_",
    # load_ and unload_ matter more than they look: plugin_manager.unload_all_plugins() and
    # GateLibraryManager.load_all() take no arguments, and calling either pulls the gate library out
    # from under the netlist this test is holding, which aborts the interpreter at exit.
    "load_", "unload_",
)

# Bindings that are expected to raise or that would make the test print, block or exit.
SKIP = {
    "print",            # writes the gate library to stdout
    "exit",
    "quit",
}

SIGNATURE_RE = re.compile(r"^\s*\w+\((.*?)\)\s*->", re.M)


def takes_no_arguments(fn, is_method):
    """Decide from the pybind11 signature whether *fn* can be called with no arguments.

    pybind11 puts the signature in the first line of the docstring. An overloaded binding lists one
    signature per overload, and it is enough that one of them takes nothing.
    """
    doc = getattr(fn, "__doc__", None)
    if not doc:
        return False
    for params in SIGNATURE_RE.findall(doc):
        params = params.strip()
        if not params:
            if not is_method:
                return True
            continue
        # Split on commas that are not inside brackets, so that "List[int]" stays one parameter.
        depth, current, parts = 0, "", []
        for ch in params:
            if ch in "[({":
                depth += 1
            elif ch in "])}":
                depth -= 1
            if ch == "," and depth == 0:
                parts.append(current)
                current = ""
            else:
                current += ch
        parts.append(current)
        parts = [p.strip() for p in parts if p.strip()]
        # A parameter with a default does not have to be passed.
        required = [p for p in parts if "=" not in p]
        if is_method:
            if len(required) == 1 and required[0].startswith("self"):
                return True
        elif not required:
            return True
    return False


def build_netlist(hal_py):
    """Build a netlist small enough to be cheap and rich enough that every accessor has something to
    report: two connected gates, a net, a module and a grouping."""
    gate_library = hal_py.GateLibraryManager.get_gate_library("example_library.hgl")
    netlist = hal_py.NetlistFactory.create_netlist(gate_library)

    and2 = gate_library.get_gate_type_by_name("AND2")
    driver = netlist.create_gate(and2, "driver")
    load = netlist.create_gate(and2, "load")

    net = netlist.create_net("net")
    net.add_source(driver, "O")
    net.add_destination(load, "I0")

    module = netlist.create_module("module", netlist.get_top_module(), [driver])
    grouping = netlist.create_grouping("grouping")
    grouping.assign_gate(driver)

    objects = {
        "Netlist": netlist,
        "GateLibrary": gate_library,
        "GateType": and2,
        "Gate": driver,
        "Net": net,
        "Module": module,
        "Grouping": grouping,
        "Endpoint": net.get_sources()[0],
        "GatePin": and2.get_pins()[0],
        "ModulePin": module.get_pins()[0] if module.get_pins() else None,
    }
    return {name: obj for name, obj in objects.items() if obj is not None}


def exercise(label, obj, failures, called):
    """Call every no-argument method and read every property of *obj*."""
    for name in sorted(dir(type(obj))):
        if name.startswith("_") or name in SKIP or name.startswith(MUTATING_PREFIXES):
            continue

        attribute = getattr(type(obj), name, None)
        if attribute is None:
            continue

        try:
            if isinstance(attribute, property):
                getattr(obj, name)
                called.append(f"{label}.{name}")
            elif callable(attribute):
                if not takes_no_arguments(attribute, is_method=True):
                    continue
                getattr(obj, name)()
                called.append(f"{label}.{name}()")
        except Exception:
            failures.append((f"{label}.{name}", traceback.format_exc(limit=2)))


def import_plugin_modules(failures):
    """Import every plugin module. An extension module can fail at import alone, and a plugin that
    is only ever imported by the documentation build is otherwise never loaded by a test."""
    imported = []
    try:
        import hal_plugins
    except ImportError:
        return imported

    for directory in hal_plugins.__path__:
        for entry in sorted(Path(directory).glob("*.so")):
            name = entry.stem
            if name.startswith("lib"):
                continue
            try:
                __import__(f"hal_plugins.{name}")
                imported.append(name)
            except ImportError as error:
                # Not every plugin exposes a Python module; those that do not are shared libraries
                # sitting in the same directory and are nothing to do with the bindings.
                if "does not define module export function" in str(error):
                    continue
                failures.append((f"import hal_plugins.{name}", traceback.format_exc(limit=2)))
            except Exception:
                failures.append((f"import hal_plugins.{name}", traceback.format_exc(limit=2)))
    return imported


def main():
    import hal_py

    # The gate library is read by a plugin, so the plugins have to be loaded before anything else.
    # They also have to be unloaded again before the interpreter exits: leaving them loaded segfaults
    # at teardown, independently of anything this test does. That is a real defect, but it predates
    # this test -- it reproduces on a build from well before the binding work -- so it is not this
    # test's job to fail on it.
    hal_py.plugin_manager.load_all_plugins()
    try:
        return run_checks(hal_py)
    finally:
        hal_py.plugin_manager.unload_all_plugins()


def run_checks(hal_py):
    failures = []
    called = []

    imported = import_plugin_modules(failures)
    print(f"imported {len(imported)} plugin modules")

    objects = build_netlist(hal_py)
    for label, obj in objects.items():
        exercise(label, obj, failures, called)

    # Submodule-level functions take arguments almost without exception, but the few that do not are
    # worth calling, as they are the ones a user reaches for first.
    for submodule in ("plugin_manager", "GateLibraryManager"):
        namespace = getattr(hal_py, submodule, None)
        if namespace is None:
            continue
        for name in sorted(dir(namespace)):
            if name.startswith("_") or name in SKIP or name.startswith(MUTATING_PREFIXES):
                continue
            function = getattr(namespace, name)
            if not callable(function) or not takes_no_arguments(function, is_method=False):
                continue
            try:
                function()
                called.append(f"{submodule}.{name}()")
            except Exception:
                failures.append((f"{submodule}.{name}", traceback.format_exc(limit=2)))

    print(f"called {len(called)} bindings across {len(objects)} object types")

    if failures:
        print(f"\n{len(failures)} binding(s) raised:\n")
        for name, trace in failures:
            print(f"--- {name} ---")
            print(trace)
        return 1

    print("all bindings callable")
    return 0


if __name__ == "__main__":
    sys.exit(main())
