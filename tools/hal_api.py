#!/usr/bin/env python3
"""Generate and query a compact, greppable index of the HAL Python API.

The HAL Python bindings are defined in C++ (pybind11) and their docstrings only
exist inside the compiled ``hal_py.so`` / ``hal_plugins/*.so``. This tool imports
the built modules, introspects them, and writes a three-tier plain-text index:

    documentation/python_api/index.md          overview, one line per class
    documentation/python_api/symbols.tsv       one line per symbol (grep target)
    documentation/python_api/<module>/<X>.md   full docstrings, read on demand

Subcommands::

    hal_api.py gen [--build-dir DIR] [--out DIR] [--no-plugins]
    hal_api.py check                 # exit 1 if the index is stale
    hal_api.py show Netlist.get_gates
    hal_api.py search "traversal"

Usage: python3 tools/hal_api.py gen
"""

import argparse
import hashlib
import importlib
import inspect
import json
import os
import re
import sys
import types

FORMAT_VERSION = 1

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEFAULT_OUT = os.path.join(REPO_ROOT, "documentation", "python_api")
BUILD_DIR_CANDIDATES = ("build", "build_dbg", "build_perf", "cmake-build-debug", "cmake-build-release")


###############################################################################
# build discovery and module loading
###############################################################################


def find_build_dir(explicit=None):
    """Return the newest build directory that contains a compiled hal_py module."""
    candidates = []
    if explicit:
        candidates.append(explicit)
    else:
        if os.environ.get("HAL_BASE_PATH"):
            candidates.append(os.environ["HAL_BASE_PATH"])
        candidates.extend(os.path.join(REPO_ROOT, d) for d in BUILD_DIR_CANDIDATES)

    valid = [c for c in candidates if c and hal_py_lib(c)]
    if not valid:
        raise SystemExit(
            "error: no build directory with lib/hal_py.so found (looked in: {}).\n"
            "       pass --build-dir <dir> or set HAL_BASE_PATH.".format(", ".join(c for c in candidates if c))
        )
    if explicit:
        return os.path.abspath(valid[0])
    return os.path.abspath(max(valid, key=lambda c: os.path.getmtime(hal_py_lib(c))))


def hal_py_lib(build_dir):
    """Return the path of the compiled hal_py module inside build_dir, or None."""
    for name in ("hal_py.so", "hal_py.dylib", "hal_py.pyd"):
        path = os.path.join(build_dir, "lib", name)
        if os.path.isfile(path):
            return path
    return None


class _muted:
    """Silence the C++ log output that HAL writes to stdout/stderr on load."""

    def __enter__(self):
        self._null = os.open(os.devnull, os.O_WRONLY)
        self._saved = [os.dup(1), os.dup(2)]
        sys.stdout.flush()
        sys.stderr.flush()
        os.dup2(self._null, 1)
        os.dup2(self._null, 2)
        return self

    def __exit__(self, *args):
        sys.stdout.flush()
        sys.stderr.flush()
        os.dup2(self._saved[0], 1)
        os.dup2(self._saved[1], 2)
        for fd in self._saved + [self._null]:
            os.close(fd)
        return False


NO_BINDINGS = "does not define module export function"


def plugin_names(build_dir):
    plugin_dir = os.path.join(build_dir, "lib", "hal_plugins")
    if not os.path.isdir(plugin_dir):
        return []
    return sorted(f[: -len(".so")] for f in os.listdir(plugin_dir) if f.endswith(".so"))


