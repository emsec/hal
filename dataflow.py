from hal_plugins import dataflow

# create dataflow analysis config
config = dataflow.Dataflow.Configuration().with_expected_sizes([8])

# run dataflow analysis
res = dataflow.Dataflow.analyze(netlist)

# create modules for groups containing more than three gates
groups = res.get_groups()
group_ids = {g_id for g_id in groups if len(groups[g_id]) > 3}
res.create_modules(group_ids)