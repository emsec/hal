#Base class (don't use directly)
class GuiExtension:
    instances = []
    fcount = 0
    def __init__(self,tag,labl):
        self.tagname = tag
        self.label   = labl
        GuiExtension.instances.append(self)
    def execute(self, netlist):
        pass

# Derive from MainMenuExtension to place items in main window menu Plugins->gui_extension_demo->label
# examples : plugin_dump_items_to_file, plugin_list_gates_by_type
class MainMenuExtension(GuiExtension):
    def __init__(self,tag,labl):
        super().__init__(tag,labl)
        self.params = []
    def execute(self, netlist, params):
        pass

# Derive from ModuleContextExtension to add to context menu if single module is selected
class ModuleContextExtension(GuiExtension):
    def __init__(self, labl):
        GuiExtension.fcount += 1
        super().__init__("mod%d" % (GuiExtension.fcount),labl)
    def execute(self, netlist, module_id):
        pass

# Derive from GateContextExtension to add to context menu if single gate is selected
# example : plugin_nets_connected_to_gate
class GateContextExtension(GuiExtension):
    count = 0
    def __init__(self, labl):
        GuiExtension.fcount += 1
        super().__init__("gat%d" % (GuiExtension.fcount),labl)
    def execute(self, netlist, gate_id):
        pass

# Derive from NetContextExtension to add to context menu if single net is selected
# example : plugin_gates_connected_by_net
class NetContextExtension(GuiExtension):
    count = 0
    def __init__(self, labl):
        GuiExtension.fcount += 1
        super().__init__("net%d" % (GuiExtension.fcount),labl)
    def execute(self, netlist, net_id):
        pass