def load_hal(build_dir, with_plugins=True, only=None, preload=()):
    """Import hal_py and the plugin modules. Returns (hal_py, {name: module}, {name: error}).

    A plugin module may use a type that another plugin module registers, which makes
    the import order matter. A failed import cannot simply be retried in the same
    process -- it leaves its already-registered types behind and the retry then fails
    with "type is already registered" -- so failures are reported and the caller
    re-attempts them in a fresh process via `preload`.
    """
    os.environ["HAL_BASE_PATH"] = build_dir
    lib_dir = os.path.join(build_dir, "lib")
    if lib_dir not in sys.path:
        sys.path.insert(0, lib_dir)

    with _muted():
        hal_py = importlib.import_module("hal_py")

    plugins, errors = {}, {}
    if not with_plugins:
        return hal_py, plugins, errors

    names = plugin_names(build_dir)
    if not names:
        return hal_py, plugins, errors

    with _muted():
        hal_py.plugin_manager.load_all_plugins()

    for name in list(preload) + [n for n in names if only is None or n == only]:
        if name in plugins:
            continue
        try:
            with _muted():
                plugins[name] = importlib.import_module("hal_plugins." + name)
        except Exception as exc:  # plugin without bindings, or a type it needs is not registered yet
            errors[name] = str(exc).splitlines()[0]
    return hal_py, plugins, errors


###############################################################################
# docstring parsing
###############################################################################

_SIG_RE = re.compile(r"^(?:\d+\.\s+)?([A-Za-z_]\w*\(.*\)(?:\s*->\s*.*)?)$")


def split_doc(doc, strip_prefixes=()):
    """Split a pybind11 docstring into (signature lines, prose).

    An overloaded function documents itself as a ``name(*args, **kwargs)`` stub
    followed by ``Overloaded function.`` and one numbered signature per overload;
    all of those signatures are collected, and the useless stub is dropped.
    """
    if not doc:
        return [], ""
    for prefix in strip_prefixes:
        doc = doc.replace(prefix + ".", "")
    sigs, prose, overloaded = [], [], "Overloaded function." in doc
    for line in doc.splitlines():
        stripped = line.strip()
        if stripped == "Overloaded function.":
            continue
        match = _SIG_RE.match(stripped)
        # signatures head the docstring, or head each numbered overload block
        if match and "(" in stripped and (not any(prose) or stripped[0].isdigit()):
            sigs.append(match.group(1))
        else:
            prose.append(line.rstrip())
    if overloaded and len(sigs) > 1:
        sigs = [s for s in sigs if "(*args, **kwargs)" not in s]
    return sigs, tidy(prose)


def tidy(lines):
    """Strip the C++ raw-string indentation and collapse blank line runs."""
    indents = [len(l) - len(l.lstrip()) for l in lines if l.strip()]
    cut = min(indents) if indents else 0
    out, blank = [], False
    for line in lines:
        line = line[cut:] if line.strip() else ""
        if not line:
            blank = True
            continue
        if blank and out:
            out.append("")
        blank = False
        out.append(line)
    return "\n".join(out)


def summarize(prose, limit=180):
    """First sentence of a docstring body, flattened to a single line."""
    body = []
    for line in prose.splitlines():
        line = line.strip()
        if not line:
            if body:
                break
            continue
        if line.startswith(":"):  # start of the :param:/:returns: block
            break
        if _SIG_RE.match(line) and "(" in line:  # leftover overload signature
            continue
        body.append(line)
    text = " ".join(body)
    match = re.match(r"(.+?\.)(\s|$)", text)
    text = match.group(1) if match else text
    return text[: limit - 1] + "…" if len(text) > limit else text


###############################################################################
# introspection
###############################################################################


class Symbol:
    __slots__ = ("qualname", "kind", "sigs", "prose")

    def __init__(self, qualname, kind, sigs, prose):
        self.qualname = qualname
        self.kind = kind
        self.sigs = sigs
        self.prose = prose

    @property
    def summary(self):
        return summarize(self.prose)


def is_enum(obj):
    return inspect.isclass(obj) and hasattr(obj, "__members__")


def member_kind(cls, name):
    static = inspect.getattr_static(cls, name, None)
    if isinstance(static, property):
        return "prop"
    if isinstance(static, staticmethod):
        return "static"
    return "meth"


