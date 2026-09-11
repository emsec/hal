"""Run by test_script_modes.py once without a GUI and once inside the offscreen GUI.

Writes what it sees to the file named by sys.argv[0] and ends with the exit code named by sys.argv[1], through
sys.exit outside the GUI and through hal_gui.quit inside it, where sys.exit would only end the script.
"""
import json
import sys

out_path, exit_code, mode = sys.argv[0], int(sys.argv[1]), sys.argv[2]

seen = {
    "argv": sys.argv,
    "has_netlist": netlist is not None,
    "gates": len(netlist.get_gates()) if netlist is not None else None,
    "design": netlist.get_design_name() if netlist is not None else None,
    "hal_py_star_import": "NetlistFactory" in globals(),
    "has_gui": "gui" in globals(),
}
with open(out_path, "w") as f:
    json.dump(seen, f)

print("probe stdout line")
print("probe stderr line", file=sys.stderr)

if mode == "raise":
    raise RuntimeError("probe raised on purpose")

if seen["has_gui"]:
    import hal_gui
    hal_gui.quit(exit_code, discard_changes=True)
else:
    sys.exit(exit_code)
