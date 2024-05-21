import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.utils.data as td
import time
import torch.optim as optim
from torchvision import datasets, transforms


device = 'cuda' if torch.cuda.is_available() else 'cpu'


class FCNet(nn.Module):
    def __init__(self):
        super(FCNet, self).__init__()
        self.fc = nn.Linear(in_features=28 * 28,
                            out_features=10)

    def forward(self, x):
        x = torch.flatten(x, start_dim=1)
        x = self.fc(x)
        output = F.log_softmax(x, dim=1)
        return output


class SimpleCNN(nn.Module):
    def __init__(self):
        super(SimpleCNN, self).__init__()
        self.conv = nn.Conv2d(in_channels=1,
                              out_channels=10,
                              kernel_size=3,
                              stride=2,
                              padding=1)
        self.fc = nn.Linear(in_features=28 * 28 * 10 // (2 * 2),
                            out_features=10)

    def forward(self, x):
        x = self.conv(x)
        x = F.relu(x)
        x = torch.flatten(x, start_dim=1)
        x = self.fc(x)
        output = F.log_softmax(x, dim=1)
        return output

image_width = 28
image_height = 28
image_channels = 1
n_classes = 10


class ResNetBlock(nn.Module):
    def __init__(self, in_channels, out_channels, stride=1):
        super().__init__()
        self.stride = stride
        
        self.block = nn.Sequential(
            nn.Conv2d(in_channels, out_channels, 3, stride=stride, padding=1, bias=False),
            nn.BatchNorm2d(out_channels),
            nn.ReLU(),
            nn.Conv2d(out_channels, out_channels, 3, padding='same', bias=False),
            nn.BatchNorm2d(out_channels),
        )
        self.relu = nn.ReLU()
        
        self.downsample = nn.Sequential(
            nn.Conv2d(in_channels, out_channels, 1, stride=stride, bias=False),
            nn.BatchNorm2d(out_channels)
        )
    
    def forward(self, x):
        identity = x if self.stride == 1 else self.downsample(x)
        return self.relu(self.block(x) + identity)
    
class ResNetLayer(nn.Module):
    def __init__(self, blocks, in_channels, expand=True):
        super().__init__()
        
        stride = 2 if expand else 1
        out_channels = stride * in_channels
        layers = [ResNetBlock(in_channels, out_channels, stride)]
        
        for _ in range(1, blocks):
            layers.append(ResNetBlock(out_channels, out_channels))
            
        self.model = nn.Sequential(*layers)
    
    def forward(self, x):
        return self.model(x)

class MyNet(nn.Module):
    def __init__(self, blocks=16, first_channels=16, dropout=0.2):
        super().__init__()
        self.model = nn.Sequential(
            nn.Conv2d(image_channels, first_channels, 3, padding='same', bias=False),
            nn.BatchNorm2d(first_channels),
            nn.ReLU(),
            
            ResNetLayer(blocks, 2 ** 0 * first_channels, expand=False),
            ResNetLayer(blocks, 2 ** 0 * first_channels),
            ResNetLayer(blocks, 2 ** 1 * first_channels),
            ResNetLayer(blocks, 2 ** 2 * first_channels),
            
            nn.AdaptiveAvgPool2d((1, 1)),
            nn.Flatten(1),
            
            nn.Dropout(dropout),
            nn.Linear(2 ** 3 * first_channels, n_classes),
            nn.BatchNorm1d(n_classes),
            nn.LogSoftmax(dim=1)
        )
        
    def forward(self, x):
        return self.model(x)


def get_model_class(_):
    """ Do not change, needed for AE """
    return [MyNet]


def classify(model, x):
    return torch.argmax(model(x), dim=1)


def accuracy(pred, y):
    return pred.eq(y).sum().item() / len(y)


def error(pred, y):
    return pred.ne(y).sum().item() / len(y)


def load_model(model):
    model.load_state_dict(torch.load('model.pt'))

def train(model: nn.Module):
    batch_sz = 64

    learning_rate = 1
    epochs = 20

    dataset = datasets.FashionMNIST('data', train=True, download=True,
                                    transform=transforms.ToTensor())

    trn_size = int(0.09 * len(dataset))
    val_size = int(0.01 * len(dataset))
    add_size = len(dataset) - trn_size - val_size  # you don't need ADDitional dataset to pass

    trn_dataset, val_dataset, add_dataset = torch.utils.data.random_split(dataset, [trn_size,
                                                                                    val_size,
                                                                                    add_size])
    trn_loader = torch.utils.data.DataLoader(trn_dataset,
                                             batch_size=batch_sz,
                                             shuffle=True)

    val_loader = torch.utils.data.DataLoader(val_dataset,
                                             batch_size=batch_sz,
                                             shuffle=False)

    device = torch.device("cpu")
    model = model.to(device)

    optimizer = optim.SGD(model.parameters(), lr=learning_rate)

    for epoch in range(1, epochs + 1):
        # training
        model.train()
        for i_batch, (x, y) in enumerate(trn_loader):
            x, y = x.to(device), y.to(device)
            optimizer.zero_grad()
            net_output = model(x)
            loss = F.nll_loss(net_output, y)
            loss.backward()
            optimizer.step()

            if i_batch % 100 == 0:
                print('[TRN] Train epoch: {}, batch: {}\tLoss: {:.4f}'.format(
                    epoch, i_batch, loss.item()))

        # validation
        model.eval()
        correct = 0
        with torch.no_grad():
            for x, y in val_loader:
                x, y = x.to(device), y.to(device)
                net_output = model(x)

                prediction = classify(model, x)
                correct += prediction.eq(y).sum().item()
        val_accuracy = correct / len(val_loader.dataset)
        print('[VAL] Validation accuracy: {:.2f}%'.format(100 * val_accuracy))

        torch.save(model.state_dict(), "model.pt")


if __name__ == '__main__':
    train(SimpleCNN())
