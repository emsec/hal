#!/usr/bin/env python3
"""
Upgrades HGL gate library files from any version < 6 to version 6.

Usage:
    python upgrade_hgl.py [--in-place] FILE [FILE ...]
    python upgrade_hgl.py [--in-place] DIRECTORY

Options:
    --in-place    Overwrite the input file(s) instead of writing to FILE.v6.hgl

What changed between version 4/5 and version 6:
  1. LUT configuration: the flat {data_identifier, data_category, bit_order} top-level
     format is replaced by an output_pins array where each entry specifies the output
     pin name, data_identifier, bit_offset, bit_count, is_ascending, and input_pins.
  2. A 'parameters' section is added to each cell that carries init data so that
     simulation and analysis tools can resolve INIT identifiers by name and size.
     - LUT cells:  one bit_vector parameter per unique data_identifier, sized 2^n.
     - FF/latch:   one bit_vector parameter of size 1 per data_identifier.
     - RAM cells:  one bit_vector parameter per data_identifier, sized bit_size / count.
  3. Legacy gate-type property names without the 'c_' prefix ("buffer", "carry",
     "mux", "lut") are renamed to their canonical forms ("c_buffer", "c_carry",
     "c_mux", "c_lut").

Limitations:
  - Multi-output LUT cells (e.g. LUT6_2) cannot be fully auto-migrated.  The
    bit_offset and bit_count for each individual output require manual knowledge of
    how the shared INIT value is partitioned.  These cells are left unchanged and a
    WARNING is printed; upgrade them by hand using the new LUT6_2 entry in
    XILINX_UNISIM.hgl as a reference.
  - Version < 2 files use a legacy pin/group format that predates pin_groups and is
    not supported here.  All known HAL-distributed gate libraries have been at version
    >= 2 for years, so this should not arise in practice.
"""

import argparse
import copy
import json
import os
import sys

TARGET_VERSION = 6

LEGACY_TYPE_RENAMES = {
    "buffer": "c_buffer",
    "carry":  "c_carry",
    "mux":    "c_mux",
    "lut":    "c_lut",
}


def _collect_pins(cell):
    """Return (input_pin_names, lut_output_pin_names) from a cell's pin_groups."""
    input_pins = []
    lut_output_pins = []
    for pg in cell.get("pin_groups", []):
        direction = pg.get("direction", "")
        for p in pg.get("pins", []):
            name = p.get("name", "")
            if direction == "input":
                input_pins.append(name)
            elif direction == "output" and p.get("type") == "lut":
                lut_output_pins.append(name)
    return input_pins, lut_output_pins


def _upgrade_lut_config(cell, cell_name, warnings):
    """
    Convert old flat lut_config to the output_pins form.
    Returns a new lut_config dict, or None if no change is needed / possible.
    """
    lut_cfg = cell["lut_config"]

    if "output_pins" in lut_cfg:
        return None  # already new format

    if "data_identifier" not in lut_cfg:
        return None  # nothing to upgrade

    data_identifier = lut_cfg["data_identifier"]
    bit_order       = lut_cfg.get("bit_order", "ascending")
    is_ascending    = (bit_order == "ascending")

    input_pins, lut_output_pins = _collect_pins(cell)

    if not lut_output_pins:
        warnings.append(
            f"  WARNING [{cell_name}]: no output pin with type 'lut' found — "
            f"lut_config not migrated, please update manually."
        )
        return None

    if not input_pins:
        warnings.append(
            f"  WARNING [{cell_name}]: no input pins found — "
            f"lut_config not migrated, please update manually."
        )
        return None

    bit_count = 1 << len(input_pins)

    new_lut_cfg = {}
    if "bit_order" in lut_cfg:
        new_lut_cfg["bit_order"] = lut_cfg["bit_order"]
    if "data_category" in lut_cfg:
        new_lut_cfg["data_category"] = lut_cfg["data_category"]

    if len(lut_output_pins) > 1:
        # Multiple outputs with potentially different input subsets or INIT slices
        # (e.g. LUT6_2 where O6 uses 6 inputs/64 bits and O5 uses 5 inputs/32 bits).
        # There is no way to determine the correct per-output input list and bit range
        # from the pin structure alone, so manual migration is required.
        warnings.append(
            f"  WARNING [{cell_name}]: has {len(lut_output_pins)} LUT output pins "
            f"({', '.join(lut_output_pins)}). Multi-output LUT cells require manual "
            f"migration because the per-output input list and bit range cannot be "
            f"determined automatically. See XILINX_UNISIM.hgl (LUT6_2) for a reference."
        )
        return None
    else:
        new_lut_cfg["output_pins"] = [{
            "pin":             lut_output_pins[0],
            "data_identifier": data_identifier,
            "bit_offset":      0,
            "bit_count":       bit_count,
            "input_pins":      input_pins,
            "is_ascending":    is_ascending,
        }]

    return new_lut_cfg