def walk_class(cls, qualname, strip, out):
    """Append the class itself, its nested classes, and its members to `out`."""
    sigs, prose = split_doc(cls.__doc__, strip)
    if is_enum(cls):
        values = list(cls.__members__)
        out.append(Symbol(qualname, "enum", ["|".join(values)], prose))
        return
    out.append(Symbol(qualname, "class", sigs, prose))

    for name in sorted(dir(cls)):
        if name.startswith("_"):
            continue
        try:
            member = getattr(cls, name)
        except Exception:
            continue
        if inspect.isclass(member) and member.__module__ != "builtins":
            walk_class(member, qualname + "." + name, strip, out)
            continue
        msigs, mprose = split_doc(getattr(member, "__doc__", None), strip)
        out.append(Symbol(qualname + "." + name, member_kind(cls, name), msigs or [name], mprose))


def walk_module(module, prefix, strip, out, seen=None):
    """Recursively collect every documented symbol reachable from `module`."""
    seen = seen if seen is not None else set()
    if id(module) in seen:
        return out
    seen.add(id(module))

    constants = []
    for name in sorted(dir(module)):
        if name.startswith("_"):
            continue
        try:
            obj = getattr(module, name)
        except Exception:
            continue
        qualname = "{}.{}".format(prefix, name) if prefix else name

        if isinstance(obj, types.ModuleType):
            # recorded so the index can group a module's symbols by submodule
            out.append(Symbol(qualname, "module", [], tidy((getattr(obj, "__doc__", "") or "").splitlines())))
            walk_module(obj, qualname, strip, out, seen)
        elif inspect.isclass(obj):
            walk_class(obj, qualname, strip, out)
        elif callable(obj):
            sigs, prose = split_doc(getattr(obj, "__doc__", None), strip)
            out.append(Symbol(qualname, "func", sigs or [name + "(...)"], prose))
        elif is_enum(type(obj)):
            constants.append("{} = {}".format(name, type(obj).__name__ + "." + str(obj).split(".")[-1]))
    if constants:
        out.append(Symbol((prefix + "." if prefix else "") + "<module constants>", "consts", constants, ""))
    return out


###############################################################################
# writing the index
###############################################################################


def module_of(qualname):
    return qualname.split(".", 1)[0]


def owner_of(qualname, kind, class_names):
    """The class or namespace a symbol is documented under.

    hal_py.Netlist.get_gates    -> hal_py.Netlist       (member of a class)
    hal_py.BooleanFunction.Value-> hal_py.BooleanFunction (nested class/enum)
    hal_py.NetlistUtils.get_... -> hal_py.NetlistUtils  (function in a namespace)
    """
    parts = qualname.split(".")
    for i in range(len(parts) - 1, 0, -1):
        candidate = ".".join(parts[:i])
        if candidate in class_names:
            return candidate
    if kind in ("class", "enum"):
        return qualname
    return ".".join(parts[:-1]) or qualname


def safe_name(qualname):
    """File name for a detail page: module prefix dropped, it is already the directory."""
    _, _, rest = qualname.partition(".")
    return (rest or qualname).replace("<", "_").replace(">", "_").replace(".", "_")


