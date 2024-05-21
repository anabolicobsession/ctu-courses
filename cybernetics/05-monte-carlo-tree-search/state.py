import numpy as np

from game_board import GameBoard


class State:
    """A wrapping class for game board. Can be used as a dictionary key."""

    def __init__(self, board, my_color, opp_color):
        self.board = GameBoard(len(board), min(my_color, opp_color), max(my_color, opp_color))
        self.board.board = board
        self.__current_color = my_color
        self.all_colors = (my_color, opp_color)
        self.__is_terminal = False
        self.rng = np.random.default_rng()

    def get_actions(self):
        actions = self.board.get_all_valid_moves(self.__current_color)
        return actions if actions is not None else []

    def has_actions(self):
        return len(self.get_actions()) > 0

    def apply_action(self, action):
        self.board.play_move(action, self.__current_color)
        self.alter_color()

        if not self.has_actions():
            self.alter_color()

            if not self.has_actions():
                self.__is_terminal = True
                self.alter_color()

    def apply_random_action(self):
        self.apply_action(self.rng.choice(self.get_actions()))

    def apply_semi_random_action(self):
        edge = None

        for action in self.get_actions():
            x, y = action

            if (x == 0 or x == self.board.board_size - 1) and (y == 0 or y == self.board.board_size - 1):
                self.apply_action(action)
                return
            elif edge is None and ((x == 0 or x == self.board.board_size - 1) or (y == 0 or y == self.board.board_size - 1)):
                edge = (x, y)

        if edge is not None:
            self.apply_action(edge)
        else:
            self.apply_random_action()

    def random_simulation(self, action_fun):
        while not self.is_terminal():
            action_fun()
        return self.get_rewards()

    def get_rewards(self):
        assert self.is_terminal(), "Error: Can't get rewards for a non-terminal state"
        stones = self.board.get_score()
        r1 = 0 if stones[0] == stones[1] else (1 if stones[0] > stones[1] else -1)
        r2 = -r1
        return {self.board.p1_color: r1, self.board.p2_color: r2}

    def is_terminal(self):
        return self.__is_terminal

    def current_color(self):
        return self.__current_color

    def get_opposite_color(self):
        return self.all_colors[0] if self.__current_color == self.all_colors[1] else self.all_colors[1]

    def alter_color(self):
        self.__current_color = self.get_opposite_color()

    def __eq__(self, other):
        if self.__current_color == other.__current_color:
            for i in range(self.board.board_size):
                for j in range(self.board.board_size):
                    if self.board.board[i][j] != other.board.board[i][j]:
                        return False
            return True

        return False

    def __hash__(self):
        value = 0

        for i in range(self.board.board_size):
            for j in range(self.board.board_size):
                value += self.board.board[i][j] * (i * 10 + j + 1)

        return abs(value)
