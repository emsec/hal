import guiext
from hal_plugins import gui_extension_demo

class ListGatesByType(guiext.MainMenuExtension):
    def __init__(self):
        super().__init__("list_gtype", "List gates with given gate type")
        self.params.append(gui_extension_demo.PluginParameter(gui_extension_demo.String,"type","Gate type starts with", "LUT"))
        self.params.append(gui_extension_demo.PluginParameter(gui_extension_demo.PushButton,"exec","List gates", ""))
    def execute(self, netlist, params):
        type = ""
        for par in params:
            if par.get_tagname() == "type":
                type = par.get_value()
                break
        list_gates_by_type(netlist,type)

instance_ListGatesByType = ListGatesByType()

def list_gates_by_type(netlist,type):
    print ("%3s %-40s %s" % ("ID", "Gate name", "Gate type"))
    for g in netlist.get_gates():
       tp = g.get_type().get_name()
       if (tp.startswith(type)):
           print ("%3d %-40s %s" % (g.get_id(),g.get_name(),tp))
