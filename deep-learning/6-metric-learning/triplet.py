import matplotlib.pyplot as plt
import numpy as np

import torch
import torchvision
import torchvision.transforms as transforms
from torch.utils.data.sampler import SubsetRandomSampler

import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
import pickle
import time

from tools import *
from optparse import OptionParser

# query if we have GPU
dev = torch.device('cuda') if torch.cuda.is_available() else torch.device('cpu')
print('Using device:', dev)

# Global datasets
transform = transforms.Compose([transforms.ToTensor(), transforms.Normalize((0.5,), (0.5,))])
# training and validation sets
train_set = Dataset_to_XY(torchvision.datasets.MNIST('../data', download=True, train=True, transform=transform))
train_set, val_set = train_set.split(valid_size=0.1)
# test set
test_set = Dataset_to_XY(torchvision.datasets.MNIST('../data', download=True, train=False, transform=transform))
test_set = test_set.fraction(fraction=0.1)
#
# dataloaders
train_loader = torch.utils.data.DataLoader(train_set, batch_size=64, shuffle=True, num_workers=0)
val_loader = torch.utils.data.DataLoader(val_set, batch_size=64, shuffle=False, num_workers=0)
test_loader = torch.utils.data.DataLoader(test_set, batch_size=64, shuffle=False, num_workers=0)
#

model_names = ['./models/' + name for name in ['net_class.pl', 'net_triplet.pl']]



class ConvNet(nn.Sequential):
    def __init__(self, num_classes: int = 10) -> None:
        layers = []
        layers += [nn.Conv2d(1, 32, kernel_size=3)]
        layers += [nn.ReLU(inplace=True)]
        layers += [nn.MaxPool2d(kernel_size=2, stride=2)]
        layers += [nn.Conv2d(32, 32, kernel_size=3)]
        layers += [nn.ReLU(inplace=True)]
        layers += [nn.MaxPool2d(kernel_size=2, stride=2)]
        layers += [nn.Conv2d(32, 64, kernel_size=3)]
        layers += [nn.ReLU(inplace=True)]
        layers += [nn.AdaptiveAvgPool2d((2, 2))]
        layers += [nn.Flatten()]
        layers += [nn.Linear(64 * 2 * 2, num_classes)]
        super().__init__(*layers)
        self.layers = layers

    def features(self, x):
        f = nn.Sequential(*self.layers[:-1]).forward(x)
        f = nn.functional.normalize(f, p=2, dim=1)
        return f


def new_net():
    return ConvNet().to(dev)


def load_net(filename):
    net = ConvNet()
    net.to(dev)
    net.load_state_dict(torch.load(filename,map_location=dev))
    return net



def distances(f1: torch.Tensor, f2: torch.Tensor):
    """All pairwise distancesbetween feature vectors in f1 and feature vectors in f2:
    f1: [N, d] array of N normalized feature vectors of dimension d
    f2: [M, d] array of M normalized feature vectors of dimension d
    return D [N,M] -- pairwise Euclidean distance matrix
    """
    assert (f1.dim() == 2)
    assert (f2.dim() == 2)
    assert (f1.size(1) == f2.size(1))


def evaluate_AP(dist: np.array, labels: np.array, query_label: int):
    """Average Precision
    dits: [N] array of distances to all documents from the query
    labels: [N] labels of all documents
    query_label: label of the query document
    return: AP -- average precision, Prec -- Precision, Rec -- Recall
    """
    ii = np.argsort(dist)
    dist = dist[ii]
    labels = labels[ii]
    rel = np.equal(labels, query_label).astype(int)
    print(str.join('', ['.' if r > 0 else 'X' for r in rel[0:100]]))
    AP = 0
    Prec = 0
    Rec = 0
    return AP, Prec, Rec

