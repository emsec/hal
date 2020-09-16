if "__decorated_gui__" not in dir():
    __decorated_gui__ = True

    import hal_gui
    import hal_py
    from functools import wraps

    # generic decorator for select_gate, select_net and select_module
    def generic_select_gate_net_module(message, type, f):
        @wraps(f)
        def decorated(*args, **kwargs):
            result = f(*args, **kwargs)
            log_string = "Function: {}, {}-ID(s): {{".format(message, type)

            if isinstance(args[1], list):
                if isinstance(args[1][0], int):
                    sorted_list = sorted(args[1])
                    log_string += "".join([str(id) + ", " for id in sorted_list])[:-2] + "}"
                else:
                    sorted_list = sorted(args[1], key=lambda gate: gate.id)
                    log_string += "".join([str(g.id) + ", " for g in sorted_list])[:-2] + "}"
            else:
                if isinstance(args[1], int):
                    log_string += str(args[1]) + "}"
                else:
                    log_string += str(args[1].id) + "}"

            if len(kwargs) > 0:
                log_string += "(keyword-arguments found, these are not supported yet!"
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

            #reduce code by remove redundancy as well as integrate checks into earlier checks (e.g. "nets", "modules"..)
            gates_found = False
            nets_found = False
            modules_found = False

            tmp_param_value = kwargs.get("gate")
            if kwargs.get("gate") is None and len(args) >= 2: # the length check (for the position) is neccessary
                if isinstance(args[1], hal_py.Gate):
                    log_string += ", Gate-Id: {" + str(args[1].id) + "}"
            elif kwargs.get("gate") is not None:
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

            # integrate the other the other position into this check so another check for gates, nets and modules
            # is not necessary anymore (another len(args) to isinstance(args[2], list): if len(args[2]) > 0 and isinstance(args[2], net):..
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
            if tmp_param_value is None and len(args) >= 2:
                if isinstance(args[1], list):
                    if len(args[1]) > 0 and isinstance(args[1][0], hal_py.Net): #python does not check second statement if first is false
                        sorted_net_ids = sorted(args[1], key=lambda net: net.id)
                        log_string += ", Net-Id(s): {" + "".join([str(n.id) + ", " for n in sorted_net_ids])[:-2] + "}"
            elif tmp_param_value is not None:
                sorted_net_ids = sorted(tmp_param_value, key=lambda net: net.id)
                log_string += ", Net-Id(s): {" + "".join([str(n.id) + ", " for n in sorted_net_ids])[:-2] + "}"

            tmp_param_value = kwargs.get("modules")
            if tmp_param_value is None and len(args) >= 2:
                if isinstance(args[1], list):
                    if len(args[1]) > 0 and isinstance(args[1][0], hal_py.Module): #python does not check second statement if first is false
                        sorted_module_ids = sorted(args[1], key=lambda module: module.id)
                        log_string += ", Module-Id(s): {" + "".join([str(m.id) + ", " for m in sorted_module_ids])[:-2] + "}"
            elif tmp_param_value is not None:
                sorted_module_ids = sorted(tmp_param_value, key=lambda module: module.id)
                log_string += ", Module-Id(s): {" + "".join([str(m.id) + ", " for m in sorted_module_ids])[:-2] + "}"

            ####################### Check for the 3 list with ints
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

            #already found variable is neccessary to not print double (and wrong)
            #this parameter is already be handled by the first check since 2 "options" overlap in regards to position:
            # (list<gate>, bool, bool) and (list<gate>, list<net>, list<module>, bool, bool)
            #tmp_param_value = kwargs.get("gates")
            #if tmp_param_value is None and len(args) >= 2:
            #    if isinstance(args[1], list):
            #        if len(args[1]) > 0 and isinstance(args[1][0], hal_py.Gate): #python does not check second statement if first is false
            #            sorted_gate_ids = sorted(args[1], key=lambda gate: gate.id)
            #            log_string += ", Gate-Id(s): {" + "".join([str(g.id) + ", " for g in sorted_gate_ids])[:-2] + "}"
            #elif tmp_param_value is not None and not gates_found:
            #    sorted_gate_ids = sorted(tmp_param_value, key=lambda gate: gate.id)
            #    log_string += ", Gate-Id(s): {" + "".join([str(n) + ", " for n in sorted_gate_ids])[:-2] + "}"

            tmp_param_value = kwargs.get("nets")
            if tmp_param_value is None and len(args) >= 3:
                if isinstance(args[2], list):
                    if len(args[2]) > 0 and isinstance(args[2][0], hal_py.Net): #python does not check second statement if first is false
                        sorted_net_ids = sorted(args[2], key=lambda net: net.id)
                        log_string += ", Net-Id(s): {" + "".join([str(g.id) + ", " for g in sorted_net_ids])[:-2] + "}"

            tmp_param_value = kwargs.get("modules")
            if tmp_param_value is None and len(args) >= 4:
                if isinstance(args[3], list):
                    if len(args[3]) > 0 and isinstance(args[3][0], hal_py.Module): #python does not check second statement if first is false
                        sorted_module_ids = sorted(args[3], key=lambda module: module.id)
                        log_string += ", Module-Id(s): {" + "".join([str(g.id) + ", " for g in sorted_module_ids])[:-2] + "}"


            print(log_string)
            return result
        return decorated

    # decorate the actual functions
    hal_gui.GuiApi.select_gate = generic_select_gate_net_module("GuiApi.select_gate", "Gate", hal_gui.GuiApi.select_gate)
    hal_gui.GuiApi.select_net = generic_select_gate_net_module("GuiApi.select_net", "Net", hal_gui.GuiApi.select_net)
    hal_gui.GuiApi.select_module = generic_select_gate_net_module("GuiApi.select_module", "Module", hal_gui.GuiApi.select_module)

    hal_gui.GuiApi.deselect_gate = generic_select_gate_net_module("GuiApi.delect_gate", "Gate", hal_gui.GuiApi.deselect_gate)
    hal_gui.GuiApi.deselect_net = generic_select_gate_net_module("GuiApi.delect_net", "Net", hal_gui.GuiApi.deselect_net)
    hal_gui.GuiApi.deselect_Module = generic_select_gate_net_module("GuiApi.delect_module", "Module", hal_gui.GuiApi.deselect_module)

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

