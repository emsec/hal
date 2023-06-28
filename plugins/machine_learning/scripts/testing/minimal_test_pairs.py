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
    (base_path + "examples/toy_cipher/toy_cipher.vhd", base_path + "examples/toy_cipher/XILINX_UNISIM.hgl"),
    (base_path + "examples/fsm/fsm.v", base_path + "examples/fsm/example_library.hgl"),
    (base_path + "examples/simple_alu/simple_alu.vhdl", base_path + "examples/simple_alu/XILINX_UNISIM.hgl"),
]

from hal_plugins import machine_learning
    
import torch
import torch.nn.functional as F
from torch_geometric.nn import GCNConv, SAGEConv

device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')

print(device)

data_set = list()

if pathlib.Path("/home/simon/projects/hal/plugins/machine_learning/data/gate_pairs.pt").exists():
    data_set = torch.load("/home/simon/projects/hal/plugins/machine_learning/data/gate_pairs.pt")
    print("Loaded dataset")
else:
    for nl_path, gl_path in netlist_paths:
        netlist = hal_py.NetlistFactory.load_netlist(nl_path, gl_path)
        pairs, labels = machine_learning.MachineLearning.GatePairLabel.test_build_labels(netlist)
        features = machine_learning.MachineLearning.GatePairFeature.test_build_feature_vec(netlist, pairs)

        #print(pairs)
        #print(features)
        #print(labels)


        # print("------------------ PAIRS -----------------------------")
        # for g_a, g_b in pairs:
        #     print("A: {} B: {}".format(g_a.id, g_b.id))
        # print("------------------------------------------------------")

        x = torch.Tensor(features).float().to(device)
        y = torch.Tensor(labels).long().to(device)

        data_set.append((x, y))

    
    #torch.save(data_set, "/home/simon/projects/hal/plugins/machine_learning/data/gate_pairs.pt")
    print("Saved dataset")


split = int(len(data_set) / 2)
training_set   = data_set[:split]
evaluation_set = data_set[split:]

#num_classes = max(list(max(d[2]) for d in data_set)).item() + 1
num_classes = 2
num_features = len(data_set[0][0][0])

print("Num classes: {}".format(num_classes))
print("Num features: {}".format(num_features))

class NN(torch.nn.Module):
    def __init__(self):
        super().__init__()
        self.l1 = torch.nn.Linear(num_features, 128)
        self.l2 = torch.nn.Linear(128, 128)
        self.l3 = torch.nn.Linear(128, 128)
        self.l4 = torch.nn.Linear(128, 128)
        self.l5 = torch.nn.Linear(128, num_classes)

    def forward(self, x):
        x = self.l1(x)
        x = F.leaky_relu(x)
        x = F.dropout(x, p=0.5, training=self.training)
        x = self.l2(x)
        x = F.leaky_relu(x)
        x = F.dropout(x, p=0.5, training=self.training)
        x = self.l3(x)
        x = F.leaky_relu(x)
        x = F.dropout(x, p=0.5, training=self.training)
        x = self.l4(x)
        x = F.leaky_relu(x)
        x = F.dropout(x, p=0.5, training=self.training)
        x = self.l5(x)
        # x = F.leaky_relu(x)
        # x = F.dropout(x, p=0.5, training=self.training)

        return x
        #return F.log_softmax(x)

model = NN().to(device)
optimizer = torch.optim.Adam(model.parameters(), lr=0.0001)
    
for epoch in range(200):
    for x, _y in training_set:
        optimizer.zero_grad()

        out = model(x)

        y = torch.eye(num_classes).to(device)[_y].squeeze()

        # print(x)
        # print(out)
        # print(y)

        loss = F.mse_loss(out, y)
        loss.backward()
        optimizer.step()

    print("Epoch {} - Loss: {}".format(epoch, loss))

# evaluate model:
model.eval()

with torch.no_grad():
    for x, _y in evaluation_set:
        out = model(x)

        y = torch.eye(num_classes).to(device)[_y].squeeze()

        torch.set_printoptions(profile="full")
        print(x)
        print(y)
        torch.set_printoptions(profile="default") # reset
        # print(out)

        loss = F.mse_loss(out, y)
        print("Eval Loss: {}".format(loss))


#unload everything hal related
hal_py.plugin_manager.unload_all_plugins()