def evaluate_mAP(net, dataset: DataXY):
    """
    Compute Mean Average Precision
    net: network with method features()
    dataset: dataset of input images and labels
    Returns: mAP -- mean average precision, mRec -- mean recall, mPrec -- mean precision
    """
    torch.manual_seed(1)
    loader = torch.utils.data.DataLoader(dataset, batch_size=64, shuffle=True, num_workers=0)
    """ use first 100 documents from this loader as queries, use all documents as possible items to retrive, exclude the query from the retrieved items """
    mAP = 0
    return mAP, mPrec, mRrec


rng = np.random.default_rng()


def extract_x_y(dataset):
    for x, y in torch.utils.data.DataLoader(dataset, batch_size=len(dataset), shuffle=True, num_workers=0):
        return x.to(dev), y.to(dev)


def get_indices_of_nearest_features(net, x):
    features = net.features(x).detach().numpy()
    differences = np.expand_dims(features, 1) - np.expand_dims(features, 0)
    distances = np.linalg.norm(differences, axis=2)
    return np.argsort(distances, axis=1)


def my_evaluate_AP(y, y_query, class_cardinality):
    rel = (y == y_query).astype("int")
    cs = np.cumsum(rel)
    prec = cs / np.arange(1, 1 + len(rel))
    recall_delta = rel / class_cardinality
    return np.sum(prec * recall_delta), prec, cs / class_cardinality


def my_evaluate_mAP(net, dataset, n_samples=100):
    x, y = extract_x_y(dataset)
    y = y.detach().numpy()
    unique, counts = np.unique(y, return_counts=True)
    nearest_indices = get_indices_of_nearest_features(net, x)
    random_indices = rng.integers(len(y), size=n_samples)

    mAP = 0
    mprec = np.zeros(len(y) - 1)
    mrec = np.zeros(len(y) - 1)

    for i in random_indices:
        class_cardinality = counts[np.argwhere(unique == y[i])[0]]
        # excluding the query
        AP, prec, rec = my_evaluate_AP(y[nearest_indices[i]][1:], y[i], class_cardinality - 1)
        mAP += AP
        mprec += prec
        mrec += rec

    return mAP / n_samples, mprec / n_samples, mrec / n_samples

def evaluate_acc(net, loss_f, loader):
    net.eval()
    with torch.no_grad():
        acc = 0
        loss = 0
        n_data = 0
        for i, (data, target) in enumerate(test_loader):
            data, target = data, target
            y = net(data)
            l = loss_f(y, target)
            loss += l.sum()
            acc += (torch.argmax(y, dim=1) == target).float().sum().item()
            n_data += data.size(0)
        acc /= n_data
        loss /= n_data
    return (loss, acc)


def train_class(net, train_loader, val_loader, epochs=20, name: str = None):
    loss_f = nn.CrossEntropyLoss(reduction='none')
    optimizer = optim.SGD(net.parameters(), lr=0.001, momentum=0.9)

    for epoch in range(epochs):
        print("Epoch {}".format(epoch))
        train_acc = 0
        train_loss = 0
        n_train_data = 0
        net.train()
        for i, (data, target) in enumerate(train_loader):
            y = net(data)
            l = loss_f(y, target)
            train_loss += l.sum().item()
            train_acc += (torch.argmax(y, dim=1) == target).float().sum().item()
            n_train_data += data.size(0)
            optimizer.zero_grad()
            l.mean().backward()
            optimizer.step()
        train_loss /= n_train_data
        train_acc /= n_train_data
        #
        val_loss, val_acc = evaluate_acc(net, loss_f, val_loader)
        print(f'Epoch: {epoch} mean loss: {train_loss}')
        print("Train accuracy {}, Val accuracy: {}".format(train_acc, val_acc))
        if name is not None:
            torch.save(net.state_dict(), name)


def select_samples(samples, y, anchor_idx, negative=False):
    boolean = y == y[anchor_idx] if not negative else y != y[anchor_idx]
    boolean[anchor_idx] = False
    indices = torch.flatten(torch.nonzero(boolean))
    return samples[anchor_idx][indices]