def write_index(out_dir, groups, fingerprint, descriptions=None):
    """Write the root index, the per-module indexes, symbols.tsv and the class detail files."""
    os.makedirs(out_dir, exist_ok=True)
    descriptions = descriptions or {}

    tsv = ["# symbol\tkind\tsignature\tsummary"]
    root = [
        "# HAL Python API index",
        "",
        "Generated by `tools/hal_api.py gen` from the compiled bindings in `{}`.".format(fingerprint["build_dir"]),
        "HAL version {} (git {}).".format(fingerprint.get("hal_version", "?"), fingerprint.get("git", "?")),
        "",
        "Lookup: `python3 tools/hal_api.py show <Symbol>` / `search <regex>`, or grep `symbols.tsv`.",
        "This page lists the modules only -- open `<module>/index.md` for its classes and functions,",
        "and `<module>/<Class>.md` for full docstrings.",
        "",
        "| module | symbols | purpose |",
        "| --- | --- | --- |",
    ]

    for module in sorted(groups):
        purpose = " ".join((descriptions.get(module) or "").split())
        if len(purpose) > 140:
            purpose = purpose[:139] + "…"
        root.append("| [{0}]({0}/index.md) | {1} | {2} |".format(module, len(groups[module]), purpose))
    root.append("")

    for module in sorted(groups):
        symbols = groups[module]
        mod_dir = os.path.join(out_dir, module)
        os.makedirs(mod_dir, exist_ok=True)
        index = [
            "# {}".format(module),
            "",
            " ".join((descriptions.get(module) or "").split()),
            "",
            "Full docstrings: `show <Symbol>`, or the `<Class>.md` file named on each line.",
            "",
        ]

        # submodules are not listed themselves, they group the symbols below them
        submodules = sorted((s.qualname for s in symbols if s.kind == "module"), key=len, reverse=True)
        symbols = [s for s in symbols if s.kind != "module"]

        def group_of(owner):
            for name in submodules:
                if owner == name or owner.startswith(name + "."):
                    return name
            return ""

        # bucket every symbol under the class/namespace it belongs to
        class_names = {s.qualname for s in symbols if s.kind == "class"}
        buckets = {}
        for sym in symbols:
            buckets.setdefault(owner_of(sym.qualname, sym.kind, class_names), []).append(sym)

        group = None
        for owner in sorted(buckets, key=lambda o: (group_of(o), o)):
            if len(submodules) > 1 and group_of(owner) != group:
                group = group_of(owner)
                index += ["", "### {}".format(group or module), ""]
            members = buckets[owner]
            head = next((s for s in members if s.qualname == owner), None)
            kind = head.kind if head else "func"

            if kind == "enum":
                values = head.sigs[0]
                if len(values) > 200:
                    values = values[:200] + "…"
                index.append("- **{}** *(enum)*: {} — {}".format(owner, values, head.summary))
            elif kind == "class":
                index.append("- **{}** — {}  `{}.md`".format(owner, head.summary, safe_name(owner)))
            else:
                # free functions: names only, the index is a map -- use `search`/symbols.tsv for signatures
                names = sorted({s.qualname.rsplit(".", 1)[-1] for s in members if s.kind == "func"})
                if names:
                    index.append("- **{}** *(functions)*: {}".format(owner, ", ".join(names)))

            if kind == "class":
                body = ["# {}".format(owner), ""]
                if head.prose:
                    body += [head.prose, ""]
                for sym in sorted(members, key=lambda s: s.qualname):
                    if sym is head:
                        continue
                    body.append("## {}  *({})*".format(sym.qualname, sym.kind))
                    body += ["    " + s for s in sym.sigs]
                    if sym.prose:
                        body += ["", sym.prose]
                    body.append("")
                with open(os.path.join(mod_dir, safe_name(owner) + ".md"), "w") as fh:
                    fh.write("\n".join(body) + "\n")

            for sym in members:
                # a class or a plain attribute carries no signature -- still list it, by name
                for sig in sym.sigs or [sym.qualname.rsplit(".", 1)[-1]]:
                    tsv.append("{}\t{}\t{}\t{}".format(sym.qualname, sym.kind, sig.replace("\t", " "), sym.summary))

        with open(os.path.join(mod_dir, "index.md"), "w") as fh:
            fh.write("\n".join(index) + "\n")

    # free functions / namespaces that have no class file still need their docs
    for module in sorted(groups):
        loose = [s for s in groups[module] if s.kind in ("func", "consts")]
        if not loose:
            continue
        body = ["# {} — functions and namespaces".format(module), ""]
        for sym in sorted(loose, key=lambda s: s.qualname):
            body.append("## {}  *({})*".format(sym.qualname, sym.kind))
            body += ["    " + s for s in sym.sigs]
            if sym.prose:
                body += ["", sym.prose]
            body.append("")
        with open(os.path.join(out_dir, module, "_functions.md"), "w") as fh:
            fh.write("\n".join(body) + "\n")

    with open(os.path.join(out_dir, "index.md"), "w") as fh:
        fh.write("\n".join(root) + "\n")
    with open(os.path.join(out_dir, "symbols.tsv"), "w") as fh:
        fh.write("\n".join(tsv) + "\n")
    with open(os.path.join(out_dir, "fingerprint.json"), "w") as fh:
        json.dump(fingerprint, fh, indent=2, sort_keys=True)
        fh.write("\n")


