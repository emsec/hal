import guiext
from hal_plugins import gui_extension_demo

class DumpItemsToFile(guiext.MainMenuExtension):
    def __init__(self):
        super().__init__("dump_items", "Dump netlist items to file")
        self.params.append(gui_extension_demo.PluginParameter(gui_extension_demo.NewFile,"filename","File name", "/tmp/dump_netlist.txt"))
        self.params.append(gui_extension_demo.PluginParameter(gui_extension_demo.PushButton,"exec","Dump items", ""))
    def execute(self, netlist, params):
        filename = ""
        for par in params:
            if par.get_tagname() == "filename":
                filename = par.get_value()
        dump_items_to_file(netlist,filename)

instance_DumpItemsToFile = DumpItemsToFile()

def dump_items_to_file(netlist,filename):
    count = [0,0,0]
    of = open(filename, "w")
    of.write ("%3s %-40s %s\n" % ("ID", "Module name", "Module type"))
    for m in netlist.get_modules():
        count[0] += 1
        of.write ("%3d %-40s %s\n" % (m.get_id(),m.get_name(),m.get_type()))
    of.write("------------------------------\n")
    of.write ("%3s %-40s %s\n" % ("ID", "Gate name", "Gate type"))
    for g in netlist.get_gates():
        count[1] += 1
        of.write ("%3d %-40s %s\n" % (g.get_id(),g.get_name(),g.get_type().get_name()))
    of.write("------------------------------\n")
    of.write ("%3s %s\n" % ("ID", "Net name"))
    for n in netlist.get_nets():
        count[2] += 1
        of.write ("%3d %s\n" % (n.get_id(),n.get_name()))
    of.write("------------------------------\n")
    of.close()
    print("Dump to file <" + filename + "> =>  Modules:%d  Gates:%d  Nets:%d" % tuple(count))
