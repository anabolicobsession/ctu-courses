#ifndef DATABASEQUERIES_QUERY_H
#define DATABASEQUERIES_QUERY_H

#include <vector>
#include <functional>
#include <atomic>

template<typename row_t>
using predicate_t = std::function<bool(const row_t &)>;

template<typename row_t>
bool is_satisfied_for_all(std::vector<predicate_t<row_t>> predicates, std::vector<row_t> data_table);

template<typename row_t>
bool is_satisfied_for_any(std::vector<predicate_t<row_t>> predicates, std::vector<row_t> data_table);


template<typename row_t>
bool is_satisfied_for_all(std::vector<predicate_t<row_t>> predicates, std::vector<row_t> data_table) {
    std::atomic<bool> ans{true};

    #pragma omp parallel for default(none) shared(predicates, data_table, ans)
    for (unsigned int i = 0; i < predicates.size(); i++) {
        #pragma omp cancellation point for
        auto &pred = predicates[i];
        bool satisfying_row_exists = false;

        for (unsigned int j = 0; j < data_table.size(); j++) {
            auto &row = data_table[j];
            bool is_satisfied = pred(row);

            if (is_satisfied) {
                satisfying_row_exists = true;
                break;
            }
        }

        if (!satisfying_row_exists) {
            ans = false;
            #pragma omp cancel for
        }
    }

    return ans;
}

template<typename row_t>
bool is_satisfied_for_any(std::vector<predicate_t<row_t>> predicates, std::vector<row_t> data_table) {
    std::atomic<bool> ans{false};

    #pragma omp parallel for default(none) shared(predicates, data_table, ans)
    for (unsigned int i = 0; i < data_table.size(); i++) {
        #pragma omp cancellation point for
        auto &row = data_table[i];

        for (unsigned int j = 0; j < predicates.size(); j++) {
            auto &pred = predicates[j];
            bool is_satisfied = pred(row);

            if (is_satisfied) {
                ans = true;
                #pragma omp cancel for
            }
        }
    }

    return ans;
}


#endif //DATABASEQUERIES_QUERY_H
