#include <unordered_set>

#include "bfs.h"

std::shared_ptr<const state> bfs(std::shared_ptr<const state> root) {
    std::vector<std::shared_ptr<const state>> frontier, next_frontier;
    frontier.push_back(root);
    std::unordered_set<unsigned long long> visited;
    visited.insert(root->get_identifier());
    std::shared_ptr<const state> goal = nullptr;

    while (!frontier.empty() && goal == nullptr) {
        #pragma omp parallel for schedule(static) default(none) shared(frontier, next_frontier, visited, goal)
        for (unsigned int i = 0; i < frontier.size(); i++) {
            #pragma omp cancellation point for
            auto &current = frontier[i];

            if (current->is_goal()) {
                #pragma omp critical
                goal = current;
                #pragma omp cancel for
            }

            for (auto &child: current->next_states()) {
                if (visited.find(child->get_identifier()) == visited.end()) {
                    #pragma omp critical
                    {
                        next_frontier.push_back(child);
                        visited.insert(child->get_identifier());
                    }
                }
            }
        }

        if (goal != nullptr) {
            for (auto &s : frontier) {
                if (s->is_goal() && s->get_identifier() < goal->get_identifier()) {
                    goal = s;
                }
            }

            break;
        }

        frontier = next_frontier;
        next_frontier = std::vector<std::shared_ptr<const state>>();
    }

    return goal;
}
