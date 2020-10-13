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

            print(log_string)
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
            print(log_string)
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
            print(log_string)
            return result
        return decorated

    # select / deselect decorator
    def select_deselect_decorator(message, f):
        @wraps(f)
        def decorated(*args, **kwargs):
            result = f(*args, **kwargs)

            log_string = "Function: " + message

            # first 3 different structures where only a gate, net or module is passed to the function
            tmp_param_value = kwargs.get("gate")
            if kwargs.get("gate") is None and len(args) >= 2: # the length check (for the position) is neccessary
                if isinstance(args[1], hal_py.Gate):
                    log_string += ", Gate-Id: {" + str(args[1].id) + "}"
            elif tmp_param_value is not None:
                log_string += ", Gate-Id: {" + str(kwargs.get("gate").id) + "}"

            tmp_param_value = kwargs.get("net")
            if tmp_param_value is None and len(args) >= 2:
                if isinstance(args[1], hal_py.Net):
                    log_string += ", Net-Id: {" + str(args[1].id) + "}"
            elif tmp_param_value is not None:
                log_string += ", Net-Id: {" + str(tmp_param_value.id) + "}"

            tmp_param_value = kwargs.get("module")
            if tmp_param_value is None and len(args) >= 2:
                if isinstance(args[1], hal_py.Module):
                    log_string += ", Module-Id: {" + str(args[1].id) + "}"
            elif tmp_param_value is not None:
                log_string += ", Module-Id: {" + str(tmp_param_value.id) + "}"

            # next evaluate the lists of gates, nets and modules and the different position they can occur, depending
            # on which parameter-signatur is passed to the function, e.g. the param-name "gates" can only occur on pos 1,
            # "nets" on pos 1 and 2 and "modules" on pos 1 and 3
            tmp_param_value = kwargs.get("gates")
            if tmp_param_value is None and len(args) >= 2:
                if isinstance(args[1], list):
                    if len(args[1]) > 0 and isinstance(args[1][0], hal_py.Gate): #python does not check second statement if first is false
                        sorted_gate_ids = sorted(args[1], key=lambda gate: gate.id)
                        log_string += ", Gate-Id(s): {" + "".join([str(g.id) + ", " for g in sorted_gate_ids])[:-2] + "}"
            elif tmp_param_value is not None:
                sorted_gate_ids = sorted(tmp_param_value, key=lambda gate: gate.id)
                log_string += ", Gate-Id(s): {" + "".join([str(g.id) + ", " for g in sorted_gate_ids])[:-2] + "}"

            tmp_param_value = kwargs.get("nets")
            if tmp_param_value is None:
                if len(args) >= 2 and isinstance(args[1], list):
                    if len(args[1]) > 0 and isinstance(args[1][0], hal_py.Net): #python does not check second statement if first is false
                        sorted_net_ids = sorted(args[1], key=lambda net: net.id)
                        log_string += ", Net-Id(s): {" + "".join([str(n.id) + ", " for n in sorted_net_ids])[:-2] + "}"
                if len(args) >= 3 and isinstance(args[2], list):
                    if len(args[2]) > 0 and isinstance(args[2][0], hal_py.Net):
                        sorted_net_ids = sorted(args[2], key=lambda net: net.id)
                        log_string += ", Net-Id(s): {" + "".join([str(n.id) + ", " for n in sorted_net_ids])[:-2] + "}"
            elif tmp_param_value is not None:
                sorted_net_ids = sorted(tmp_param_value, key=lambda net: net.id)
                log_string += ", Net-Id(s): {" + "".join([str(n.id) + ", " for n in sorted_net_ids])[:-2] + "}"

            tmp_param_value = kwargs.get("modules")
            if tmp_param_value is None:
                if len(args) >=2 and isinstance(args[1], list):
                    if len(args[1]) > 0 and isinstance(args[1][0], hal_py.Module): #python does not check second statement if first is false
                        sorted_module_ids = sorted(args[1], key=lambda module: module.id)
                        log_string += ", Module-Id(s): {" + "".join([str(m.id) + ", " for m in sorted_module_ids])[:-2] + "}"
                if len(args) >= 4 and isinstance(args[3], list):
                    if len(args[3]) > 0 and isinstance(args[3][0], hal_py.Module):
                        sorted_module_ids = sorted(args[3], key=lambda module: module.id)
                        log_string += ", Module-Id(s): {" + "".join([str(m.id) + ", " for m in sorted_module_ids])[:-2] + "}"
            elif tmp_param_value is not None:
                sorted_module_ids = sorted(tmp_param_value, key=lambda module: module.id)
                log_string += ", Module-Id(s): {" + "".join([str(m.id) + ", " for m in sorted_module_ids])[:-2] + "}"

            # check for the case when 3 integer lists are passed, on pos 1 are the gate ids, on pos 2 the net ids and
            # on pos 3 the module ids
            tmp_param_value = kwargs.get("gate_ids")
            if tmp_param_value is None and len(args) >= 2:
                if isinstance(args[1], list):
                    if len(args[1]) > 0 and isinstance(args[1][0], int): #python does not check second statement if first is false
                        sorted_gate_ids = sorted(args[1])
                        log_string += ", Gate-Id(s): {" + "".join([str(g) + ", " for g in sorted_gate_ids])[:-2] + "}"
            elif tmp_param_value is not None:
                sorted_gate_ids = sorted(tmp_param_value)
                log_string += ", Gate-Id(s): {" + "".join([str(g) + ", " for g in sorted_gate_ids])[:-2] + "}"

            tmp_param_value = kwargs.get("net_ids")
            if tmp_param_value is None and len(args) >= 3:
                if isinstance(args[2], list):
                    if len(args[2]) > 0 and isinstance(args[2][0], int): #python does not check second statement if first is false
                        sorted_net_ids = sorted(args[2])
                        log_string += ", Net-Id(s): {" + "".join([str(g) + ", " for g in sorted_net_ids])[:-2] + "}"
            elif tmp_param_value is not None:
                sorted_net_ids = sorted(tmp_param_value)
                log_string += ", Net-Id(s): {" + "".join([str(n) + ", " for n in sorted_net_ids])[:-2] + "}"

            tmp_param_value = kwargs.get("module_ids")
            if tmp_param_value is None and len(args) >= 4:
                if isinstance(args[3], list):
                    if len(args[3]) > 0 and isinstance(args[3][0], int): #python does not check second statement if first is false
                        sorted_module_ids = sorted(args[3])
                        log_string += ", Module-Id(s): {" + "".join([str(g) + ", " for g in sorted_module_ids])[:-2] + "}"
            elif tmp_param_value is not None:
                sorted_module_ids = sorted(tmp_param_value)
                log_string += ", Module-Id(s): {" + "".join([str(n) + ", " for n in sorted_module_ids])[:-2] + "}"

            print(log_string)
            return result
        return decorated

    # decorate the actual functions
    hal_gui.GuiApi.select_gate = generic_select_gate_net_module("GuiApi.select_gate", "Gate", hal_gui.GuiApi.select_gate)
    hal_gui.GuiApi.select_net = generic_select_gate_net_module("GuiApi.select_net", "Net", hal_gui.GuiApi.select_net)
    hal_gui.GuiApi.select_module = generic_select_gate_net_module("GuiApi.select_module", "Module", hal_gui.GuiApi.select_module)

    hal_gui.GuiApi.deselect_gate = generic_select_gate_net_module("GuiApi.delect_gate", "Gate", hal_gui.GuiApi.deselect_gate)
    hal_gui.GuiApi.deselect_net = generic_select_gate_net_module("GuiApi.delect_net", "Net", hal_gui.GuiApi.deselect_net)
    hal_gui.GuiApi.deselect_module = generic_select_gate_net_module("GuiApi.delect_module", "Module", hal_gui.GuiApi.deselect_module)

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


