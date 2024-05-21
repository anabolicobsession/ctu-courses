import time
from copy import deepcopy
from typing import Dict

from state import State
from ucb_bandit import UCBBandit


class MyPlayer:
    """Monte Carlo tree search with semi-random simulation"""

    def __init__(self, my_color, opponent_color, board_size=8):
        self.name = 'saranego'
        self.my_color = my_color
        self.opp_color = opponent_color

        self.tree: Dict[State, UCBBandit] = {}
        self.time_limit = 4.9

    def move(self, board, verbose=False):
        current_state = State(deepcopy(board), self.my_color, self.opp_color)
        start_time = time.time()
        simulations = 0

        # a classic MCTS loop: selection -> expansion -> simulation -> backpropagation
        while (time.time() - start_time) < self.time_limit:
            state = deepcopy(current_state)

            if not self.has_node(state):
                self.add_node(deepcopy(state))

            trace = self.select(state)

            if state.is_terminal():
                rewards = state.get_rewards()
            else:
                self.add_node(state)
                rewards = state.random_simulation(state.apply_semi_random_action)

            self.backpropagation(trace, rewards)
            simulations += 1

        if verbose:
            print(f'Explored tree of depth {self.__estimate_tree_depth(deepcopy(current_state))} with speed {int(simulations / self.time_limit):4} simulations/s')

        return self.get_bandit(current_state).best_action()

    def has_node(self, state):
        return state in self.tree

    def add_node(self, state):
        self.tree[deepcopy(state)] = UCBBandit(state.get_actions())

    def get_bandit(self, state):
        return self.tree[state]

    def select(self, state):
        trace = []

        while self.has_node(state) and not state.is_terminal():
            trace.append(deepcopy(state))
            state.apply_action(self.get_bandit(state).select())

        return trace

    def backpropagation(self, trace, rewards):
        for state in trace:
            self.get_bandit(state).update(rewards[state.current_color()])

    def __estimate_tree_depth(self, state):
        best = state
        depth = -1

        while self.has_node(best) and not best.is_terminal():
            best.apply_action(self.get_bandit(best).best_action())
            depth += 1

        return depth
