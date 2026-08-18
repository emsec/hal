#!/usr/bin/env python3
"""Write a canonical description of what HAWKEYE finds in a set of benchmark netlists.

Intended to pin down the current behavior before a refactoring and to compare against
afterwards, so run it once before the change and once after and diff the two files.

Unlike the summary printed by the benchmark scripts of the artifact, this records the
gates of every candidate and of every S-box rather than only their sizes, so that a
candidate whose contents change while its size stays the same does not slip through.
Everything is keyed and ordered by gate ID, never by iteration order of a `std::set`
of pointers, so that the output does not depend on where the gates happen to be
allocated.

Usage:
    hal --python-script characterize.py --python-args "<benchmark_dir> <sbox_db> <out_file> [--identify]"

or, running the HAL Python module directly:

    HAL_BASE_PATH=<build_dir> PYTHONPATH=<build_dir>/lib \\
        python3 characterize.py <benchmark_dir> <sbox_db> <out_file>

The benchmark directory is expected to hold the `.json` benchmark descriptions of the
HAWKEYE artifact together with the netlists they refer to.
"""

import builtins
import json
import sys
from pathlib import Path

import hal_py
from hal_plugins import hawkeye, netlist_preprocessing, xilinx_toolbox

MIN_STATE_SIZE = 40
MIN_REGISTER_SIZE = 10
TIMEOUT = 10


def preprocess(nl):
    """Apply the same clean-up the artifact scripts apply before running HAWKEYE."""
    netlist_preprocessing.remove_unconnected_gates(nl)
    xilinx_toolbox.split_luts(nl)
    netlist_preprocessing.simplify_lut_inits(nl)
    netlist_preprocessing.remove_unused_lut_inputs(nl)
    netlist_preprocessing.remove_buffers(nl)
    netlist_preprocessing.unify_ff_outputs(nl)


def ids(gates):
    """The IDs of the given gates in ascending order."""
    return sorted(g.id for g in gates)


def make_configs(is_fpga):
    """The two detection methods used to evaluate the artifact."""
    c1a = hawkeye.DetectionConfiguration()
    c1a.control = hawkeye.DetectionConfiguration.Control.CHECK_PINS
    c1a.components = hawkeye.DetectionConfiguration.Components.NONE
    c1a.timeout = TIMEOUT
    c1a.min_register_size = MIN_REGISTER_SIZE

    c1b = hawkeye.DetectionConfiguration()
    c1b.control = hawkeye.DetectionConfiguration.Control.CHECK_NETS
    c1b.components = hawkeye.DetectionConfiguration.Components.NONE
    c1b.timeout = TIMEOUT
    c1b.min_register_size = MIN_REGISTER_SIZE

    c2 = hawkeye.DetectionConfiguration()
    c2.control = hawkeye.DetectionConfiguration.Control.CHECK_TYPE
    c2.components = hawkeye.DetectionConfiguration.Components.CHECK_SCC
    c2.equivalent_types = [["FDRE", "FDSE"]] if is_fpga else [["FD1", "FD1P"]]
    c2.timeout = TIMEOUT
    c2.min_register_size = MIN_REGISTER_SIZE

    return {"method_1": [c1a, c1b], "method_2": [c2]}


def describe_sboxes(candidate, sbox_db, identify):
    """Describe every S-box of a candidate, ordered so that the result is stable.

    Identification is optional because it dominates the runtime by a wide margin: the S-box
    search deliberately over-produces, and identifying every one of its results costs many
    times what locating them does. The benchmark scripts of the artifact avoid this by
    stopping at the first S-box they identify, which is why they complete quickly.

    What identification produces is already covered by running those benchmark scripts, so
    leave it off to characterize the structure that this script exists for, and compare the
    S-box names by running the benchmark scripts instead.
    """
    sboxes = candidate.locate_sboxes()
    if sboxes is None:
        return None

    described = []
    for sbox in sboxes:
        name = candidate.identify_sbox(sbox, sbox_db) if identify else None
        described.append(
            {
                "component": ids(sbox.component),
                "input_gates": ids(sbox.input_gates),
                "output_gates": ids(sbox.output_gates),
                "identified_as": (name if name else "") if identify else "not identified",
            }
        )

    # order by contents rather than by discovery order
    described.sort(key=lambda s: (s["input_gates"], s["output_gates"], s["component"]))
    return described


