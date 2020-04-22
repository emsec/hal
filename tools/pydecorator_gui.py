from functools import wraps
import hal_py
from hal_gui import gui

# decorates select_gate, overloads: gate-id, gate, list-gate-ids, list-gates
def select_gate_decorator(message, f):
    @wraps(f)
    def decorated(*args, **kwargs):
        result = f(*args, **kwargs)
        if isinstance(args[0], list):
            print("we have a list!")
        else:
            print("it is not a list!")
        #print("GUI-Function: {}, {}-ID: {}".format(message, type, str(args[0])))
        return result
    return decorated

# different types: gate, net, module, Gate-List, Net-List, Module-List, (gate-ids,net-ids,module-ids), (gate-list,net-list,module-list)
# "second param always clear_current_selection
def select_overload_decorator(message, f):
    @wraps(f)
    def decorated(*args, **kwargs):
        result = f(*args, **kwargs)
        print("GUI-Function: {}, {}-ID: {}".format(message, type, str(args[0])))
        return result
    return decorated

gui.select_gate = select_gate_decorator("select_gate", gui.select_gate)
#gui.select_net = select_decorator("select_net", "Net", gui.select_net)
#gui.select_module = select_decorator("select_module", "Module", gui.select_module)
gui.select = select_overload_decorator("test", gui.select)
