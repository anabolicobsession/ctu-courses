#include "sort.h"
#include <iostream>

void radix_par_worker(std::vector<std::string *> &vector_to_sort, const MappingFunction &mappingFunction,
                      unsigned long alphabet_size, unsigned long string_lengths) {
    std::vector<std::vector<std::string *>> buckets(alphabet_size);
    unsigned int char_idx = vector_to_sort[0]->size() - string_lengths;

    for (unsigned long i = 0; i < vector_to_sort.size(); i++) {
        auto idx = mappingFunction(vector_to_sort[i]->at(char_idx));
        buckets[idx].push_back(vector_to_sort[i]);
    }

    if (string_lengths > 1) {
        for (unsigned long i = 0; i < buckets.size(); i++) {
            if (!buckets[i].empty()) {
#pragma omp task default(none) shared(buckets, i, mappingFunction, alphabet_size, string_lengths)
                radix_par(buckets[i], mappingFunction, alphabet_size, string_lengths - 1);
            }
        }
    }

#pragma omp taskwait
    for (unsigned long i = 0, idx = 0; i < buckets.size(); i++) {
        for (unsigned long j = 0; j < buckets[i].size(); j++) {
            vector_to_sort[idx++] = buckets[i][j];
        }
    }
}

void radix_par(std::vector<std::string *> &vector_to_sort, const MappingFunction &mappingFunction,
               unsigned long alphabet_size, unsigned long string_lengths) {
#pragma omp parallel default(none) shared(vector_to_sort, mappingFunction, alphabet_size, string_lengths)
#pragma omp single
    radix_par_worker(vector_to_sort, mappingFunction, alphabet_size, string_lengths);
}