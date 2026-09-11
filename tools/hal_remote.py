#!/usr/bin/env python3
"""Run Python inside a HAL GUI that was started with --remote <socket>.

As a module:

    from hal_remote import HalRemote
    with HalRemote("/tmp/hal.sock") as hal:
        exit_code, out, err = hal.run("gui.select_gate(1)")
        gates = hal.eval("len(netlist.get_gates())")

As a command:

    hal_remote.py /tmp/hal.sock -c "print(netlist.get_design_name())"
    hal_remote.py /tmp/hal.sock -f setup.py
    echo "gui.select_gate(1)" | hal_remote.py /tmp/hal.sock

The GUI runs each request on its Python thread, as the Python editor would, with `netlist` and `gui` predefined,
one request at a time. Output is streamed back as it appears; the exit code is 0 when the code ran to its end,
the number given to sys.exit, and 1 on an unhandled exception. Only the standard library is needed.
"""
import ast
import json
import socket
import sys


class HalRemote:
    def __init__(self, socket_path, timeout=None):
        self._socket = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        self._socket.settimeout(timeout)
        self._socket.connect(socket_path)
        self._file = self._socket.makefile("rwb")

    def close(self):
        self._file.close()
        self._socket.close()

    def __enter__(self):
        return self

    def __exit__(self, *_):
        self.close()

    def _request(self, request, stdout=None, stderr=None):
        self._file.write(json.dumps(request).encode() + b"\n")
        self._file.flush()
        out, err = [], []
        while True:
            line = self._file.readline()
            if not line:
                raise ConnectionError("the GUI closed the connection")
            reply = json.loads(line)
            if "stdout" in reply:
                out.append(reply["stdout"])
                if stdout:
                    stdout.write(reply["stdout"])
                    stdout.flush()
            if "stderr" in reply:
                err.append(reply["stderr"])
                if stderr:
                    stderr.write(reply["stderr"])
                    stderr.flush()
            if "error" in reply:
                err.append(reply["error"] + "\n")
                if stderr:
                    stderr.write(reply["error"] + "\n")
            if reply.get("done"):
                return reply.get("exit_code", 1), "".join(out), "".join(err)

    def run(self, code, stdout=None, stderr=None):
        """Run code; returns (exit_code, stdout, stderr). Pass streams to also see the output as it appears."""
        return self._request({"code": code}, stdout, stderr)

    def run_file(self, path, stdout=None, stderr=None):
        """Run a script file that the GUI can read; returns (exit_code, stdout, stderr)."""
        return self._request({"file": str(path)}, stdout, stderr)

    def eval(self, expression):
        """Evaluate an expression in the GUI and return its value, for values that repr() round-trips.

        Raises RuntimeError with the GUI's traceback if the expression fails.
        """
        exit_code, out, err = self.run(f"print(repr({expression}))")
        if exit_code != 0:
            raise RuntimeError(err.strip() or f"exit code {exit_code}")
        return ast.literal_eval(out.strip())

    def abort(self):
        """Interrupt whatever is running."""
        return self._request({"abort": True})

    def screenshot(self, path, view_id=0):
        """Render the graph of a view (the shown one by default) to an image file the GUI can write; returns True on success."""
        return self.eval(f"gui.grab_graph_view({str(path)!r}, {int(view_id)})")

    def window_screenshot(self, path):
        """Save a picture of the whole main window to an image file the GUI can write; returns True on success."""
        return self.eval(f"gui.grab_window({str(path)!r})")

    def quit(self, exit_code=0, discard_changes=False):
        """Close the GUI; the process ends with the given code."""
        return self.run(f"import hal_gui; hal_gui.quit({int(exit_code)}, {bool(discard_changes)})")


def main(argv):
    import argparse

    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("socket", help="the socket path the GUI was started with (--remote)")
    source = parser.add_mutually_exclusive_group()
    source.add_argument("-c", "--code", help="Python code to run")
    source.add_argument("-f", "--file", help="script file to run; read by the GUI, so give a path it can see")
    parser.add_argument("--abort", action="store_true", help="interrupt what is running instead")
    args = parser.parse_args(argv)

    with HalRemote(args.socket) as hal:
        if args.abort:
            exit_code, _, _ = hal.abort()
        elif args.file:
            exit_code, _, _ = hal.run_file(args.file, sys.stdout, sys.stderr)
        else:
            code = args.code if args.code is not None else sys.stdin.read()
            exit_code, _, _ = hal.run(code, sys.stdout, sys.stderr)
    return exit_code


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