###############################################################################
# staleness tracking
###############################################################################


def binary_paths(build_dir, with_plugins=True):
    paths = [hal_py_lib(build_dir)]
    plugin_dir = os.path.join(build_dir, "lib", "hal_plugins")
    if with_plugins and os.path.isdir(plugin_dir):
        paths += [os.path.join(plugin_dir, f) for f in sorted(os.listdir(plugin_dir)) if f.endswith(".so")]
    return [p for p in paths if p]


def make_fingerprint(build_dir, with_plugins, hash_files=True):
    binaries = {}
    for path in binary_paths(build_dir, with_plugins):
        stat = os.stat(path)
        entry = {"size": stat.st_size, "mtime_ns": stat.st_mtime_ns}
        if hash_files:
            digest = hashlib.sha256()
            with open(path, "rb") as fh:
                for chunk in iter(lambda: fh.read(1 << 20), b""):
                    digest.update(chunk)
            entry["sha256"] = digest.hexdigest()[:16]
        binaries[os.path.relpath(path, build_dir)] = entry

    fingerprint = {
        "format_version": FORMAT_VERSION,
        "build_dir": build_dir,
        "with_plugins": with_plugins,
        "binaries": binaries,
    }
    version_file = os.path.join(REPO_ROOT, "CURRENT_VERSION")
    if os.path.isfile(version_file):
        with open(version_file) as fh:
            fingerprint["hal_version"] = fh.read().strip()
    head = os.path.join(REPO_ROOT, ".git", "HEAD")
    if os.path.isfile(head):
        try:
            with open(head) as fh:
                ref = fh.read().strip()
            if ref.startswith("ref: "):
                with open(os.path.join(REPO_ROOT, ".git", ref[5:])) as fh:
                    ref = fh.read().strip()
            fingerprint["git"] = ref[:12]
        except OSError:
            pass
    return fingerprint


def staleness(out_dir):
    """Return None if the index is up to date, otherwise a human-readable reason."""
    path = os.path.join(out_dir, "fingerprint.json")
    if not os.path.isfile(path):
        return "no index at {} yet".format(out_dir)
    with open(path) as fh:
        old = json.load(fh)
    if old.get("format_version") != FORMAT_VERSION:
        return "index was written by an older version of tools/hal_api.py"

    build_dir = old["build_dir"]
    if not hal_py_lib(build_dir or ""):
        return "build directory {} no longer contains hal_py".format(build_dir)

    # cheap pass first: only hash when size/mtime disagree, so an unchanged
    # rebuild does not report the index as stale
    cheap = make_fingerprint(build_dir, old.get("with_plugins", True), hash_files=False)
    if cheap["binaries"].keys() != old["binaries"].keys():
        return "the set of built plugin libraries changed"
    suspects = [k for k, v in cheap["binaries"].items() if old["binaries"][k]["mtime_ns"] != v["mtime_ns"] or old["binaries"][k]["size"] != v["size"]]
    if not suspects:
        return None
    for name in suspects:
        digest = hashlib.sha256()
        with open(os.path.join(build_dir, name), "rb") as fh:
            for chunk in iter(lambda: fh.read(1 << 20), b""):
                digest.update(chunk)
        if digest.hexdigest()[:16] != old["binaries"][name].get("sha256"):
            return "{} was rebuilt and changed".format(name)
    return None


###############################################################################
# subcommands
###############################################################################


def symbols_of(module, name):
    # plugin types report themselves as hal_plugins.X, which is not how they are addressed
    return walk_module(module, name, ("hal_py", "hal_plugins", name), [])


