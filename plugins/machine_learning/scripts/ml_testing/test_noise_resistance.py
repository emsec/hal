import torch

samples = 128
noise_dimension = 1024

device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')

a = torch.randint(2, (samples,), device=device)
b = torch.randint(2, (samples,), device=device)

c = a & b

#n = torch.randint(2, (samples, noise_dimension))
n = torch.randint(2, (samples, noise_dimension), device=device)

x = torch.stack([a, b], dim=1, out=None)
x = torch.cat((n, x), 1)
y = c.unsqueeze(1)

# print(a)
# print(b)
# print(c)
# print(n)
# print(x)
# print(y)

num_classes = 1
num_features = 2 + noise_dimension

h = 4

import torch.nn.functional as F


class NN(torch.nn.Module):
    def __init__(self):
        super().__init__()
        self.l1 = torch.nn.Linear(num_features, h)
        self.l2 = torch.nn.Linear(h, num_classes)
        #self.l3 = torch.nn.Linear(h, num_classes)

    def forward(self, x):
        x = self.l1(x)
        x = F.relu6(x)
        #x = F.dropout(x, p=0.1, training=self.training)
        x = self.l2(x)
        x = F.relu6(x)
        #x = F.dropout(x, p=0.1, training=self.training)
        # x = self.l3(x)
        # x = F.relu(x)

        return x

model = NN().to(device)
optimizer = torch.optim.Adam(model.parameters(), lr=0.001)
    
for epoch in range(10000):
    optimizer.zero_grad()

    out = model(x.float())

    # print(out)
    # print(y)

    loss = F.mse_loss(out, y.float())
    loss.backward()
    optimizer.step()

    if (epoch % 50) == 0:
        print("Epoch {} - Loss: {}".format(epoch, loss))