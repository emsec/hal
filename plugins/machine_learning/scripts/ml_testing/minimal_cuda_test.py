import torch

print(torch.__version__)
print(torch.version.cuda)
print(torch.cuda.is_available())


x = torch.rand(5, 3).cuda()
print(x)
