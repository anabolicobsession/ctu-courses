import numpy as np
import torch


class FFModel():
    def __init__(self, hdim, device, dtype):
        """ hdim -- hidden layer size """
        # hidden layer
        self.w1 = torch.empty([2, hdim], dtype=dtype, device=device).uniform_(-1.0, 1.0)
        self.w2 = torch.empty([hdim, 1], dtype=dtype, device=device).uniform_(-1.0, 1.0)
        self.w1.requires_grad = True
        self.w2.requires_grad = True

        self.b1 = torch.empty([1, hdim], dtype=dtype, device=device).uniform_(-1.0, 1.0)
        self.b2 = torch.empty([1, 1], dtype=dtype, device=device).uniform_(-1.0, 1.0)
        self.b1.requires_grad = True
        self.b2.requires_grad = True

        self.parameters = [self.w1, self.b1, self.w2, self.b1]

    def score(self, x):
        """ Compute scores for inputs x 
        x : [N x d] 
        """
        if isinstance(x, np.ndarray):
            x = torch.tensor(x).to(self.w1)

        a1 = torch.maximum(torch.mm(x, self.w1) + self.b1, torch.zeros(1, 1))
        scores = torch.mm(a1, self.w2) + self.b2
        return torch.squeeze(scores)

    def classify(self, x):
        scores = self.score(x)
        return scores.sign()

    def mean_loss(self, x, y):
        """               
        Compute the mean_loss of the training data = average negative log likelihood
        *
        :param train_data: tuple(x,y)
        x [N x d]
        y [N], encoded +-1 classes
        :return: mean negative log likelihood
        """
        scores = self.score(x)
        e = torch.exp(scores)
        probs = e / (e + 1)

        for i in range(len(probs)):
            if y[i] == -1:
                probs[i] = 1 - probs[i]

        loss = -torch.mean(torch.log(probs))
        return loss

    def mean_accuracy(self, x, targets):
        y = self.classify(x)
        acc = (y == targets).float().mean()
        return acc

    def mean_accuracy_working_function(self, x, y):
        y_hat = self.classify(x)
        acc = np.mean(y == np.array(y_hat.detach()))
        return acc

    def zero_grad(self):
        # set .grad to None (or zeroes) for all parameters
        for p in self.parameters:
            p.grad = None

    def compute_loss_with_parameter(self, x, y, pname, pvalue):
        p = getattr(self, pname)
        setattr(self, pname, pvalue)
        loss = self.mean_loss(x, y)
        setattr(self, pname, p)
        return loss

    def check_gradient(self, x, y, pname, eps=1e-5):
        p = getattr(self, pname)

        u = torch.rand(p.shape)
        u /= torch.linalg.norm(u)
        der = torch.sum(p.grad * u)

        lp = self.compute_loss_with_parameter(x, y, pname, p + eps * u)
        lm = self.compute_loss_with_parameter(x, y, pname, p - eps * u)
        der_approx = (lp - lm) / (2 * eps)

        print("# Grad error in {}: {:.4}".format(pname, torch.abs(der - der_approx).item()))
