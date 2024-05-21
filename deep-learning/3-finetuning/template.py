# %%
## Import
import os
import numpy as np

import torch
print(torch.__version__)
import torch.nn.functional as F
from torch import nn
import torchvision.models
from torchvision import transforms,datasets
from PIL import Image


import matplotlib
import matplotlib.pyplot as plt

#!%load_ext autoreload
#!%autoreload 2
#!%matplotlib inline

## Helper Functions
def show_tensor_image(img_t):
    """
    Function to show the image after initial transform, note: need to permute dimensions from the tensor CxWxH convention
    """
    im = img_t.detach()
    im = torch.squeeze(im, 0) if im.dim()==4 else im
    im = im.permute([1,2,0]) if im.dim()==3 else im
    im = im - im.min()
    im /= im.max()
    return plt.gca().imshow(im.cpu().numpy())

def select_device():
    """ 
    Find the CUDA device with max available memory and set the global dev variable to it
    If less than 4GB memory on all devices, resort to dev='cpu'
    Repeated calls to the function select the same GPU previously selected
    """
    global dev
    global my_gpu
    if 'my_gpu' in globals() and 'cuda' in str(my_gpu):
        dev = my_gpu
    else:
        # find free GPU
        os.system('nvidia-smi -q -d Memory |grep -A4 GPU|grep Used >tmp')
        memory_used = np.array([int(x.split()[2]) for x in open('tmp', 'r').readlines()])
        print(memory_used)
        ii = np.arange(len(memory_used))
        mask = memory_used < 4000
        print(mask)
        if mask.any():
            mask_index = np.argmin(memory_used[mask])
            index = (ii[mask])[mask_index]
            my_gpu = torch.device(index)
        else:
            my_gpu = torch.device('cpu')
        dev = my_gpu


# %%
# Load Network

model = torchvision.models.squeezenet1_0(weights=torchvision.models.SqueezeNet1_0_Weights.DEFAULT)

print(model)
dev = model.features[0].weight.device
print(dev)

# %% [markdown]
# ### Test Classification
# 
model.eval()
# Load Test Image
img = Image.open("dog.jpg")
# plt.figure()
# plt.imshow(img)
# plt.show()

# reshape as [C x H x W], normalize
transform = transforms.Compose([transforms.ToTensor(), transforms.Resize(224, antialias=True), transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225])])

# reshape as [B C H W], move to device
x = transform(img).unsqueeze(0).to(dev)

# Load labels
with open("imagenet_classes.txt", "r") as f:
    classes = [s.strip() for s in f.readlines()]

# TASK PART 1: compute predictions and print top 5 with confidences
# -------------Implement---------------
out = model.forward(x)
probs = F.softmax(out, dim=1).squeeze()
probs, indices = probs.sort(descending=True)

print("Top 5 classes and their probabilities:")
for i in range(5):
    print(f"{classes[indices[i]]}: {probs[i]:.4f}")

# %% [markdown]
# ### Network First Conv Filters and Features
# -------------Implement---------------
def normalize_to_0_1(t):
    t -= torch.amin(t, dim=(2, 3), keepdim=True)
    t /= torch.amax(t, dim=(2, 3), keepdim=True)
    return t

filters = normalize_to_0_1(model.features[0].weight.clone()).permute(0, 2, 3, 1)
_, axs = plt.subplots(8, 12)  # , figsize=(12, 12)
for f, ax in zip(filters, axs.flatten()):
    ax.imshow(f.detach().numpy())
    ax.axis('off')
plt.subplots_adjust(wspace=0, hspace=0)
plt.show()

def visualize_in_4_by_4_grid(x):
    _, axs = plt.subplots(4, 4)  # , figsize=(12, 12)
    x = x.clone().squeeze()
    x -= torch.amin(x, dim=(1, 2), keepdim=True)
    x /= torch.amax(x, dim=(1, 2), keepdim=True)

    for f, ax in zip(filters[:16], axs.flatten()):
        ax.imshow(f.detach().numpy())
        ax.axis('off')

    plt.subplots_adjust(wspace=0, hspace=0)
    plt.show()

visualize_in_4_by_4_grid(model.features[0:1](x))
visualize_in_4_by_4_grid(model.features[0:2](x))

# %% [markdown]
# ### Data Loader from Image Folders

# %%
# train_data = datasets.ImageFolder('/local/temporary/butterflies/train', transforms.ToTensor())
train_data = datasets.ImageFolder('butterflies/train', transforms.ToTensor())
# train_loader = torch.utils.data.DataLoader(train_data, batch_size=1, shuffle=True, num_workers=0)

# %%
# Normalization Statistics
# -------------Implement---------------
train_loader = torch.utils.data.DataLoader(train_data, batch_size=128, shuffle=True, num_workers=0)
mean = np.zeros(3)
std = np.zeros(3)

for x, _ in train_loader:
    # set channels as the first dimensions and flatten the other ones
    mean += x.permute(1, 0, 2, 3).reshape(3, -1).mean(dim=1).numpy()
