import random

import numpy as np


def init_policy(problem):
    policy = dict()
    for state in problem.get_all_states():
        if problem.is_goal_state(state):
            policy[state.x, state.y] = None
            continue
        actions = [action for action in problem.get_actions(state)]
        policy[state.x, state.y] = random.choice(actions)
    return policy


def init_utils(problem):
    '''
    Initialize all state utilities to zero except the goal states
    :param problem: problem - object, for us it will be kuimaze.Maze object
    :return: dictionary of utilities, indexed by state coordinates
    '''
    utils = dict()
    x_dims = problem.observation_space.spaces[0].n
    y_dims = problem.observation_space.spaces[1].n

    for x in range(x_dims):
        for y in range(y_dims):
            utils[(x,y)] = 0

    for state in problem.get_all_states():
        if not problem.is_terminal_state(state):
            utils[(state.x, state.y)] = problem.get_reward(state)
    return utils


def compute_state_action_value(problem, utils, state, action, discount_factor):
    return np.sum([prob * (problem.get_reward(new_state) + discount_factor * utils[new_state.x, new_state.y])
                   for new_state, prob
                   in problem.get_next_states_and_probs(state, action)])


def find_policy_via_policy_iteration(problem, discount_factor, theta=1e-3):
    policy = init_policy(problem)
    utils = init_utils(problem)
    is_stable = False

    while not is_stable:
        # policy evaluation
        max_change = np.inf
        while max_change >= theta:
            max_change = -np.inf

            for state in problem.get_all_states():
                if not problem.is_terminal_state(state):
                    old_value = utils[state.x, state.y]
                    utils[state.x, state.y] = compute_state_action_value(problem, utils, state, policy[state.x, state.y], discount_factor)
                    max_change = max(max_change, abs(utils[state.x, state.y] - old_value))

        # policy improvement
        is_stable = True
        for state in problem.get_all_states():
            if not problem.is_terminal_state(state):
                old_action = policy[state.x, state.y]
                actions = list(problem.get_actions(state))
                policy[state.x, state.y] = actions[np.argmax([compute_state_action_value(problem, utils, state, action, discount_factor)
                                                              for action in actions])]
                is_stable = is_stable and policy[state.x, state.y] == old_action

    return policy


def find_policy_via_value_iteration(problem, discount_factor, epsilon):
    utils = init_utils(problem)

    # value iteration
    max_change = np.inf
    while max_change >= epsilon:
        max_change = -np.inf

        for state in problem.get_all_states():
            if not problem.is_terminal_state(state):
                old_value = utils[state.x, state.y]
                utils[state.x, state.y] = np.max([compute_state_action_value(problem, utils, state, action, discount_factor)
                                                  for action in list(problem.get_actions(state))])
                max_change = max(max_change, abs(utils[state.x, state.y] - old_value))

    # extraction of the final policy
    policy = {}
    for state in problem.get_all_states():
        if not problem.is_terminal_state(state):
            actions = list(problem.get_actions(state))
            policy[state.x, state.y] = actions[np.argmax([compute_state_action_value(problem, utils, state, action, discount_factor)
                                               for action in actions])]
        else:
            policy[state.x, state.y] = None

    return policy
