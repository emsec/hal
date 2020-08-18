if "__decorated_gui__" not in dir():
    __decorated_gui__ = True

    import hal_gui
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


    # decorate the actual functions
    hal_gui.GuiApi.select_gate = generic_select_gate_net_module("GuiApi.select_gate", "Gate", hal_gui.GuiApi.select_gate)
    hal_gui.GuiApi.select_net = generic_select_gate_net_module("GuiApi.select_net", "Net", hal_gui.GuiApi.select_net)
    hal_gui.GuiApi.select_module = generic_select_gate_net_module("GuiApi.select_module", "Module", hal_gui.GuiApi.select_module)

else:
    hal_py.log_info("Gui slready decorated. Not applying again.")

