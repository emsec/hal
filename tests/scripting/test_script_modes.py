"""Checks that --python-script behaves the same without a GUI and inside the GUI.

Usage: test_script_modes.py <hal binary> <project directory> <work directory>
"""
import json
import os
import subprocess
import sys
from pathlib import Path

HAL, PROJECT, WORK = sys.argv[1], sys.argv[2], Path(sys.argv[3])
PROBE = Path(__file__).with_name("probe.py")
WORK.mkdir(parents=True, exist_ok=True)


def run(gui, exit_code, mode, project=True):
    out = WORK / f"probe_{'gui' if gui else 'headless'}_{mode}_{exit_code}.json"
    out.unlink(missing_ok=True)
    cmd = [HAL, "--python-script", str(PROBE), "--python-args", f"{out} {exit_code} {mode}", "--no-log"]
    if project:
        cmd += ["-p", PROJECT]
    if gui:
        cmd.insert(1, "--gui")
    env = dict(os.environ, QT_QPA_PLATFORM="offscreen")
    proc = subprocess.Popen(cmd, env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    try:
        stdout, stderr = proc.communicate(timeout=60 if gui and mode == "raise" else 300)
        rc = proc.returncode
    except subprocess.TimeoutExpired:
        # inside the GUI a script that raised leaves the window open on purpose, so there is no exit code to read;
        # kill rather than terminate, as the GUI catches SIGTERM to dump its action log
        proc.kill()
        stdout, stderr = proc.communicate(timeout=30)
        rc = None
    seen = json.loads(out.read_text()) if out.exists() else None
    return rc, seen, stdout, stderr


failures = []


def expect(label, condition, detail=""):
    if not condition:
        failures.append(f"{label}: {detail}")


for mode, exit_code, expected_rc in (("exit", 0, 0), ("exit", 3, 3), ("raise", 0, 1)):
    headless = run(False, exit_code, mode)
    gui = run(True, exit_code, mode)
    label = f"{mode}/{exit_code}"
    for name, (rc, seen, out, err) in (("headless", headless), ("gui", gui)):
        if name == "gui" and mode == "raise":
            expect(f"{label} gui stays open after the script raised", rc is None, f"exited with {rc}")
        else:
            expect(f"{label} {name} exit code", rc == expected_rc, f"got {rc}, expected {expected_rc}\n{err[-800:]}")
        expect(f"{label} {name} wrote its result", seen is not None, err[-800:])
        expect(f"{label} {name} stdout reached the terminal", "probe stdout line" in out, out[-400:])
        expect(f"{label} {name} stderr reached the terminal", "probe stderr line" in err, err[-400:])
        if mode == "raise":
            expect(f"{label} {name} traceback reached the terminal", "probe raised on purpose" in err, err[-400:])
    if headless[1] and gui[1]:
        h = dict(headless[1], has_gui=None, argv=headless[1]["argv"][1:])
        g = dict(gui[1], has_gui=None, argv=gui[1]["argv"][1:])
        expect(f"{label} both modes saw the same netlist and arguments", h == g, f"\nheadless {h}\ngui      {g}")
        expect(f"{label} netlist was loaded", headless[1]["has_netlist"] and headless[1]["gates"] > 0, str(headless[1]))
        expect(f"{label} gui object only inside the GUI", (not headless[1]["has_gui"]) and gui[1]["has_gui"])

rc, seen, out, err = run(False, 0, "exit", project=False)
expect("no project: script still runs with netlist None", rc == 0 and seen is not None and not seen["has_netlist"], f"rc={rc} {seen}\n{err[-400:]}")

if failures:
    print(f"{len(failures)} check(s) failed:")
    for f in failures:
        print("  -", f)
    sys.exit(1)
print("both modes behave the same")
