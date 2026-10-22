#!/usr/bin/env python3
"""Check that an interpreter which loaded the plugins can exit.

Loading the plugins and letting the interpreter exit without unloading them used to segfault. The
parser and writer registries live in libhal_netlist and hold a std::function per plugin that provides
one; the plugin libraries were closed when the map holding them was destroyed, which happens before
those registries are, so every one of those functions was left pointing into memory that had been
unmapped.

The check runs in a subprocess, because what is being tested is the exit of a process rather than
anything observable from within it. It also has to avoid importing any plugin module: an imported
module holds a reference of its own to the library, which keeps it mapped and hides the fault. That
is why the binding smoke test does not notice this one.
"""

import os
import subprocess
import sys

CASES = {
    "load the plugins and exit": "import hal_py; hal_py.plugin_manager.load_all_plugins()",
    "load, unload, and exit": "import hal_py; hal_py.plugin_manager.load_all_plugins(); hal_py.plugin_manager.unload_all_plugins()",
    "load, unload, load again, and exit": (
        "import hal_py; "
        "hal_py.plugin_manager.load_all_plugins(); "
        "hal_py.plugin_manager.unload_all_plugins(); "
        "hal_py.plugin_manager.load_all_plugins()"
    ),
}


def main():
    failures = []
    for description, code in CASES.items():
        result = subprocess.run([sys.executable, "-c", code], capture_output=True, env=os.environ.copy())
        if result.returncode != 0:
            reason = f"signal {-result.returncode}" if result.returncode < 0 else f"exit code {result.returncode}"
            failures.append((description, reason, result.stderr.decode(errors="replace")[-400:]))
        print(f"  {description}: {'ok' if result.returncode == 0 else 'FAILED'}")

    if failures:
        print(f"\n{len(failures)} interpreter(s) did not exit cleanly:\n", file=sys.stderr)
        for description, reason, stderr in failures:
            print(f"--- {description}: {reason} ---", file=sys.stderr)
            print(stderr, file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
