import hal_py
import os
import pydecorator
import pydecorator

current_file_path = os.path.abspath(os.path.dirname(__file__))
relative_path_to_example = os.path.join(current_file_path, "../examples/fsm.v")
relative_path_to_example_hal = os.path.join(current_file_path, "../examples/fsm.hal")
#netlist = NetlistFactory.load_netlist(relative_path_to_example, "verilog", "EXAMPLE_LIB")
netlist = NetlistFactory.load_netlist(relative_path_to_example_hal)

gate_id_test = 6
net_id_test = 15

log_info("#############################################")
log_info("NETLIST FUNCTIONS TESTING")
log_info("#############################################\n")

print(type(netlist))

log_info("Calling function get_gate_by_id with NAMED-parameter id = {}".format(gate_id_test))
test_gate = netlist.get_gate_by_id(gate_id = gate_id_test)
log_info("--------------------------------------------")

log_info("Calling function get_gate_by_id with  id = {}".format(gate_id_test))
test_gate = netlist.get_gate_by_id(gate_id_test)
log_info("--------------------------------------------")

log_info("Calling function get_net_by_id with NAMED-parameter id = {}".format(net_id_test))
test_net = netlist.get_net_by_id(net_id = net_id_test)
log_info("--------------------------------------------")

log_info("Calling function get_net_by_id with id = {}".format(net_id_test))
test_net = netlist.get_net_by_id(net_id_test)
log_info("--------------------------------------------")

log_info("Calling function get_gates")
netlist.get_gates()
log_info("--------------------------------------------")

log_info("Calling function get_nets")
netlist.get_nets()
log_info("--------------------------------------------")

log_info("Calling function create_module")
test_module = netlist.create_module("hey", netlist.get_top_module())
log_info("--------------------------------------------")

log_info("Calling function get_modules")
netlist.get_modules()
log_info("--------------------------------------------")

log_info("Calling function delete_module")
netlist.delete_module(module = test_module)
log_info("--------------------------------------------")

log_info("Calling function get_module_by_id")
netlist.get_module_by_id(1)
log_info("--------------------------------------------")

log_info("Calling function create_module")
test_module = netlist.create_module("hey", netlist.get_top_module())
log_info("--------------------------------------------")


log_info("#############################################")
log_info("GATE FUNCTIONS TESTS")
log_info("#############################################\n")

log_info("Calling function get_id")
test_gate.get_id()
log_info("--------------------------------------------")

log_info("Calling function get_name")
test_gate.get_name()
log_info("--------------------------------------------")

log_info("Calling function get_type")
test_gate.get_type()
log_info("--------------------------------------------")

log_info("Calling function get_data_by_key")
test_gate.get_data_by_key("generic","init")
log_info("--------------------------------------------")

log_info("Calling function set_data")
test_gate.set_data("category_0", "key_0", "data_type_0", "value_0")
log_info("--------------------------------------------")

log_info("Calling function set_name")
test_gate.set_name("TEST_GATE_NAME")
log_info("--------------------------------------------")

log_info("Calling function get_predecessors")
test_gate.get_predecessors()
log_info("--------------------------------------------")

log_info("Calling function get_predecessor")
test_gate.get_predecessor("I0")
log_info("--------------------------------------------")

log_info("Calling function get_successors")
test_gate.get_successors()
log_info("--------------------------------------------")

log_info("#############################################")
log_info("NET FUNCTIONS TESTS")
log_info("#############################################\n")

log_info("Calling function get_id")
test_net.get_id()
log_info("--------------------------------------------")

log_info("Calling function get_name")
test_net.get_name()
log_info("--------------------------------------------")

log_info("Calling function set_data")
test_net.set_data("category_0", "key_0", "data_type_0", "value_0")
log_info("--------------------------------------------")

log_info("Calling function get_source")
test_net.get_source()
log_info("--------------------------------------------")

log_info("Calling function get_sources")
test_net.get_sources()
log_info("--------------------------------------------")

log_info("Calling function get_destinations")
test_net.get_destinations()
log_info("--------------------------------------------")

log_info("#############################################")
log_info("MODULE FUNCTIONS TESTING")
log_info("#############################################\n")

log_info("Calling function assign_gate")
test_module.assign_gate(test_gate)
log_info("--------------------------------------------")

test_module.remove_gate(test_gate)

log_info("Calling function assign_gate with NAMED-parameter")
test_module.assign_gate(gate = test_gate)
log_info("--------------------------------------------")

log_info("Calling function get_gates")
test_module.get_gates()
log_info("--------------------------------------------")

log_info("#############################################")
log_info("ENDPOINT FUNCTION TESTING")
log_info("#############################################\n")
endpoint = test_gate.get_successors()[0]

log_info("Calling function get_gate")
endpoint.get_gate()
log_info("--------------------------------------------")