def retry_in_subprocess(build_dir, name, preload):
    """Re-import a plugin module in a fresh process, after `preload`, and return its symbols."""
    import subprocess

    cmd = [sys.executable, os.path.abspath(__file__), "dump", name, "--build-dir", build_dir]
    if preload:
        cmd += ["--preload", ",".join(preload)]
    result = subprocess.run(cmd, capture_output=True, text=True)
    # HAL can crash while tearing down the interpreter; if the module was introspected
    # before that happened, the output is still complete and perfectly usable.
    try:
        return [Symbol(*row) for row in json.loads(result.stdout)], None
    except ValueError:
        return None, (result.stderr.strip().splitlines() or ["exit code {}".format(result.returncode)])[-1]


HAL_PY_PURPOSE = "Core HAL API: netlist, gates, nets, modules, gate libraries, Boolean functions and the netlist decorators."


def plugin_descriptions(hal_py, groups):
    """What each module is for, taken from the plugin's own description string."""
    descriptions = {"hal_py": HAL_PY_PURPOSE}
    for name in groups:
        if name == "hal_py":
            continue
        try:
            with _muted():
                plugin = hal_py.plugin_manager.get_plugin_instance(name)
            descriptions[name] = (plugin.get_description() or "").strip() if plugin else ""
        except Exception:
            descriptions[name] = ""
    return descriptions


def cmd_gen(args):
    build_dir = find_build_dir(args.build_dir)
    with_plugins = not args.no_plugins
    print("reading bindings from {}".format(build_dir), file=sys.stderr)

    hal_py, plugins, errors = load_hal(build_dir, with_plugins)

    groups = {"hal_py": walk_module(hal_py, "hal_py", ("hal_py",), [])}
    for name, module in sorted(plugins.items()):
        symbols = symbols_of(module, name)
        if symbols:
            groups[name] = symbols

    for name, error in sorted(errors.items()):
        if NO_BINDINGS in error:
            continue  # plugin simply has no Python bindings
        symbols, retry_error = retry_in_subprocess(build_dir, name, sorted(plugins))
        if symbols:
            groups[name] = symbols
            print("note: hal_plugins.{} needs the other plugin modules imported first".format(name), file=sys.stderr)
        else:
            print("warning: could not import hal_plugins.{}: {}".format(name, retry_error or error), file=sys.stderr)

    fingerprint = make_fingerprint(build_dir, with_plugins)
    write_index(args.out, groups, fingerprint, plugin_descriptions(hal_py, groups))

    total = sum(len(v) for v in groups.values())
    print("wrote {} symbols from {} modules to {}".format(total, len(groups), args.out), file=sys.stderr)
    return 0


def cmd_dump(args):
    """Internal: introspect a single plugin module after importing `--preload` first."""
    build_dir = find_build_dir(args.build_dir)
    preload = [p for p in (args.preload or "").split(",") if p]
    _, plugins, errors = load_hal(build_dir, True, only=args.module, preload=preload)
    if args.module not in plugins:
        print(errors.get(args.module, "import failed"), file=sys.stderr)
        return 1
    symbols = symbols_of(plugins[args.module], args.module)
    json.dump([[s.qualname, s.kind, s.sigs, s.prose] for s in symbols], sys.stdout)
    return 0


def cmd_check(args):
    reason = staleness(args.out)
    if reason:
        print("stale: {} -- run `python3 tools/hal_api.py gen`".format(reason))
        return 1
    print("up to date")
    return 0


def load_symbols(out_dir):
    path = os.path.join(out_dir, "symbols.tsv")
    if not os.path.isfile(path):
        raise SystemExit("error: no index at {}. run `python3 tools/hal_api.py gen` first.".format(out_dir))
    rows = []
    with open(path) as fh:
        for line in fh:
            if line.startswith("#"):
                continue
            parts = line.rstrip("\n").split("\t")
            if len(parts) == 4:
                rows.append(parts)
    return rows


def warn_if_stale(out_dir):
    try:
        reason = staleness(out_dir)
    except Exception:
        return
    if reason:
        print("warning: index may be stale ({}); run `hal_api.py gen`\n".format(reason), file=sys.stderr)


