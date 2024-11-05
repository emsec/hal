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

benchmarks_base_path = pathlib.Path("/home/simon/projects/benchmarks")
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

device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')

print(device)

data_set = list()

if pathlib.Path("/home/simon/projects/hal/plugins/machine_learning/data/gate_pairs.pt").exists():
    data_set = torch.load("/home/simon/projects/hal/plugins/machine_learning/data/gate_pairs.pt")
    print("Loaded dataset")
else:
    total_pairs = 0
    for nl_path, gl_path in netlist_paths[:2]:
        #print(nl_path)

        netlist = hal_py.NetlistFactory.load_netlist(nl_path, gl_path)
        pairs, labels = machine_learning.MachineLearning.GatePairLabel.test_build_labels(netlist)

        total_pairs += len(pairs)
        print("Found {} pairs!".format(len(pairs)))

        features = machine_learning.MachineLearning.GatePairFeature.test_build_feature_vec(netlist, pairs)

        #print(pairs)
        #print(features)
        #print(labels)


        # print("------------------ PAIRS -----------------------------")
        # for g_a, g_b in pairs:
        #     print("A: {} B: {}".format(g_a.id, g_b.id))
        # print("------------------------------------------------------")

        x = torch.Tensor(features).float()
        y = torch.Tensor(labels).long()
        data_set.append((x, y))

    # torch.save(data_set, "/home/simon/projects/hal/plugins/machine_learning/data/gate_pairs_benchmarks.pt")
    print("Found a total of {:.2} Billion pairs!".format(total_pairs / 1_000_000_000))
    print("Saved dataset")


split = int(len(data_set) + 1 / 2)
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
    
for epoch in range(2000):
    for x, _y in training_set:
        optimizer.zero_grad()

        out = model(x.to(device))

        y = torch.eye(num_classes).to(device)[_y.to(device)].squeeze()

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