def describe_candidate(candidate, sbox_db, state_regs, has_sbox, identify):
    """Describe a candidate together with its round function and S-boxes.

    S-boxes are only located for a candidate whose output register is one of the known state
    registers of the benchmark, which is the same rule the benchmark scripts of the artifact
    apply. Locating and identifying S-boxes within a candidate that is a false positive costs
    far more than the whole rest of the analysis, as the register of such a candidate can be
    an order of magnitude wider than any real cipher state.
    """
    output_reg = ids(candidate.get_output_reg())
    is_state_reg = output_reg in state_regs

    described = {
        "size": candidate.get_size(),
        "is_round_based": candidate.is_round_based(),
        "input_reg": ids(candidate.get_input_reg()),
        "output_reg": output_reg,
        "is_state_reg": is_state_reg,
    }

    if not candidate.build_round_function():
        described["round_function"] = None
        return described

    described["round_function"] = {
        "state_logic": ids(candidate.get_round_logic()),
        "input_reg": ids(candidate.get_input_reg()),
        "output_reg": ids(candidate.get_output_reg()),
        "num_state_inputs": len(candidate.get_state_inputs()),
        "num_control_inputs": len(candidate.get_control_inputs()),
        "num_other_inputs": len(candidate.get_other_inputs()),
        "num_state_outputs": len(candidate.get_state_outputs()),
    }

    if not is_state_reg:
        described["sboxes"] = "skipped: not a known state register"
    elif not has_sbox:
        described["sboxes"] = "skipped: benchmark has no S-box"
    else:
        described["sboxes"] = describe_sboxes(candidate, sbox_db, identify)

    return described


def characterize(benchmark_dir, sbox_db_path, identify):
    sbox_db = hawkeye.SBoxDatabase.from_file(sbox_db_path)
    result = {}

    root = Path(benchmark_dir)
    # follow symbolic links, so that a set of benchmarks can be assembled by linking directories together
    descriptions = sorted(p for p in root.glob("**/*.json") if p.is_file())
    descriptions += sorted(p for d in root.iterdir() if d.is_symlink() and d.is_dir() for p in sorted(d.glob("**/*.json")))

    # HAL injects its own names into the namespace of a script it runs, among them a `set` that shadows the
    # built-in one, so reach for the built-in explicitly
    for description in sorted(builtins.set(descriptions)):
        with open(description) as ifile:
            benchmark = json.load(ifile)

        # benchmark names are only unique within a category, the same cipher exists as an ASIC and as an FPGA netlist,
        # so key the result by the path of the description relative to the benchmark directory
        try:
            name = str(description.relative_to(root).with_suffix(""))
        except ValueError:
            name = str(Path(description.parent.name) / description.stem)
        netlist_path = description.parent / benchmark["fpga_netlist"]
        gate_library = benchmark["gate_library"]

        gl = hal_py.GateLibraryManager.get_gate_library(gate_library)
        nl = hal_py.NetlistFactory.load_netlist(str(netlist_path), gl)
        if nl is None:
            result[name] = {"error": "could not load netlist"}
            continue
        preprocess(nl)

        # the known state registers of the benchmark, as sorted ID lists, to recognize a true positive
        state_regs = [sorted(reg) for reg in benchmark.get("state_reg_ids", [])]
        has_sbox = benchmark.get("has_sbox", False)

        entry = {"gate_library": gate_library, "num_gates": len(nl.get_gates())}
        for method, configs in make_configs("UNISIM" in gate_library).items():
            candidates = hawkeye.CipherCandidate.detect(nl, configs, MIN_STATE_SIZE)
            if candidates is None:
                entry[method] = {"error": "detection failed"}
                continue

            described = [describe_candidate(c, sbox_db, state_regs, has_sbox, identify) for c in candidates]
            # candidates come back in the order of the analysis, sort them by contents
            described.sort(key=lambda c: (-c["size"], c["input_reg"], c["output_reg"]))
            entry[method] = described

        result[name] = entry
        print(f"characterized {name}", file=sys.stderr)

    return result


def main():
    # HAL hands the script only the arguments given to --python-args, while running it as a program puts the
    # path of the script in front of them, so drop that rather than counting on either invocation
    args = [arg for arg in sys.argv if not arg.endswith("characterize.py")]
    identify = "--identify" in args
    args = [arg for arg in args if arg != "--identify"]

    if len(args) != 3:
        print(__doc__)
        return 1

    benchmark_dir, sbox_db_path, out_file = args

    # the gate library and netlist parsers are plugins, so they have to be loaded before a netlist can be read
    hal_py.plugin_manager.load_all_plugins()

    log_mng = hal_py.LogManager()
    log_mng.deactivate_all_channels()

    result = characterize(benchmark_dir, sbox_db_path, identify)

    # an empty result is always a mistake and writing it out is dangerous, as comparing against an empty
    # characterization succeeds no matter what the code under test does
    if not result:
        print(f"error: no benchmarks found below '{benchmark_dir}', refusing to write an empty characterization", file=sys.stderr)
        return 1

    with open(out_file, "w") as ofile:
        json.dump(result, ofile, indent=2, sort_keys=True)

    print(f"characterized {len(result)} benchmarks, wrote {out_file}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
