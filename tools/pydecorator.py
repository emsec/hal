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
			log_string = "Function: " + message + " , Module-IDs: {"
			if len(result) == 0:
				log_string += "empty}"
			else:
				sorted_modules = sorted(result, key=lambda module: module.id)
				log_string += "".join([str(mod.id) + ", " for mod in sorted_modules])[:-2] + "}"
			hal_py.log_info(log_string)
			return result
		return decorated

	def netlist_create_grouping(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			log_string = "Function: {}, Grouping-Id: ".format(message) + str(result.id)
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

	def net_get_source(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			if result.gate is None:
				hal_py.log_info("Function: {}".format(message) + ", Gate-ID: {empty}")
			else:
				hal_py.log_info("Function: {}, Gate-ID: {}".format(message, result.gate.id))
			return result
		return decorated

	def net_get_destinations_get_sources(message, f):
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

	######## Module and Grouping Decorator (similiar structures)

	def module_grouping_assign_gate(message, object_type, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			assigned_gate = kwargs.get("gate")
			if assigned_gate is None:
				assigned_gate = args[1]
			hal_py.log_info("Function: {}, {}-ID: {}, Gate-ID: {}".format(message, object_type, args[0].id, assigned_gate.id))
			return result
		return decorated

	def module_grouping_get_gates(message, obect_type, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			log_string = "Function: " + message + ", " + str(obect_type) + "-ID: " + str(args[0].id) + ", Gate-IDs: {"
			if len(result) == 0:
				log_string += "empty}"
			else:
				sorted_gates = sorted(result) if isinstance(result[0], int) else sorted(result, key=lambda gate: gate.id)
				#sorted_gates = sorted(result, key=lambda gate: gate.id)
				if isinstance(result[0], int):
					log_string += "".join([str(g) + ", " for g in sorted_gates])[:-2] + "}"
				else:
					log_string += "".join([str(g.id) + ", " for g in sorted_gates])[:-2] + "}"
			hal_py.log_info(log_string)
			return  result
		return decorated

	def module_grouping_remove_gate(message, object_type, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			gate = kwargs.get("gate")
			if gate is None:
				gate = args[1]
			log_string = "Function: " + message + ", " + object_type + "-ID: " + str(args[0].id) + ", Gate-ID: " + str(gate.id)
			hal_py.log_info(log_string)
			return result
		return decorated

	############# Grouping specific Decorator
	def grouping_assign_gate_by_id(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			assigned_gate_id = kwargs.get("gate_id")
			if assigned_gate_id is None:
				assigned_gate_id = args[1]
			hal_py.log_info("Function: {}, Grouping-ID: {}, Gate-ID: {}".format(message, args[0].id, assigned_gate_id))
			return result
		return decorated

	def grouping_remove_gate_by_id(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			gate_id = kwargs.get("gate_id") if kwargs.get("gate_id") is not None else args[1]
			# gate_id = kwargs.get("gate_id")
			# if gate_id is None:
			# 	gate_id = args[1]
			log_string = "Function: {}, Grouping-ID: {}, Gate-ID: {}".format(message, args[0].id, gate_id)
			hal_py.log_info(log_string)
			return result
		return decorated

	def grouping_assign_net(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			net_id = -1
			if kwargs.get("net") is None:
				if kwargs.get("net_id") is None:
					net_id = args[1].id if isinstance(args[1], hal_py.Net) else args[1]
				else:
					net_id = kwargs.get("net_id")
			else:
				net_id = kwargs.get("net").id
			log_string = "Function: {}, Grouping-ID: {}, Net-ID: {}".format(message, args[0].id, net_id)
			hal_py.log_info(log_string)
			return result
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
	hal_py.Netlist.get_gate_by_id = netlist_get_gate_or_net_by_id("Netlist.get_gate_by_id", "Gate", hal_py.Netlist.get_gate_by_id)
	hal_py.Netlist.get_gates = netlist_get_gates("Netlist.get_gates", hal_py.Netlist.get_gates)
	hal_py.Netlist.get_net_by_id = netlist_get_gate_or_net_by_id("Netlist.get_net_by_id", "Net", hal_py.Netlist.get_net_by_id)
	hal_py.Netlist.get_nets = netlist_get_nets("Netlist.get_nets", hal_py.Netlist.get_nets)
	hal_py.Netlist.create_module = netlist_create_module("Netlist.create_module", hal_py.Netlist.create_module)
	hal_py.Netlist.delete_module = netlist_delete_module("Netlist.delete_module", hal_py.Netlist.delete_module)
	hal_py.Netlist.get_module_by_id = netlist_get_module_by_id("Netlist.get_module_by_id", hal_py.Netlist.get_module_by_id)
	hal_py.Netlist.get_modules = netlist_get_modules("Netlist.get_modules", hal_py.Netlist.get_modules)
	hal_py.Netlist.create_grouping = netlist_create_grouping("Netlist.create_grouping", hal_py.Netlist.create_grouping)

	####### Gate Functions
	hal_py.Gate.get_name = generic_decorator("Gate.get_name", "Gate", hal_py.Gate.get_name)
	hal_py.Gate.get_id = generic_decorator("Gate.get_id", "Gate", hal_py.Gate.get_id)
	hal_py.Gate.get_type = generic_decorator("Gate.get_type", "Gate", hal_py.Gate.get_type)
	hal_py.Gate.get_data_by_key = generic_decorator("Gate.get_data_by_key", "Gate", hal_py.Gate.get_data_by_key)
	hal_py.Gate.set_data = generic_decorator("Gate.set_data", "Gate", hal_py.Gate.set_data)
	hal_py.Gate.set_name = generic_decorator("Gate.set_name", "Gate", hal_py.Gate.set_name)
	hal_py.Gate.get_predecessors = gate_get_predecessors_or_successors("Gate.get_predecessors", "Predecessor", hal_py.Gate.get_predecessors)
	hal_py.Gate.get_successors = gate_get_predecessors_or_successors("Gate.get_successors", "Successor", hal_py.Gate.get_successors)
	hal_py.Gate.get_predecessor = gate_get_predecessor("Gate.get_predecessor", hal_py.Gate.get_predecessor)

	####### Net Functions
	hal_py.Net.get_id = generic_decorator("Net.get_id", "Net", hal_py.Net.get_id)
	hal_py.Net.get_name = generic_decorator("Net.get_name", "Net", hal_py.Net.get_name)
	hal_py.Net.set_data = generic_decorator("Net.set_data", "Net", hal_py.Net.set_data)
	hal_py.Net.set_name = generic_decorator("Net.set_name", "Net", hal_py.Net.set_name)
	hal_py.Net.get_source = net_get_source("Net.get_source", hal_py.Net.get_source)
	hal_py.Net.get_sources = net_get_destinations_get_sources("Net.get_sources", hal_py.Net.get_sources)
	hal_py.Net.get_destinations = net_get_destinations_get_sources("Net.get_destinations", hal_py.Net.get_destinations)

	####### Module Functions
	hal_py.Module.assign_gate = module_grouping_assign_gate("Module.assign_gate", "Module", hal_py.Module.assign_gate)
	hal_py.Module.get_gates = module_grouping_get_gates("Module.get_gates", "Module", hal_py.Module.get_gates)
	hal_py.Module.remove_gate = module_grouping_remove_gate("Module.remove_gate", "Module", hal_py.Module.remove_gate)

	###### Endpoint Functions
	hal_py.Endpoint.get_gate = endpoint_get_gate("Endpoint.get_gate", hal_py.Endpoint.get_gate)

	###### Grouping Functions
	hal_py.Grouping.get_id = generic_decorator("Grouping.get_id", "Grouping", hal_py.Grouping.get_id)
	hal_py.Grouping.get_name = generic_decorator("Grouping.get_name", "Grouping", hal_py.Grouping.get_name)
	hal_py.Grouping.get_netlist = generic_decorator("Grouping.get_netlist", "Grouping", hal_py.Grouping.get_netlist)
	hal_py.Grouping.set_name = generic_decorator("Grouping.set_name", "Grouping", hal_py.Grouping.set_name)
	hal_py.Grouping.assign_gate = module_grouping_assign_gate("Grouping.assign_gate", "Grouping", hal_py.Grouping.assign_gate)
	hal_py.Grouping.assign_gate_by_id = grouping_assign_gate_by_id("Grouping.assign_gate_by_id", hal_py.Grouping.assign_gate_by_id)
	hal_py.Grouping.get_gates = module_grouping_get_gates("Grouping.get_gates", "Grouping", hal_py.Grouping.get_gates)
	hal_py.Grouping.get_gate_ids = module_grouping_get_gates("Grouping.get_gate_ids", "Grouping", hal_py.Grouping.get_gate_ids)
	hal_py.Grouping.remove_gate = module_grouping_remove_gate("Grouping.remove_gate", "Grouping", hal_py.Grouping.remove_gate)
	hal_py.Grouping.remove_gate_by_id = grouping_remove_gate_by_id("Grouping.remove_gate_by_id", hal_py.Grouping.remove_gate_by_id)
	hal_py.Grouping.assign_net = grouping_assign_net("Grouping.assign_net", hal_py.Grouping.assign_net)
	hal_py.Grouping.assign_net_by_id = grouping_assign_net("Grouping.assign_net_by_id", hal_py.Grouping.assign_net_by_id)

else:
	hal_py.log_info("Already decorated. Not applying again.")
