import guiext

class NetsConnectedToGate(guiext.GateContextExtension):
    def __init__(self):
        super().__init__("Highlight nets connected to gate")
    def execute (self,netlist,gate_id):
        hilite_nets(netlist,gate_id)

instance_NetsConnectedToGate = NetsConnectedToGate()

def hilite_nets(netlist,gate_id):
    g = netlist.get_gate_by_id(gate_id)
    grp = netlist.create_grouping("connected to gate '" + g.get_name() + "'")
    for n in g.get_fan_in_nets():
        grp.assign_net(n)
    for n in g.get_fan_out_nets():
        grp.assign_net(n)

