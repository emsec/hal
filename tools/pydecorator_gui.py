if "__decorated_gui__" not in dir():
    __decorated_gui__ = True

    import hal_gui
    from functools import wraps

    # select_gate decorator, possible arguments: [gate-id, list of gate-ids, gate, list of gates,] , bool clear_curr_selec, bool nav_to_selec
    def select_gate_decorator(message, f):
        @wraps(f)
        def decorated(*args, **kwargs):
            result = f(*args, **kwargs)
            log_string = "Function: " + message + ", Gate-ID(s): {"
            
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


    hal_gui.GuiApi.select_gate = select_gate_decorator("GuiApi.select_gate", hal_gui.GuiApi.select_gate)

else:
    hal_py.log_info("Gui slready decorated. Not applying again.")

