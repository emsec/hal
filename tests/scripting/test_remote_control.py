"""Starts the GUI offscreen with --remote and drives it through tools/hal_remote.py.

Usage: test_remote_control.py <hal binary> <project directory> <work directory>
"""
import os
import subprocess
import sys
import tempfile
import time
from pathlib import Path

HAL, PROJECT, WORK = sys.argv[1], sys.argv[2], Path(sys.argv[3])
sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools"))
from hal_remote import HalRemote  # noqa: E402

WORK.mkdir(parents=True, exist_ok=True)
# a Unix socket path is limited to about a hundred characters, which a build directory can exceed
SOCKET = Path(tempfile.mkdtemp(prefix="hal-remote-")) / "hal.sock"

env = dict(os.environ, QT_QPA_PLATFORM="offscreen")
proc = subprocess.Popen([HAL, "--gui", "-p", PROJECT, "--remote", str(SOCKET), "--no-log"], env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

failures = []


def expect(label, condition, detail=""):
    if not condition:
        failures.append(f"{label}: {detail}")


try:
    deadline = time.time() + 120
    while not SOCKET.exists() and time.time() < deadline and proc.poll() is None:
        time.sleep(0.2)
    expect("socket appeared", SOCKET.exists(), "GUI did not open the socket")

    with HalRemote(str(SOCKET), timeout=120) as hal:
        expect("netlist is predefined", hal.eval("netlist.get_design_name()") == "FSM")
        gates = hal.eval("len(netlist.get_gates())")
        expect("gates readable", gates > 0, str(gates))

        exit_code, out, err = hal.run("print('hello from the gui')")
        expect("stdout comes back", exit_code == 0 and "hello from the gui" in out, f"{exit_code} {out!r} {err!r}")

        exit_code, out, err = hal.run("raise ValueError('remote failure')")
        expect("exception: exit code 1 and traceback", exit_code == 1 and "remote failure" in err, f"{exit_code} {err!r}")

        exit_code, _, _ = hal.run("import sys; sys.exit(4)")
        expect("sys.exit code comes back", exit_code == 4, str(exit_code))

        hal.run("gui.select_gate([g.get_id() for g in netlist.get_gates()][:2])")
        expect("selection through gui", len(hal.eval("gui.get_selected_gate_ids()")) == 2)

        view_id = hal.eval("gui.View.isolate_in_new([], netlist.get_gates()[:3])")
        expect("view created", isinstance(view_id, int) and view_id > 0, str(view_id))
        expect("view holds the gates", hal.eval(f"len(gui.View.get_gates({view_id}))") == 3)
        expect("view can be shown", hal.eval(f"gui.View.show({view_id})") is True)
        expect("unknown view is refused", hal.eval("gui.View.show(999999)") is False)
        named = hal.eval("gui.View.isolate_in_new([], netlist.get_gates()[:2], 'probe view')")
        expect("named view", hal.eval(f"gui.View.get_name({named})") == "probe view")

        graph_png, window_png = WORK / "graph.png", WORK / "window.png"
        for png in (graph_png, window_png):
            png.unlink(missing_ok=True)
        expect("graph screenshot", hal.screenshot(graph_png, view_id) is True)
        expect("graph screenshot is a PNG", graph_png.exists() and graph_png.read_bytes()[:8] == b"\x89PNG\r\n\x1a\n" and graph_png.stat().st_size > 1000, str(graph_png.stat().st_size if graph_png.exists() else None))
        expect("window screenshot", hal.window_screenshot(window_png) is True)
        expect("window screenshot is a PNG", window_png.exists() and window_png.read_bytes()[:8] == b"\x89PNG\r\n\x1a\n" and window_png.stat().st_size > 1000)

        # requests queue behind one another: the second waits for the first
        exit_code, out, _ = hal.run("import time; time.sleep(1); print('first')")
        exit_code2, out2, _ = hal.run("print('second')")
        expect("requests run in order", "first" in out and "second" in out2)

        hal.quit(0, discard_changes=True)

    rc = proc.wait(timeout=60)
    expect("GUI exited with the requested code", rc == 0, str(rc))
finally:
    if proc.poll() is None:
        proc.kill()
        proc.communicate()

if failures:
    print(f"{len(failures)} check(s) failed:")
    for f in failures:
        print("  -", f)
    sys.exit(1)
print("remote control works")
