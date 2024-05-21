from time import time

import numpy as np


def _extract_policy(q_table):
    """
    Internal helper for converting a Q-table to the corresponding policy dictionary.
    """
    # find the actions which maximize a reward
    policy_table = np.argmax(q_table, axis=-1)
    # convert a numpy array to a dictionary
    return {(y, x): policy_table[y][x] for y in range(policy_table.shape[0]) for x in range(policy_table.shape[1])}


def learn_policy(env, learning_rate=0.1, discount_factor=0.9, exploration_factor=0.1, time_limit=5):
    """
    Learn policy from interacting with the given environment using Q-learning.
    """
    rng = np.random.default_rng()
    start_time = time()
    q_table = np.zeros([env.observation_space.spaces[0].n, env.observation_space.spaces[1].n, env.action_space.n], dtype=float)
    state = env.reset()[0:2]

    while time() - start_time < time_limit:
        # epsilon-greedy exploration
        action = env.action_space.sample() if rng.random() < exploration_factor else np.argmax(q_table[state[0]][state[1]])
        obv, reward, is_done, _ = env.step(action)

        next_state = obv[0:2]
        q_table[state[0]][state[1]][action] += learning_rate * (reward +
                                                                discount_factor * np.max(q_table[next_state[0]][next_state[1]]) -
                                                                q_table[state[0]][state[1]][action])

        if is_done:
            state = env.reset()[0:2]
            continue
        else:
            state = next_state

    return _extract_policy(q_table)
