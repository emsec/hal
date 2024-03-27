#!/usr/bin/env python3
import sys, os
import pathlib

#some necessary configuration:
base_path = "/home/simon/projects/hal/"

sys.path.append(base_path + "build/lib/") #this is where your hal python lib is located
os.environ["HAL_BASE_PATH"] = base_path + "build" # hal base path
import hal_py

lm = hal_py.LogManager()
lm.remove_sink_from_default("stdout")

#initialize HAL
hal_py.plugin_manager.load_all_plugins()


netlist_paths = [
    (base_path + "examples/uart/uart.v", base_path + "examples/uart/example_library.hgl"),
    (base_path + "examples/fsm/fsm.v", base_path + "examples/fsm/example_library.hgl"),
    #(base_path + "examples/simple_alu/simple_alu.vhdl", base_path + "examples/simple_alu/XILINX_UNISIM.hgl"),
    #(base_path + "examples/toy_cipher/toy_cipher.vhd", base_path + "examples/toy_cipher/XILINX_UNISIM.hgl"),
]

from hal_plugins import machine_learning
    
import torch
import torch.nn.functional as F
from torch_geometric.nn import GCNConv, SAGEConv

device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')

print(device)

def generate_netlist_graph(nl_path, gl_path):
    netlist = hal_py.NetlistFactory.load_netlist(nl_path, gl_path)

    g = machine_learning.MachineLearning.Graph.test_construct_netlist_graph(netlist)
    l = machine_learning.MachineLearning.Graph.test_construct_node_labels(netlist)

    # print(g)
    # print(g.node_features)
    # print(g.edge_list)

    # print(l)

    edge_index = torch.Tensor(g.edge_list).long().to(device)
    x = torch.Tensor(g.node_features).to(device)
    y = torch.Tensor(l).long().to(device)

    #torch.nn.functional.normalize(x, dim=0, out=x)

    return (edge_index, x, y)

data_set = list()

for nl_path, gl_path in netlist_paths:
    print(nl_path)
    edge_index, x, y = generate_netlist_graph(nl_path, gl_path)

    data_set.append((edge_index, x, y))

split = int(len(data_set) / 2)
training_set   = data_set[:split]
evaluation_set = data_set[split:]


num_classes = max(list(max(d[2]) for d in data_set)).item() + 1
num_features = len(data_set[0][1][0])

print(num_classes, num_features)

class GNN(torch.nn.Module):
    def __init__(self):
        super().__init__()
        self.conv1 = SAGEConv(num_features, 16, aggr="mean")
        self.conv2 = SAGEConv(16, 16, aggr="mean")
        self.lin1 = torch.nn.Linear(16, 16)
        self.lin2 = torch.nn.Linear(16, num_classes)

    def forward(self, x, edge_list):
        x = self.conv1(x, edge_list)
        x = F.sigmoid(x)
        x = F.dropout(x, p=0.5, training=self.training)
        x = self.conv2(x, edge_list)
        x = F.sigmoid(x)
        x = F.dropout(x, p=0.5, training=self.training)

        x = self.lin1(x)
        x = F.sigmoid(x)
        x = F.dropout(x, p=0.5, training=self.training)
        x= self.lin2(x)

        return x
        #return F.softmax(x)

class NN(torch.nn.Module):
    def __init__(self):
        super().__init__()
        self.l1 = torch.nn.Linear(num_features, 16)
        self.l2 = torch.nn.Linear(16, num_classes)

    def forward(self, x):
        x = self.l1(x)
        x = F.leaky_relu(x)
        x = F.dropout(x, p=0.1, training=self.training)
        x = self.l2(x)
        x = F.leaky_relu(x)
        x = F.dropout(x, p=0.1, training=self.training)

        return x
        #return F.softmax(x)

model = GNN().to(device)
#model = NN().to(device)
optimizer = torch.optim.Adam(model.parameters(), lr=0.001)
    
for epoch in range(2500):
    for edge_index, x, _y in training_set:
        optimizer.zero_grad()

        out = model(x, edge_index)
        #out = model(x)

        y = torch.eye(num_classes).to(device)[_y].squeeze()

        # print(out)
        # print(y)

        loss = F.cross_entropy(out, y)
        loss.backward()
        optimizer.step()
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
        print(out)
        print(y)

        loss = F.cross_entropy(out, y)
        print("Eval Loss: {}".format(loss))


#unload everything hal related
hal_py.plugin_manager.unload_all_plugins()