def _make_parameters(cell):
    """
    Derive the 'parameters' list for a cell based on its config sections.
    Returns a list of parameter dicts (possibly empty).
    """
    params = []
    seen_identifiers = {}  # ident -> required_size

    if "lut_config" in cell:
        lut_cfg = cell["lut_config"]
        if "output_pins" in lut_cfg:
            for op in lut_cfg["output_pins"]:
                ident    = op["data_identifier"]
                required = op["bit_offset"] + op["bit_count"]
                seen_identifiers[ident] = max(seen_identifiers.get(ident, 0), required)

    if "ff_config" in cell:
        ff = cell["ff_config"]
        if "data_identifier" in ff:
            ident = ff["data_identifier"]
            seen_identifiers.setdefault(ident, 1)

    if "latch_config" in cell:
        latch = cell["latch_config"]
        if "data_identifier" in latch:
            ident = latch["data_identifier"]
            seen_identifiers.setdefault(ident, 1)

    if "ram_config" in cell:
        ram = cell["ram_config"]
        if "data_identifiers" in ram and "bit_size" in ram:
            idents   = ram["data_identifiers"]
            per_size = ram["bit_size"] // len(idents) if idents else 0
            for ident in idents:
                seen_identifiers[ident] = per_size

    for ident, size in seen_identifiers.items():
        params.append({
            "name":    ident,
            "type":    "bit_vector",
            "size":    size,
            "default": "0x0",
        })

    return params


def _upgrade_cell(cell, warnings):
    """Upgrade a single cell dict in-place. Returns True if the cell was modified."""
    modified  = False
    cell_name = cell.get("name", "(unknown)")

    # 1. Rename legacy gate-type property strings.
    if "types" in cell:
        new_types = [LEGACY_TYPE_RENAMES.get(t, t) for t in cell["types"]]
        if new_types != cell["types"]:
            cell["types"] = new_types
            modified = True

    # 2. Upgrade flat lut_config to output_pins format.
    if "lut_config" in cell:
        new_cfg = _upgrade_lut_config(cell, cell_name, warnings)
        if new_cfg is not None:
            cell["lut_config"] = new_cfg
            modified = True

    # 3. Add 'parameters' section if absent.
    if "parameters" not in cell:
        params = _make_parameters(cell)
        if params:
            cell["parameters"] = params
            modified = True

    return modified


def upgrade_library(data):
    """
    Upgrade a parsed HGL dict to version 6.
    Returns (upgraded_data, warnings_list).
    """
    warnings = []
    version  = data.get("version", 1)

    if version >= TARGET_VERSION:
        return data, warnings

    if version < 2:
        warnings.append(
            f"  WARNING: version {version} uses a legacy pin/group format (pre pin_groups) "
            f"that this script does not support.  Only files at version >= 2 can be "
            f"auto-upgraded."
        )
        return data, warnings

    result          = copy.deepcopy(data)
    result["version"] = TARGET_VERSION

    for cell in result.get("cells", []):
        _upgrade_cell(cell, warnings)

    return result, warnings


def main():
    parser = argparse.ArgumentParser(
        description="Upgrade HGL gate library files to version 6.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    parser.add_argument(
        "inputs", nargs="+", metavar="FILE_OR_DIR",
        help="HGL file(s) or directory containing HGL files",
    )
    parser.add_argument(
        "--in-place", action="store_true",
        help="Overwrite input files instead of writing to FILE.v6.hgl",
    )
    args = parser.parse_args()

    # Collect all .hgl paths.
    files = []
    for inp in args.inputs:
        if os.path.isdir(inp):
            for fn in sorted(os.listdir(inp)):
                if fn.endswith(".hgl"):
                    files.append(os.path.join(inp, fn))
        elif os.path.isfile(inp):
            files.append(inp)
        else:
            print(f"ERROR: '{inp}' is not a file or directory", file=sys.stderr)
            sys.exit(1)

    if not files:
        print("No .hgl files found.", file=sys.stderr)
        sys.exit(1)

    exit_code = 0
    for path in files:
        with open(path, "r", encoding="utf-8") as f:
            try:
                data = json.load(f)
            except json.JSONDecodeError as e:
                print(f"ERROR: {path}: invalid JSON: {e}", file=sys.stderr)
                exit_code = 1
                continue

        version = data.get("version", 1)
        if version >= TARGET_VERSION:
            print(f"SKIP  {path}  (already version {version})")
            continue

        upgraded, warnings = upgrade_library(data)

        for w in warnings:
            print(w)

        if args.in_place:
            out_path = path
        else:
            base, _ = os.path.splitext(path)
            out_path = base + ".v6.hgl"

        with open(out_path, "w", encoding="utf-8") as f:
            json.dump(upgraded, f, indent=4)
            f.write("\n")

        print(f"OK    {path}  (v{version} -> v{TARGET_VERSION})  ->  {out_path}")

    sys.exit(exit_code)


if __name__ == "__main__":
    main()
