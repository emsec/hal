sources = list()
destinations = list()

for n in netlist.get_nets():
    if len(n.get_sources()) != 1 and not n.is_global_input_net():
        sources.append([n.name, n.get_sources()])
    if len(n.get_destinations()) < 1 and not n.is_global_output_net():
        destinations.append([n.name, n.get_destinations()])
        
print("sources:")
for s in sources:
    print(s[0], len(s[1]))
    
print("destinations:")
for d in destinations:
    print(d[0], len(d[1]))
