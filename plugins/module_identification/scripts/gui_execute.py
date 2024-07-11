from hal_plugins import module_identification

for mod in netlist.get_modules():
    netlist.delete_module(mod)

config = module_identification.module_identification.Configuration(netlist)
config.multithreading_handling = module_identification.MultithreadingHandling.onlyOuterCandidates
config.maxControlSignals = 4
#config.multithreading_handling = module_identification.MultithreadingHandling.none
#config.to_check_types = [module_identification.CandidateType.value_check]
#config.to_check_types = [module_identification.CandidateType.counter]
#config.to_check_types = list()

res = module_identification.module_identification.execute(config)
res.create_modules_in_netlist()
