if "__decorated_gui__" not in dir():
    __decorated_gui__ = True

    import hal_gui
    import hal_py
    from functools import wraps

    # generic decorator for select_gate, select_net and select_module as well as the deselect counterparts
    def generic_select_gate_net_module(message, type, f):
        @wraps(f)
        def decorated(*args, **kwargs):
            result = f(*args, **kwargs)
            log_string = "Function: {}, {}-ID(s): {{".format(message, type)

            possible_arg_names = ["gate", "net", "module", "gates", "nets", "modules", "gate_id", "net_id", "module_id", "gate_ids", "net_ids", "module_ids"]
            item_to_select = None

            for arg_name in possible_arg_names:
                if kwargs.get(arg_name) is not None:
                    item_to_select = kwargs.get(arg_name)
                    break;

            if item_to_select is None:
                item_to_select = args[1]

            if isinstance(item_to_select, list):
                if len(item_to_select) > 0 and isinstance(item_to_select[0], int):
                    sorted_list = sorted(item_to_select)
                    log_string += "".join([str(id) + ", " for id in sorted_list])[:-2] + "}"
                else:
                    sorted_list = sorted(item_to_select, key=lambda gate: gate.id)
                    log_string += "".join([str(g.id) + ", " for g in sorted_list])[:-2] + "}"
            else:
                if isinstance(item_to_select, int):
                    log_string += str(item_to_select) + "}"
                else:
                    log_string += str(item_to_select.id) + "}"

            hal_py.log_info(log_string)
            return result
        return decorated

    #decorate main getter functions (except get_selected_items)
    def generic_get_selected_ids_decorator(message, type, f):
        @wraps(f)
        def decorated(*args, **kwargs):
            result = f(*args, **kwargs)
            log_string = "Function: {}, {}-ID(s): {{".format(message, type)
            if len(result) == 0:
                log_string += "}"
            else:
                if isinstance(result[0], int):
                    log_string += "".join([str(id) +", " for id in sorted(result)])[:-2] + "}"
                else:
                    id_list = [item.id for item in result]
                    log_string += "".join([str(id) + ", " for id in sorted(id_list)])[:-2] + "}"
            hal_py.log_info(log_string)
            return result
        return decorated

    # decorator for the 2 functions get_selected_items and get_selected_item_ids
    def get_selected_items_decorator(message, f):
        @wraps(f)
        def decorated(*args, **kwargs):
            result = f(*args, **kwargs)
            log_string = "Function: " + message
            end_result_list = []
            for list in result:
                if len(list) == 0:
                    end_result_list.append([])
                else:
                    if isinstance(list[0], int):
                        end_result_list.append(sorted(list))
                    else:
                        end_result_list.append(sorted([item.id for item in list]))
            log_string += ", Gate-ID(s): " + str(end_result_list[0]) + ", Net-ID(s): " + str(end_result_list[1]) + ", Module-ID(s): " + str(end_result_list[2])
            log_string = log_string.replace("[", "{").replace("]", "}")
            hal_py.log_info(log_string)
            return result
        return decorated

    # select / deselect decorator
    def select_deselect_decorator(message, f):
        @wraps(f)
        def decorated(*args, **kwargs):
            result = f(*args, **kwargs)

            log_string = "Function: " + message

            # first 3 different structures where only a gate, net or module is passed to the function
            # these are handled in the same manner
            possible_args_type1 = [("gate", hal_py.Gate), ("net", hal_py.Net), ("module", hal_py.Module)]
            tmp_param_value = None
            for arg_tup in possible_args_type1:
                tmp_param_value = kwargs.get(arg_tup[0])
                if tmp_param_value is None and len(args) >= 2:
                    if isinstance(args[1], arg_tup[1]):
                        log_string += ", {}-Id: {{".format(str(arg_tup[0]).capitalize()) + str(args[1].id) + "}"
                elif tmp_param_value is not None:
                    log_string += ", {}-Id: {{".format(str(arg_tup[0]).capitalize()) + str(tmp_param_value.id) + "}"

            # next evaluate the lists of gates, nets and modules and the different position they can occur, depending
            # on which parameter-signatur is passed to the function, e.g. the param-name "gates" can only occur on pos 1,
            # "nets" on pos 1 and 2 and "modules" on pos 1 and 3

            # the name, the first and the second possible position and the desired object type within the list
            # (the "gates" parameter is a bit hacky because it has only 1 possible position (so just use a very
            # big number so the second check for arg-length always returns false (could also check if the
            # first condition was already visited (with a boolean)
            possible_args_type2 = [("gates", 1, 10000, hal_py.Gate), ("nets", 1, 2, hal_py.Net), ("modules", 1, 3, hal_py.Module)]
            tmp_param_value = None
            # a bit hacky but i need to check if an empty net or module list has already
            # been logged so that the next for-loop does not log an empty list again
            logged_empty_gate_list = False
            logged_empty_net_list = False
            logged_empty_module_list = False

            for arg_tup in possible_args_type2:
                tmp_param_value = kwargs.get(arg_tup[0])
                if tmp_param_value is None:
                    if len(args) >= arg_tup[1]+1 and isinstance(args[arg_tup[1]], list):
                        if len(args[arg_tup[1]]) > 0 and isinstance(args[arg_tup[1]][0], arg_tup[3]):
                            sorted_item_ids = sorted(args[arg_tup[1]], key = lambda  item: item.id)
                            log_string += ", " + arg_tup[0].capitalize()[:-1] + "-Id(s): {" + "".join([str(i.id) + ", " for i in sorted_item_ids])[:-2] + "}"
                        elif len(args[arg_tup[1]]) == 0:
                            if ("net" in arg_tup[0] and not logged_empty_net_list) or ("module" in arg_tup[0] and not logged_empty_module_list) or ("gate" in arg_tup[0] and not logged_empty_gate_list):
                                log_string += ", " + arg_tup[0].capitalize()[:-1] + "-Id(s): {}"
                                logged_empty_gate_list = True if "gate" in arg_tup[0] else logged_empty_gate_list
                                logged_empty_net_list = True if "net" in arg_tup[0] else logged_empty_net_list
                                logged_empty_module_list = True if "module" in arg_tup[0] else logged_empty_module_list
                    if len(args) >= arg_tup[2]+1 and isinstance(args[arg_tup[2]], list):
                        if len(args[arg_tup[2]]) > 0 and isinstance(args[arg_tup[2]][0], arg_tup[3]):
                            sorted_item_ids = sorted(args[arg_tup[2]], key = lambda  item: item.id)
                            log_string += ", " + arg_tup[0].capitalize()[:-1] + "-Id(s): {" + "".join([str(i.id) + ", " for i in sorted_item_ids])[:-2] + "}"
                        elif len(args[arg_tup[2]]) == 0:
                            if ("net" in arg_tup[0] and not logged_empty_net_list) or ("module" in arg_tup[0] and not logged_empty_module_list) or ("gate" in arg_tup[0] and not logged_empty_gate_list):
                                log_string += ", " + arg_tup[0].capitalize()[:-1] + "-Id(s): {}"
                                logged_empty_gate_list = True if "gate" in arg_tup[0] else logged_empty_gate_list
                                logged_empty_net_list = True if "net" in arg_tup[0] else logged_empty_net_list
                                logged_empty_module_list = True if "module" in arg_tup[0] else logged_empty_module_list
                elif tmp_param_value is not None:
                    if len(tmp_param_value) == 0:
                        if ("net" in arg_tup[0] and not logged_empty_net_list) or ("module" in arg_tup[0] and not logged_empty_module_list) or ("gate" in arg_tup[0] and not logged_empty_gate_list):
                            logged_empty_gate_list = True if "gate" in arg_tup[0] else logged_empty_gate_list
                            logged_empty_net_list = True if "net" in arg_tup[0] else logged_empty_net_list
                            logged_empty_module_list = True if "module" in arg_tup[0] else logged_empty_module_list
                            sorted_item_ids = sorted(tmp_param_value, key=lambda item: item.id)
                            log_string += ", " + arg_tup[0].capitalize()[:-1] + "-Id(s): {" + "".join([str(i.id) + ", " for i in sorted_item_ids])[:-2] + "}"
                    else:
                        sorted_item_ids = sorted(tmp_param_value, key=lambda item: item.id)
                        log_string += ", " + arg_tup[0].capitalize()[:-1] + "-Id(s): {" + "".join([str(i.id) + ", " for i in sorted_item_ids])[:-2] + "}"

            possible_args_type3 = [("gate_ids", 1), ("net_ids", 2), ("module_ids", 3)]
            tmp_param_value = None
            for arg_tup in possible_args_type3:
                tmp_param_value = kwargs.get(arg_tup[0])
                if tmp_param_value is None and len(args) >= arg_tup[1]+1 and isinstance(args[arg_tup[1]], list) and len(args[arg_tup[1]]) > 0 and isinstance(args[arg_tup[1]][0], int):
                    sorted_item_ids = sorted(args[arg_tup[1]])
                    log_string += ", {}-Id(s): {{".format(arg_tup[0].capitalize()[:-4]) + "".join([str(i) + ", " for i in sorted_item_ids])[:-2] + "}"
                elif tmp_param_value is not None:
                    if ("net" in arg_tup[0] and not logged_empty_net_list) or ("module" in arg_tup[0] and not logged_empty_module_list) or ("gate" in arg_tup[0] and not logged_empty_gate_list):
                        sorted_item_ids = sorted(tmp_param_value)
                        log_string += ", {}-Id(s): {{".format(arg_tup[0].capitalize()[:-4]) + "".join([str(i) + ", " for i in sorted_item_ids])[:-2] + "}"
            hal_py.log_info(log_string)
            return result
        return decorated

    # decorate the actual functions
    hal_gui.GuiApi.select_gate = generic_select_gate_net_module("GuiApi.select_gate", "Gate", hal_gui.GuiApi.select_gate)
    hal_gui.GuiApi.select_net = generic_select_gate_net_module("GuiApi.select_net", "Net", hal_gui.GuiApi.select_net)
    hal_gui.GuiApi.select_module = generic_select_gate_net_module("GuiApi.select_module", "Module", hal_gui.GuiApi.select_module)

    hal_gui.GuiApi.deselect_gate = generic_select_gate_net_module("GuiApi.deselect_gate", "Gate", hal_gui.GuiApi.deselect_gate)
    hal_gui.GuiApi.deselect_net = generic_select_gate_net_module("GuiApi.deselect_net", "Net", hal_gui.GuiApi.deselect_net)
    hal_gui.GuiApi.deselect_module = generic_select_gate_net_module("GuiApi.deselect_module", "Module", hal_gui.GuiApi.deselect_module)

    hal_gui.GuiApi.get_selected_gate_ids = generic_get_selected_ids_decorator("GuiApi.get_selected_gate_ids", "Gate", hal_gui.GuiApi.get_selected_gate_ids)
    hal_gui.GuiApi.get_selected_net_ids = generic_get_selected_ids_decorator("GuiApi.get_selected_net_ids", "Net", hal_gui.GuiApi.get_selected_net_ids)
    hal_gui.GuiApi.get_selected_module_ids = generic_get_selected_ids_decorator("GuiApi.get_selected_module_ids", "Module", hal_gui.GuiApi.get_selected_module_ids)

    hal_gui.GuiApi.get_selected_gates = generic_get_selected_ids_decorator("GuiApi.get_selected_gates", "Gate", hal_gui.GuiApi.get_selected_gates)
    hal_gui.GuiApi.get_selected_nets = generic_get_selected_ids_decorator("GuiApi.get_selected_nets", "Net", hal_gui.GuiApi.get_selected_nets)
    hal_gui.GuiApi.get_selected_modules = generic_get_selected_ids_decorator("GuiApi.get_selected_modules", "Module", hal_gui.GuiApi.get_selected_modules)

    hal_gui.GuiApi.get_selected_items = get_selected_items_decorator("GuiApi.get_selected_items", hal_gui.GuiApi.get_selected_items)
    hal_gui.GuiApi.get_selected_item_ids = get_selected_items_decorator("GuiApi.get_selected_item_ids", hal_gui.GuiApi.get_selected_item_ids)

    hal_gui.GuiApi.select = select_deselect_decorator("GuiApi.select", hal_gui.GuiApi.select)
    hal_gui.GuiApi.deselect = select_deselect_decorator("GuiApi.deselect", hal_gui.GuiApi.deselect)

else:
    hal_py.log_info("Gui slready decorated. Not applying again.")
