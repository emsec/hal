from hal_plugins import hawkeye

config_scc = hawkeye.DetectionConfiguration()
config_scc.control = hawkeye.DetectionConfiguration.Control.CHECK_NETS
config_scc.components = hawkeye.DetectionConfiguration.Components.NONE
config_scc.timeout = 10
config_scc.min_register_size = 10

min_state_size = 40
gate_ids = []

res = hawkeye.CipherCandidate.detect(
    netlist,
    [config_scc],
    min_state_size,
    [netlist.get_gate_by_id(gid) for gid in gate_ids],
)

print(f"found {len(res)} candidates: {[c.get_size() for c in res]}")

for candidate in res:
    print(candidate.get_size(), candidate.get_output_reg()[0].name)

    if not candidate.build_round_function():
        print("skipped candidate")
        continue

    # do something

print("done")
