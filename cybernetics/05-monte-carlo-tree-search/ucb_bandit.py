import numpy as np


class UCBBandit:
    def __init__(self, actions, exploration_factor=2):
        self.actions = actions
        self.qs = np.zeros(len(self.actions))
        self.visits = np.zeros(len(self.actions))
        self.exploration_factor = exploration_factor
        self.last_selected = None

    def select(self):
        self.visits[self.last_selected] += 1
        ucb = self.qs + self.exploration_factor * np.sqrt(np.log(np.sum(self.visits)) / self.visits)
        self.last_selected = np.argmax(ucb)
        return self.actions[self.last_selected]

    def update(self, reward):
        self.qs[self.last_selected] = (reward - self.qs[self.last_selected]) / self.visits[self.last_selected]

    def best_action(self):
        return self.actions[np.argmax(self.qs)]