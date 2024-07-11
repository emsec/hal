
from hal_plugins import module_identification

config = module_identification.module_identification.Configuration(netlist)
config.maxControlSignals = 3
config.multithreading_handling = module_identification.MultithreadingHandling.time_priority

config.to_check_types = [
            module_identification.CandidateType.adder, 
            module_identification.CandidateType.constant_multiplication, 
            module_identification.CandidateType.absolute, 
            module_identification.CandidateType.counter, 
            module_identification.CandidateType.equal, 
            module_identification.CandidateType.less_equal, 
            module_identification.CandidateType.signed_less_equal, 
            module_identification.CandidateType.value_check,
        ]

res = module_identification.module_identification.execute(config)

res.create_modules_in_netlist()

