import guiext

class GatesConnectedByNet(guiext.NetContextExtension):
    def __init__(self):
        super().__init__("Highlight gates connected to net")
    def execute (self,netlist,net_id):
        hilite_gates(netlist,net_id)

instance_GatesConnectedByNet = GatesConnectedByNet()

def hilite_gates(netlist,net_id):
    n = netlist.get_net_by_id(net_id)
    grp = netlist.create_grouping("connected by net '" + n.get_name() + "'")
    for ep in n.get_destinations():
        g = ep.get_gate()
        grp.assign_gate(g)
    for ep in n.get_sources():
        g = ep.get_gate()
        grp.assign_gate(g)

