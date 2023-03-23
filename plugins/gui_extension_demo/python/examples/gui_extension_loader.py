import glob
import re
import importlib
import types
import os
import sys
import hal_gui

# path to python demo pugins. Please adjust manually if HAL_BASE_PATH not present
pathDemo = os.environ.get('HAL_BASE_PATH') + "/../plugins/gui_extension_demo/python/examples"
sys.path.append(pathDemo)

# load and clear demo extension C++-plugin
import guiext
guiext.GuiExtension.instances = []
from hal_plugins import gui_extension_demo
ged = hal_py.plugin_manager.get_plugin_instance("gui_extension_demo")
plg = gui_extension_demo.GuiExtensionPythonBase("ext", "Base Extension")
plg.clear()

# load all python plugins
reModule = re.compile("^.*plugin_(.*)\.py$")
for fname in glob.glob(pathDemo + "/plugin_*.py"):
    m = reModule.match(fname)
    if m:
        mname = m.group(1)
        loader = importlib.machinery.SourceFileLoader(mname,fname)
        mod = loader.load_module()
        if mod:
            print ("successfully loaded module <" + mname + ">")
        else:
            print ("failed to load <" + fname + ">")

# register all gui extensions
for gi in guiext.GuiExtension.instances:
    if isinstance(gi, guiext.MainMenuExtension):
        gpars = []
        gpars.append(gui_extension_demo.PluginParameter(gui_extension_demo.TabName,gi.tagname,gi.label,""))
        for par in gi.params:
            par.set_tagname(gi.tagname + "/" + par.get_tagname())
            gpars.append(par)
        plg.add_main_menu(gpars)
    elif isinstance(gi, guiext.ModuleContextExtension):
        plg.add_module_context(gi.tagname,gi.label)
    elif isinstance(gi, guiext.GateContextExtension):
        plg.add_gate_context(gi.tagname,gi.label)
    elif isinstance(gi, guiext.NetContextExtension):
        plg.add_net_context(gi.tagname,gi.label)
    else:
        print ("plugin " + gi.tagname + " <" + gi.label + "> ignored")

# pause script and wait for any extension to be activated
hal_gui.gui_input.wait_for_menu_selection()

# evaluate function call and execute appropriate python code
fc = plg.get_function_call()
n = fc.find('/')
if n > 0:
    fc = fc[0:n]
for gi in guiext.GuiExtension.instances:
    if gi.tagname != fc:
        continue
    if isinstance(gi, guiext.MainMenuExtension):
        prefix = gi.tagname + "/"
        n = len(prefix)
        gpars = []
        for par in plg.get_parameter():
            tag = par.get_tagname()
            if not tag.startswith(prefix):
                continue
            par.set_tagname(tag[n:])
            gpars.append(par)
        gi.execute(netlist,gpars)
    elif isinstance(gi, guiext.ModuleContextExtension):
        mods = plg.get_selected_modules()
        if mods:
            gi.execut(netlist,mods[0])
    elif isinstance(gi, guiext.GateContextExtension):
        gats = plg.get_selected_gates()
        print ("gats: " + str(gats))
        if gats:
            gi.execute(netlist,gats[0])
    elif isinstance(gi, guiext.NetContextExtension):
        nets = plg.get_selected_nets()
        print ("nets: " + str(nets))
        if nets:
            gi.execute(netlist,nets[0])

    break

plg.clear()

#    plg.add_parameter(gui_extension_demo.PluginParameter(gui_extension_demo.String,"name","Enter name", "moose"))
#    plg.add_parameter(gui_extension_demo.PluginParameter(gui_extension_demo.Integer,"id","Enter id", "27"))
#    plg.register_extension()

