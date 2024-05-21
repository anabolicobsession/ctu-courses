import kuimaze
from queue import PriorityQueue


def manhattan_distance(a, b):
    return abs(a[0] - b[0]) + abs(a[1] - b[1])


def reconstruct_path(predecessor, goal):
    path = []
    state = goal

    while state is not None:
        path.append(state)
        state = predecessor.get(state, None)

    path.reverse()
    return path


class Agent(kuimaze.BaseAgent):
    def __init__(self, environment):
        self.environment = environment

    def find_path(self):
        """
        Basic A* implementation.
        """
        observation = self.environment.reset()
        start = observation[0][0:2]
        goal = observation[1][0:2]
        heuristic = lambda state: manhattan_distance(state, goal)

        queue = PriorityQueue()
        queue.put((heuristic(start), start))

        # for path reconstructing in the future
        predecessor = {}

        # also known as g() function
        cost_dict = {start: 0}

        while not queue.empty():
            state = queue.get()[1]
            if state == goal:
                break
            new_states = self.environment.expand(state)

            for expanded in new_states:
                new_state, cost = expanded
                new_cost = cost_dict[state] + cost

                # add a state only if it hasn't been visited yet or if a new path is cheaper
                if new_state not in cost_dict or new_cost < cost_dict[new_state]:
                    cost_dict[new_state] = new_cost
                    queue.put((new_cost + heuristic(new_state), new_state))
                    predecessor[new_state] = state

        return reconstruct_path(predecessor, goal) if goal in predecessor else None