def cmd_search(args):
    warn_if_stale(args.out)
    pattern = re.compile(args.pattern, re.IGNORECASE)
    hits = 0
    for qualname, kind, sig, summary in load_symbols(args.out):
        if pattern.search(qualname) or pattern.search(sig) or pattern.search(summary):
            print("{}\t{}\t{}".format(qualname, sig, summary))
            hits += 1
            if args.limit and hits >= args.limit:
                print("... (truncated at {} hits, use --limit 0 for all)".format(args.limit))
                break
    if not hits:
        print("no match for {!r}".format(args.pattern))
        return 1
    return 0


def cmd_show(args):
    warn_if_stale(args.out)
    rows = load_symbols(args.out)
    query = args.symbol
    matches = [r for r in rows if r[0] == query or r[0].endswith("." + query)]
    if not matches:
        # a namespace such as NetlistFactory holds functions but is no symbol itself
        members = [r for r in rows if r[0].startswith(query + ".") or ("." + query + ".") in r[0]]
        if members:
            namespace = sorted({r[0].rsplit(".", 1)[0] for r in members})
            if len(namespace) == 1:
                print("# {}  (namespace) -- `show {}.<name>` for the full docs\n".format(namespace[0], query))
                for row in members:
                    print("{}\n    {}\n".format(row[2], row[3]))
                return 0

        prefix = [r for r in rows if query.lower() in r[0].lower()]
        if not prefix:
            print("unknown symbol {!r} -- try `hal_api.py search {}`".format(query, query))
            return 1
        print("no exact match for {!r}, did you mean:".format(query))
        for row in prefix[:20]:
            print("  {}".format(row[0]))
        return 1

    distinct = sorted({r[0] for r in matches})
    if len(distinct) > 1:
        print("{!r} is ambiguous, pick one:".format(query))
        for name in distinct:
            print("  {}".format(name))
        return 1

    qualname, kind = matches[0][0], matches[0][1]
    module = module_of(qualname)
    owner = owner_of(qualname, kind, {r[0] for r in rows if r[1] == "class"})
    detail = os.path.join(args.out, module, safe_name(owner) + ".md")
    if not os.path.isfile(detail):
        detail = os.path.join(args.out, module, "_functions.md")
    if not os.path.isfile(detail):
        for row in matches:
            print("{}\t{}\t{}".format(row[0], row[2], row[3]))
        return 0

    with open(detail) as fh:
        text = fh.read()
    if qualname == owner:  # asked for the whole class
        sys.stdout.write(text)
        return 0

    section, capturing = [], False
    for line in text.splitlines():
        if line.startswith("## "):
            capturing = line[3:].split("  *(")[0].strip() == qualname
        if capturing:
            section.append(line)
    if section:
        print("\n".join(section).rstrip())
    else:
        sys.stdout.write(text)
    return 0


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--out", default=DEFAULT_OUT, help="index directory (default: documentation/python_api)")
    sub = parser.add_subparsers(dest="command", required=True)

    gen = sub.add_parser("gen", help="(re)generate the index from the compiled bindings")
    gen.add_argument("--build-dir", help="HAL build directory (default: newest build*/ with lib/hal_py.so)")
    gen.add_argument("--no-plugins", action="store_true", help="index hal_py only, skip plugin modules")
    gen.set_defaults(func=cmd_gen)

    dump = sub.add_parser("dump", help=argparse.SUPPRESS)  # internal, used by gen for retries
    dump.add_argument("module")
    dump.add_argument("--build-dir")
    dump.add_argument("--preload", help="comma-separated plugin modules to import first")
    dump.set_defaults(func=cmd_dump)

    check = sub.add_parser("check", help="exit 1 if the index no longer matches the built libraries")
    check.set_defaults(func=cmd_check)

    show = sub.add_parser("show", help="print the full docs of a class or member")
    show.add_argument("symbol", help="e.g. Netlist, Netlist.get_gates, hal_py.NetlistFactory")
    show.set_defaults(func=cmd_show)

    search = sub.add_parser("search", help="regex search over all symbols and summaries")
    search.add_argument("pattern")
    search.add_argument("--limit", type=int, default=60, help="max hits (0 = unlimited)")
    search.set_defaults(func=cmd_search)

    args = parser.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
