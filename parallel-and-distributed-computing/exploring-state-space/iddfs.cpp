#include "iddfs.h"

void dls(const std::shared_ptr<const state> &current, unsigned int max_cost, std::shared_ptr<const state> &goal) {
    if (current->current_cost() == max_cost) {
        if (current->is_goal()) {
            #pragma omp critical
            if (goal == nullptr || current->get_identifier() < goal->get_identifier()) {
                goal = current;
            }
        }

        return;
    }

    for (auto &child : current->next_states()) {
        if (current->get_predecessor() == nullptr || child->get_identifier() != current->get_predecessor()->get_identifier()) {
            #pragma omp task default(none) firstprivate(child) shared(max_cost, goal)
            if (child->current_cost() <= max_cost) {
                dls(child, max_cost, goal);
            }
        }
    }
}

std::shared_ptr<const state> iddfs(std::shared_ptr<const state> root) {
    std::shared_ptr<const state> goal = nullptr;

    for (int max_cost = 0; goal == nullptr; max_cost++) {
        #pragma omp parallel default(none) shared(root, max_cost, goal)
        #pragma omp single
        dls(root, max_cost, goal);
    }

    return goal;
}