def triplet_loss(features: torch.Tensor, y: torch.Tensor, alpha=0.5, n_anchors=10) -> torch.Tensor:
    """
    triplet loss
    features [N, d] tensor of features for N data points
    labels [N] true labels of the data points

    Implement: max(0, d(a,p) - d(a,n) + alpha )) for a=0:10 and all valid p,n in the batch
    """
    distances = torch.cdist(features[:n_anchors], features)
    L = torch.zeros(1).to(dev)
    zero = torch.zeros(1).to(dev)

    for a in range(n_anchors):
        positive = select_samples(distances, y, a)
        negative = select_samples(distances, y, a, negative=True)

        # there's a slight chance that a batch will not contain any positives, so we need to deal with that case
        if positive.numel() > 0 and negative.numel() > 0:
            # differences between every possible pair of positives and negatives
            differences = torch.unsqueeze(positive, 1) - torch.unsqueeze(negative, 0)
            L += torch.sum(torch.maximum(torch.flatten(differences) + alpha, zero))

    return L


def train_triplets(net, train_loader, lr=1e-3, epochs=1, name: str = None):
    """
    training with triplet loss
    """
    print("Running training...")
    optim = torch.optim.SGD(net.parameters(), lr=lr)
    mloss = np.zeros(epochs)
    t1 = time.time()

    for e in range(epochs):
        for x, y in train_loader:
            x, y = x.to(dev), y.to(dev)
            l = triplet_loss(net.features(x), y)
            mloss[e] += l.item()
            optim.zero_grad()
            l.backward()
            optim.step()

        mloss[e] /= len(train_loader)
        td = int(time.time() - t1)
        print(f"Mean loss during the epoch #{1 + e} in {td}s: {mloss[e]:.3f}")

    return mloss

if __name__ == '__main__':
    op = OptionParser()
    op.add_option("--train", type=int, default=-1, help="run training: 0 -- classification loss, 1 -- triplet loss")
    op.add_option("--eval", type=int, default=-1, help="run evaluation: 0 -- classification loss, 1 -- triplet loss")
    op.add_option("-e", "--epochs", type=int, default=1, help="training epochs")
    (opts, args) = op.parse_args()
    o = dotdict(**vars(opts))

    if o.train == 0:
        net = ConvNet(10)
        net.to(dev)
        train_class(net, train_loader, val_loader, epochs=o.epochs)
        torch.save(net.state_dict(), model_names[0])

    if o.train == 1:
        net = ConvNet(10)
        net.to(dev)
        mloss = train_triplets(net, train_loader, epochs=o.epochs, name=model_names[1])
        torch.save(net.state_dict(), model_names[1])

        plt.plot(mloss)
        plt.title("Training loss")
        plt.ylabel("Training loss")
        plt.xlabel("Epoch")
        plt.savefig("training_loss")

    if o.eval > -1:
        net = ConvNet(10)
        net.to(dev)
        net.load_state_dict(torch.load(model_names[1], map_location=dev))
        mAP, mPrec, mRec = my_evaluate_mAP(net, test_set)
        plt.plot(mRec, mPrec, label=f"Triplet loss (mAP={mAP:.2f})")

        net = ConvNet(10)
        net.to(dev)
        net.load_state_dict(torch.load(model_names[0], map_location=dev))
        mAP, mPrec, mRec = my_evaluate_mAP(net, test_set)
        plt.plot(mRec, mPrec, label=f"Cross-entropy (mAP={mAP:.2f})")

        plt.title("Precision-recall curve for different losses")
        plt.ylabel("Precision")
        plt.xlabel("Recall")
        plt.legend(loc="lower left")
        plt.savefig("precision_recall")

        # loss_f = nn.CrossEntropyLoss(reduction='none')
        # loss, acc = evaluate_acc(net, loss_f, test_loader)
        # print(f"Test accuracy: {acc*100:3.2f}%")
        # print(f"Test mAP: {mAP:3.2f}")