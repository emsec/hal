#!/usr/bin/env python3
"""Check that NetlistFactory.load_netlist reads a .hal file without being given a gate library.

A .hal file names the gate library it was built with, so the gate library argument is optional for
it. The binding used to default that argument to std::filesystem::path(), which pybind11 converts to
pathlib.Path("") once, when the function is bound. That is PosixPath('.'), so leaving the argument
out passed '.' instead of an empty path, and every such call tried to parse '.' as a gate library
and returned None.
"""

import sys
import tempfile
from pathlib import Path


def main():
    import hal_py

    hal_py.plugin_manager.load_all_plugins()

    gate_library = hal_py.GateLibraryManager.get_gate_library("example_library.hgl")
    if gate_library is None:
        print("could not load example_library.hgl", file=sys.stderr)
        return 1

    netlist = hal_py.NetlistFactory.create_netlist(gate_library)
    netlist.create_gate(gate_library.get_gate_type_by_name("AND2"), "and")

    failures = []
    with tempfile.TemporaryDirectory() as tmp:
        hal_file = Path(tmp) / "netlist.hal"
        if not hal_py.NetlistSerializer.serialize_to_file(netlist, hal_file):
            print(f"could not write {hal_file}", file=sys.stderr)
            return 1

        cases = {
            "load a .hal file without a gate library": lambda: hal_py.NetlistFactory.load_netlist(hal_file),
            "load a .hal file with an empty gate library path": lambda: hal_py.NetlistFactory.load_netlist(hal_file, ""),
            "load a .hal file with its gate library file": lambda: hal_py.NetlistFactory.load_netlist(hal_file, gate_library.get_path()),
        }
        for description, load in cases.items():
            loaded = load()
            ok = loaded is not None and loaded.get_gate_library().get_name() == gate_library.get_name() and [g.get_name() for g in loaded.get_gates()] == ["and"]
            print(f"  {description}: {'ok' if ok else 'FAILED'}")
            if not ok:
                failures.append(description)
            del loaded

    if failures:
        print(f"\n{len(failures)} load(s) failed: {', '.join(failures)}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
