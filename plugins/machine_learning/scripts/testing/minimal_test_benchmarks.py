#!/usr/bin/env python3
import sys, os
import pathlib

user_name = os.getlogin()

# some necessary configuration:
if user_name == "simon":
    base_path = "/home/simon/projects/hal/"
    benchmarks_base_path = pathlib.Path("/home/simon/projects/benchmarks")
if user_name == "klix":
    base_path = "/home/klix/projects/hal"
    benchmarks_base_path = pathlib.Path("/home/klix/projects/benchmarks")
else:
    print("add base paths for user {}before executing...".format(user_name))
    exit()

sys.path.append(base_path + "build/lib/") #this is where your hal python lib is located
os.environ["HAL_BASE_PATH"] = base_path + "build" # hal base path
import hal_py

lm = hal_py.LogManager()
lm.remove_sink_from_default("stdout")

#initialize HAL
hal_py.plugin_manager.load_all_plugins()


netlist_base_paths = benchmarks_base_path / "netlists_preprocessed/yosys/NangateOpenCellLibrary_functional"

netlist_paths = list()
for netlist_path in netlist_base_paths.glob("**/*.hal"):
    # netlist_base_path = netlist_path.parent
    # netlist_information_path = netlist_base_path / "netlist_information.json"
    # netlist_information = json.load(json_file)

    netlist_paths.append((netlist_path, benchmarks_base_path / "gate_libraries" / "NangateOpenCellLibrary_functional.hgl"))

from hal_plugins import machine_learning
    
import torch
import torch.nn.functional as F
from torch_geometric.nn import GCNConv, SAGEConv

device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')

print(device)

def generate_netlist_graph(nl_path, gl_path):
    print(nl_path)
    netlist = hal_py.NetlistFactory.load_netlist(nl_path, gl_path)

    g = machine_learning.MachineLearning.Graph.test_construct_netlist_graph(netlist)
    l = machine_learning.MachineLearning.Graph.test_construct_node_labels(netlist)

    # print(g)
    # print(g.node_features)
    # print(g.edge_list)

    # print(l)

    edge_index = torch.Tensor(g.edge_list).long().to(device)
    x = torch.Tensor(g.node_features).float().to(device)
    y = torch.Tensor(l).long().to(device)

    return (edge_index, x, y)

data_set = list()

if pathlib.Path("/home/simon/projects/hal/plugins/machine_learning/data/netlist_graphs.pt").exists():
    data_set = torch.load("/home/simon/projects/hal/plugins/machine_learning/data/netlist_graphs.pt")
    print("Loaded dataset")
else:
    for nl_path, gl_path in netlist_paths[:1]:
        #print(nl_path)
        edge_index, x, y = generate_netlist_graph(nl_path, gl_path)

        data_set.append((edge_index, x, y))
    
    torch.save(data_set, "/home/simon/projects/hal/plugins/machine_learning/data/netlist_graphs_benchmarks.pt")
    print("Saved dataset")

split = int(len(data_set) / 2)
training_set   = data_set[:split]
evaluation_set = data_set[split:]


#num_classes = max(list(max(d[2]) for d in data_set)).item() + 1
num_classes = 2
num_features = len(data_set[0][1][0])

print(num_classes, num_features)

class GNN(torch.nn.Module):
    def __init__(self):
        super().__init__()
        self.conv1 = SAGEConv(num_features, 64, aggr="mean")
        self.conv2 = SAGEConv(64, 64, aggr="mean")
        self.lin1 = torch.nn.Linear(64, 64)
        self.lin2 = torch.nn.Linear(64, num_classes)

    def forward(self, x, edge_list):
        x = self.conv1(x, edge_list)
        x = F.sigmoid(x)
        #x = F.dropout(x, p=0.5, training=self.training)
        x = self.conv2(x, edge_list)
        x = F.sigmoid(x)
        x = F.dropout(x, p=0.5, training=self.training)

        x = self.lin1(x)
        x = F.sigmoid(x)
        #x = F.dropout(x, p=0.5, training=self.training)
        x= self.lin2(x)

        #return x
        return F.softmax(x, dim=1)

class NN(torch.nn.Module):
    def __init__(self):
        super().__init__()
        self.l1 = torch.nn.Linear(num_features, 64)
        self.l2 = torch.nn.Linear(64, num_classes)

    def forward(self, x):
        x = self.l1(x)
        x = F.relu6(x)
        #x = F.dropout(x, p=0.1, training=self.training)
        x = self.l2(x)
        x = F.relu6(x)
        #x = F.dropout(x, p=0.1, training=self.training)

        #return x
        return F.softmax(x)

model = GNN().to(device)
#model = NN().to(device)
optimizer = torch.optim.Adam(model.parameters(), lr=0.001)
    
for epoch in range(250):
    for edge_index, x, _y in training_set:
        optimizer.zero_grad()

        out = model(x, edge_index)
        #out = model(x)

        y = torch.eye(num_classes).to(device)[_y].squeeze()

        print(out)
        print(y)

        loss = F.cross_entropy(out, y)
        loss.backward()
        optimizer.step()

    if (epoch % 50) == 0 or epoch < 20:
        print("Epoch {} - Loss: {}".format(epoch, loss))

# evaluate model:
model.eval()

with torch.no_grad():
    for edge_index, x, _y in evaluation_set:
        out = model(x, edge_index)
        #out = model(x)

        y = torch.eye(num_classes).to(device)[_y].squeeze()

        # print(graph[1])
        # print(graph[2])
        # print(out)
        # print(y)

        loss = F.cross_entropy(out, y)
        print("Eval Loss: {}".format(loss))


#unload everything hal related
hal_py.plugin_manager.unload_all_plugins()