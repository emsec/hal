import torch

t = torch.tensor([1, 1, 1, 1, 1, 1, 1, 4, 4, 4, 4, 4, 4, 8], dtype=torch.float32)

t_mean = torch.mean(t)
t_var = torch.var(t)

print(t_mean)
print(t_var)

t_norm = (t - t_mean) / torch.sqrt(t_var)

print(t_norm)