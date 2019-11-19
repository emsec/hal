if "__decorated__" not in dir():
	__decorated__ = True

	from functools import wraps
	import hal_py

	# Common valid object types are, for example, Net and Gate
	def generic_decorator(message, object_type, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			hal_py.log_info("Function: {}, {}-ID: {}".format(message, object_type, args[0].id))
			return result
		return decorated

	#######################Specific Functions

	#######Netlist Decorator

	# you could get the net- or gate-id via the parameters (kwargs.get("gate_id/net_id") or args[1]),
	# but you can also grab the id from the result. if the result is a nullpointer(None), the id is -1
	def netlist_get_gate_or_net_by_id(message, object_type, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			result_id = -1
			if result is not None:
				result_id = result.id
			hal_py.log_info("Function: {}, {}-ID: {}".format(message, object_type, result_id))
			return result
		return decorated

	def netlist_get_gates(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			log_string = "Function: " + message + ", Gate-IDs: {"
			if len(result) == len(args[0].gates):
				log_string += "all}"
			elif len(result) == 0:
				log_string += "empty}"
			else:
				sorted_result = sorted(result, key=lambda gate: gate.id)
				log_string += "".join([str(g.id) + ", " for g in sorted_result])[:-2] + "}"
			hal_py.log_info(log_string)
			return result
		return decorated

	def netlist_get_nets(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			log_string = "Function: " + message + ", Net-IDs: {"
			if len(result) == len(args[0].nets):
				log_string += "all}"
			elif len(result) == 0:
				log_string += "empty}"
			else:
				sorted_result = sorted(result, key=lambda net: net.id)
				log_string += "".join([str(n.id) + ", " for n in sorted_result])[:-2] + "}"
			hal_py.log_info(log_string)
			return result
		return decorated

	def netlist_create_module(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			hal_py.log_info("Function: {}, Module-ID: {}".format(message, result.id))
			return result
		return decorated

	def netlist_delete_module(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			module_to_delete = kwargs.get("module")
			if module_to_delete is None:
				module_to_delete = args[1]
			module_id = module_to_delete.id
			gate_ids = module_to_delete.gates
			result = f(*args, **kwargs)
			log_string = "Function: " + message + ", Module-ID: " + str(module_id) + ", Gate-IDs: {"
			if len(gate_ids) != 0:
				sorted_gate_ids = sorted(gate_ids, key=lambda gate: gate.id)
				log_string += "".join([str(g.id) + ", " for g in sorted_gate_ids])[:-2] + "}"
			else:
				log_string += "empty}"
			hal_py.log_info(log_string)
			return result
		return decorated

	def netlist_get_module_by_id(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			gate_ids = result.gates
			module_id = result.id
			log_string = "Function: " + message + ", Module-ID: " + str(module_id) + ", Gate-IDs: {"
			if len(gate_ids) != 0:
				sorted_gate_ids = sorted(gate_ids, key=lambda gate: gate.id)
				log_string += "".join([str(g.id) + ", " for g in sorted_gate_ids])[:-2] + "}"
			else:
				log_string += "empty}"
			hal_py.log_info(log_string)
			return result
		return decorated

	def netlist_get_modules(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			log_string = "Function called: " + message + " , Module-IDs: {"
			if len(result) == 0:
				log_string += "empty}"
			else:
				sorted_modules = sorted(result, key=lambda module: module.id)
				log_string += "".join([str(mod.id) + ", " for mod in sorted_modules])[:-2] + "}"
			hal_py.log_info(log_string)
			return result
		return decorated

	######### Gate Decorator
	def gate_get_predecessors_or_successors(message, predecessors_or_successors, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			log_string = "Function: " + message + ", Gate-ID: " + str(args[0].id) + ", " + predecessors_or_successors + "-IDs: {"
			if len(result) != 0:
				sorted_endpoints = sorted(result, key=lambda ep: ep.gate.id)
				log_string += "".join([str(endpoint.gate.id) + ", " for endpoint in sorted_endpoints])[:-2] + "}"
			else:
				log_string += "empty}"
			hal_py.log_info(log_string)
			return result
		return decorated

	def gate_get_predecessor(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			result_gate = result.gate
			if result_gate is not None:
				hal_py.log_info("Function: {}, Gate-ID: {}, Predecessor-ID: {}".format(message, str(args[0].id), result_gate.id))
			else:
				hal_py.log_info("Function: {}, Gate-ID: {}, Predecessor-ID: -1".format(message, str(args[0].id)))
			return result
		return decorated


	########## Net Decorator

	def net_get_src(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			if result.gate is None:
				hal_py.log_info("Function: {}".format(message) + ", Gate-ID: {empty}")
			else:
				hal_py.log_info("Function: {}, Gate-ID: {}".format(message, result.gate.id))
			return result
		return decorated

	def net_get_dests(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			log_string = "Function: " + message + ", Gate-IDs: {"
			if len(result) == 0:
				log_string += "empty}"
			else:
				sorted_endpoints = sorted(result, key=lambda ep: ep.gate.id)
				log_string += "".join([str(endpoint.gate.id) + ", " for endpoint in sorted_endpoints])[:-2] + "}"
			hal_py.log_info(log_string)
			return result
		return decorated

	######## Module Decorator

	def module_assign_gate(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			assigned_gate = kwargs.get("gate")
			if assigned_gate is None:
				assigned_gate = args[1]
			hal_py.log_info("Function: {}, Module-ID: {}, Gate-ID: {}".format(message, args[0].id, assigned_gate.id))
			return result
		return decorated

	def module_gates(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			log_string = "Function: " + message + ", Module-ID: " + str(args[0].id) + ", Gate-IDs: {"
			if len(result) == 0:
				log_string += "empty}"
			else:
				sorted_gates = sorted(result, key=lambda gate: gate.id)
				log_string += "".join([str(g.id) + ", " for g in sorted_gates])[:-2] + "}"
			hal_py.log_info(log_string)
			return  result
		return decorated

	######### Endpoint Decorator

	def endpoint_get_gate(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			if result is not None:
				hal_py.log_info("Function: {}, Gate-ID: {}".format(message, result.id))
			else:
				hal_py.log_info("Function: {}, Gate-ID: -1".format(message))
			return result
		return decorated


	##### Decorate actual functions

	####### Netlist functions
	hal_py.netlist.get_gate_by_id = netlist_get_gate_or_net_by_id("netlist.get_gate_by_id", "Gate", hal_py.netlist.get_gate_by_id)
	hal_py.netlist.get_gates = netlist_get_gates("netlist.get_gates", hal_py.netlist.get_gates)
	hal_py.netlist.get_net_by_id = netlist_get_gate_or_net_by_id("netlist.get_net_by_id", "Net", hal_py.netlist.get_net_by_id)
	hal_py.netlist.get_nets = netlist_get_nets("netlist.get_nets", hal_py.netlist.get_nets)
	hal_py.netlist.create_module = netlist_create_module("netlist.create_module", hal_py.netlist.create_module)
	hal_py.netlist.delete_module = netlist_delete_module("netlist.delete_module", hal_py.netlist.delete_module)
	hal_py.netlist.get_module_by_id = netlist_get_module_by_id("netlist.get_module_by_id", hal_py.netlist.get_module_by_id)
	hal_py.netlist.get_modules = netlist_get_modules("netlist.get_modules", hal_py.netlist.get_modules)

	####### Gate Functions
	hal_py.gate.get_name = generic_decorator("gate.get_name", "Gate", hal_py.gate.get_name)
	hal_py.gate.get_id = generic_decorator("gate.get_id", "Gate", hal_py.gate.get_id)
	hal_py.gate.get_type = generic_decorator("gate.get_type", "Gate", hal_py.gate.get_type)
	hal_py.gate.get_data_by_key = generic_decorator("gate.get_data_by_key", "Gate", hal_py.gate.get_data_by_key)
	hal_py.gate.set_data = generic_decorator("gate.set_data", "Gate", hal_py.gate.set_data)
	hal_py.gate.set_name = generic_decorator("gate.set_name", "Gate", hal_py.gate.set_name)
	hal_py.gate.get_predecessors = gate_get_predecessors_or_successors("gate.get_predecessors", "Predecessor", hal_py.gate.get_predecessors)
	hal_py.gate.get_successors = gate_get_predecessors_or_successors("gate.get_successors", "Successor", hal_py.gate.get_successors)
	hal_py.gate.get_predecessor = gate_get_predecessor("gate.get_predecessor", hal_py.gate.get_predecessor)

	####### Net Functions
	hal_py.net.get_id = generic_decorator("net.get_id", "Net", hal_py.net.get_id)
	hal_py.net.get_name = generic_decorator("net.get_name", "Net", hal_py.net.get_name)
	hal_py.net.set_data = generic_decorator("net.set_data", "Net", hal_py.net.set_data)
	hal_py.net.set_name = generic_decorator("net.set_name", "Net", hal_py.net.set_name)
	hal_py.net.get_src = net_get_src("net.get_src", hal_py.net.get_src)
	hal_py.net.get_dsts = net_get_dests("net.get_dsts", hal_py.net.get_dsts)

	####### Module Functions
	hal_py.module.assign_gate = module_assign_gate("module.assign_gate", hal_py.module.assign_gate)
	hal_py.module.get_gates = module_gates("module.gates", hal_py.module.get_gates)

	###### Endpoint Functions
	hal_py.endpoint.get_gate = endpoint_get_gate("endpoint.get_gate", hal_py.endpoint.get_gate)

else:
	hal_py.log_info("Already decorated. Not applying again.")