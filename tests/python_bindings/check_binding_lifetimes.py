#!/usr/bin/env python3
"""Check that the Python bindings do not hand out a borrowed pointer without keeping its owner alive.

Reads the binding sources rather than running them, because the failure it looks for does not show
up as a crash: a gate taken from a netlist that is then dropped keeps working and quietly returns
another gate's data.

Two rules are checked.

1. A class bound with a non-owning holder must never be handed over by a factory that returns a
   `unique_ptr`, because nothing would ever free it. `NetlistGraph` drifted into exactly this and
   leaked every graph ever built.

2. A binding that returns something owned by the receiver has to say so with
   `return_value_policy::reference_internal`, which pybind applies to each element of a returned
   container as well, or with an explicit `keep_alive`. Without it the owner can be freed while
   Python still refers to the object.

The check is deliberately signature-based. An earlier docstring-based version missed 40 percent of
the cases, because `:rtype:` is documentation and drifts: one binding said `hal_py.module` in
lower case and another wrote `set(...)` where the rest of the file writes `set[...]`.
"""

import re
import sys
from collections import defaultdict
from pathlib import Path

# Objects owned by something else and only borrowed by Python. Anything returning one of these has
# to keep its owner alive.
BORROWED = {
    "Gate", "Net", "Module", "Grouping", "Endpoint", "GatePin", "ModulePin",
    "GatePinGroup", "ModulePinGroup", "GateType", "GateTypeComponent",
}

# Owned by a manager or a singleton that outlives every netlist, so no link is needed.
IMMORTAL_OWNERS = {"GateLibrary", "BasePluginInterface", "ProjectManager"}

CLASS_RE = re.compile(r"py::class_<\s*([\w:]+)\s*(?:,\s*([^>]*?))?\s*>", re.S)
DEF_RE = re.compile(
    r"(\w+)\.(def|def_static|def_property_readonly|def_property|def_readonly|def_readwrite)"
    r"\(\s*\n?\s*\"(\w+)\""
)


def binding_files(root: Path):
    # Plugins kept in a repository of their own are checked as well: they bind the same borrowed
    # types and get the lifetime wrong in the same ways.
    for base in ("src/python_bindings", "plugins"):
        for path in sorted((root / base).rglob("*.cpp")):
            text = str(path)
            if "python" in text or "binding" in text:
                yield path


def main() -> int:
    root = Path(__file__).resolve().parents[2]
    non_owning, sources = set(), {}
    for path in binding_files(root):
        sources[path] = path.read_text()
        for match in CLASS_RE.finditer(sources[path]):
            extra = match.group(2) or ""
            if "RawPtrWrapper" in extra or "BasePin<" in extra:
                non_owning.add(match.group(1).split("::")[-1])

    failures = defaultdict(list)

    for path, text in sources.items():
        rel = path.relative_to(root)

        # rule 1
        for match in re.finditer(r"->\s*(?:std::optional<)?std::unique_ptr<\s*([\w:]+)", text):
            name = match.group(1).split("::")[-1]
            if name in non_owning:
                line = text[: match.start()].count("\n") + 1
                failures["a factory hands over ownership of a class bound with a non-owning holder"].append(
                    f"{rel}:{line}: {name}"
                )

        # rule 2
        submodules = set(re.findall(r"auto\s+(\w+)\s*=\s*\w+\.def_submodule", text))
        for match in DEF_RE.finditer(text):
            receiver, form, name = match.groups()
            if receiver in submodules or receiver == "m" or form == "def_static":
                continue  # no receiver to tie the result to, see the deferred cases
            end = text.find(')");', match.start())
            body = text[match.start(): end if end != -1 else match.start() + 4000]
            if 'R"(' not in body:
                continue
            head, doc = body.split('R"(')[0], body.split('R"(', 1)[1]
            if "return_value_policy" in head or "keep_alive" in head or "RawPtrWrapper" in head:
                continue
            returned = re.search(
                r":(?:rtype|type):\s*(?:list|set|dict|tuple)?[\[(]?\s*(?:[\w]+\.)?(\w+)", doc
            )
            if not returned:
                continue
            kind = returned.group(1)
            if kind in BORROWED and kind not in IMMORTAL_OWNERS:
                line = text[: match.start()].count("\n") + 1
                failures["a borrowed object is returned without keeping its owner alive"].append(
                    f"{rel}:{line}: {name} -> {kind}"
                )

    if not failures:
        print(f"checked {len(sources)} binding files, {len(non_owning)} non-owning classes: ok")
        return 0

    for reason, entries in failures.items():
        print(f"\n{reason} ({len(entries)}):", file=sys.stderr)
        for entry in entries:
            print(f"  {entry}", file=sys.stderr)
    print(
        "\nAdd py::return_value_policy::reference_internal to an instance method, or a keep_alive "
        "naming the argument that owns the result. See tests/python_bindings/check_binding_lifetimes.py.",
        file=sys.stderr,
    )
    return 1


if __name__ == "__main__":
    sys.exit(main())
