#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "graph.h"

#define START_NUM_EDGES 1024
#define REALLOC_MULTIPLY_NUM_EDGES_BY 2
#define MAX_INT_CHARS 10

graph_t* allocate_graph() {
    graph_t *graph = (graph_t *)malloc(sizeof(graph_t));
    graph->capacity = START_NUM_EDGES;
    graph->edges = (edge_t *)malloc(graph->capacity * sizeof(edge_t));
    graph->size = 0;
    return graph;
}

void free_graph(graph_t **graph) {
    free((*graph)->edges);
    free(*graph);
    *graph = NULL;
}

void realloc_graph(graph_t *graph) {
    graph->capacity *= REALLOC_MULTIPLY_NUM_EDGES_BY;
    graph->edges = (edge_t *)realloc(graph->edges, graph->capacity * sizeof(edge_t));
    assert(graph->edges != NULL);
}

int my_atoi(char *str, int len) {
    int num = 0;
    for (int i = 0; i < len; ++i) {
        num *= 10; 
        num += str[i] - '0';
    }
    return num;
}

int load_value(int *ptr, FILE *file, char stop_char) {
    char buffer[MAX_INT_CHARS + 1];
    if((buffer[0] = fgetc(file)) == EOF) return EOF;
    int i;
    for (i = 1; (buffer[i] = fgetc(file)) != stop_char; ++i);
    *ptr = my_atoi(buffer, i);
    return 0;
}

void load_txt(const char *fname, graph_t *graph) {
    FILE *file = fopen(fname, "r");
    assert(file != NULL);

    int i;
    for (i = 0; load_value(&(graph->edges[i].start), file, ' ') != EOF; ++i) { 
        load_value(&(graph->edges[i].end), file, ' ');
        load_value(&(graph->edges[i].cost), file, '\n');
        if (i >= graph->capacity - 1) realloc_graph(graph);
    }

    graph->size = i;
    assert(fclose(file) != EOF);
}

void load_bin(const char *fname, graph_t *graph) {
    FILE *file = fopen(fname, "rb");
    assert(file != NULL);

    int i;
    for (i = 0; fread(&(graph->edges[i]), sizeof(edge_t), 1, file) == 1; ++i) { 
        if (i >= graph->capacity - 1) realloc_graph(graph);
    }
    
    graph->size = i;
    assert(fclose(file) != EOF);
}

int value_length(int value) {
    int i;
    for (i = 1; value >= 10; ++i) value /= 10;
    return i;
}

void save_value(int value, FILE *file) {
    char buffer[MAX_INT_CHARS + 1];
    int len = value_length(value);

    int i;
    for (i = len - 1; i >= 0; --i) {
        buffer[i] = value % 10 + '0';
        value /= 10;
    }
    
    for (i = 0; i < len; ++i) {
        fputc(buffer[i], file);
    }
}

void save_txt(const graph_t * const graph, const char *fname) {
    FILE *file = fopen(fname, "w");
    assert(file != NULL);
    for (int i = 0; i < graph->size; ++i) {
        save_value(graph->edges[i].start, file);
        fputc(' ', file);
        save_value(graph->edges[i].end, file);
        fputc(' ', file);
        save_value(graph->edges[i].cost, file);
        fputc('\n', file);
    }
    assert(fclose(file) != EOF);
}

void save_bin(const graph_t * const graph, const char *fname) {
    FILE *file = fopen(fname, "wb");
    assert(file != NULL);
    fwrite(graph->edges, sizeof(edge_t), graph->size, file);
    assert(fclose(file) != EOF);
}