mean /= len(train_loader)
mean_4d = torch.from_numpy(mean.reshape((1, 3, 1, 1)))

for x, _ in train_loader:
    # set channels as the first dimensions and flatten the other ones
    std += ((x - mean_4d) ** 2).permute(1, 0, 2, 3).reshape(3, -1).mean(dim=1).numpy()
std = np.sqrt(std / len(train_loader))

print(mean)
print(std)

mean=[0.485, 0.456, 0.406]
std=[0.229, 0.224, 0.225]

# %%
transform = transforms.Compose([transforms.ToTensor(), transforms.Normalize(mean=mean, std=std)])
# train_data = datasets.ImageFolder('/local/temporary/butterflies/train', transform)
# train_data = datasets.ImageFolder('butterflies/train', transform)


# split train data into training and validation
rng = np.random.default_rng(0)
all_indices = np.arange(len(train_data))
train_indices = rng.choice(all_indices, int(0.9 * len(train_data)), replace=False)
val_indices = np.array(list(set(all_indices) - set(train_indices)))

train_sampler = torch.utils.data.SubsetRandomSampler(train_indices)
val_sampler = torch.utils.data.SubsetRandomSampler(val_indices)

batch_size = 8
train_loader = torch.utils.data.DataLoader(train_data, batch_size=batch_size, sampler=train_sampler, shuffle=False, num_workers=0)
val_loader = torch.utils.data.DataLoader(train_data, batch_size=batch_size, sampler=val_sampler, shuffle=False, num_workers=0)

# -------------Implement---------------
# test set, using the same transform
test_data = datasets.ImageFolder('butterflies/test', transform)
# test_data = datasets.ImageFolder('/local/temporary/butterflies/test', transform)
test_loader = torch.utils.data.DataLoader(train_data, batch_size=batch_size, shuffle=True, num_workers=0)

# %% [markdown]
# ### Finetune last layer
model = torchvision.models.vgg11(weights=torchvision.models.VGG11_Weights.DEFAULT)
# %%
# Move model to device
# select_device()
model.to(dev)
print(model.features[0].weight.device)

# %%
def evaluate(model, loader):
    l = 0
    acc = 0

    for x,t in loader:
        score = model(x)
        l += F.nll_loss(torch.log_softmax(score, -1), t, reduction='mean')
        acc += (score.argmax(dim=1) == t).float().mean()

    l /= len(loader)
    acc /= len(loader)
    return l, acc

# %%
epochs = 10
model.eval()
# freeze all layers but the last one, re-initialze last layer
# ----------Implement----------
for param in model.parameters():
    param.requires_grad = False

# print(model.features)
model = nn.Sequential(
    model.features,
    nn.AdaptiveAvgPool2d((3, 3)),
    nn.Flatten(start_dim=1),
    nn.Linear(3 * 3 * 512, 100),
    nn.Dropout(0.5),
    nn.ReLU(),
    nn.Linear(100, 10),
)
# print(model)

# optimizer
optimizer = torch.optim.SGD(model.parameters(), lr = 0.01)
# train and validation loaders
# train_loader = torch.utils.data.DataLoader(train_set, batch_size=8, shuffle=True, num_workers=0)
# val_loader = torch.utils.data.DataLoader(val_set, batch_size=8, shuffle=True, num_workers=0)
# trianing loop

# I've had troubles with running PyCharm on the remote GPUs.
# It denies to give me a permission to run an environment script (python_ml from the courseware).
# I myself don't have enough power to train this network, so I'm sorry for that, I'll try to set it up next time

for e in range(epochs):
    for (x,t) in train_loader:
        x = x.to(dev)
        t = t.to(dev)
        score = model(x)
        log_p = torch.log_softmax(score, -1)
        l = F.nll_loss(log_p, t, reduction='sum')
        optimizer.zero_grad()
        l.backward()
        optimizer.step()
    # ----------Implement----------
    # Compute loss and accuracy using training set
    # Compute loss and accuracy using validation set
    # Save so-far best model and its validation accuracy
    print(f"Training loss/acc: {evaluate(model, val_loader)}")
    print(f"Validation loss/acc: {evaluate(model, val_loader)}")

# %%
## Test
test_loader = torch.utils.data.DataLoader(test_data, batch_size=8, shuffle=False, num_workers=0)
evaluate(test_loader)
# Error case analyzis
# ----------Implement----------
print(f"Validation loss/acc: {evaluate(model, test_loader)}")
# %% [markdown]
# ### Part 5: Data Augmentation
transforms = torch.nn.Sequential(
    transforms.RandomHorizontalFlip(0.5),
    transforms.RandomAffine(degrees=10, translate=(0.1, 0.3), scale=(0.9, 1.1)),
    transforms.RandomAdjustSharpness(1.1, p=0.5),
    transforms.Normalize((0.485, 0.456, 0.406), (0.229, 0.224, 0.225)),
)

