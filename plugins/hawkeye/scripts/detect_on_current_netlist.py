from hal_plugins import hawkeye

config_scc = hawkeye.DetectionConfiguration()
config_scc.control = hawkeye.DetectionConfiguration.Control.CHECK_NETS
config_scc.components = hawkeye.DetectionConfiguration.Components.NONE
config_scc.timeout = 10
config_scc.min_register_size = 10

min_state_size = 40
gate_ids = []

res = hawkeye.detect_candidates(
    netlist,
    [config_scc],
    min_state_size,
    [netlist.get_gate_by_id(gid) for gid in gate_ids],
)

print(f"found {len(res)} candidates: {[len(c.get_output_reg()) for c in res]}")

for rc in res:
    print(len(rc.get_output_reg()), list(rc.get_output_reg())[0].name)

    sc = hawkeye.StateCandidate.from_register_candidate(rc)
    if sc == None:
        print("skipped register candidate")
        continue

    # do something

print("done")
