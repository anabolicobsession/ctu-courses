#ifndef __GRAPH_H__
#define __GRAPH_H__

typedef struct {
    int start;
    int end;
    int cost;
} edge_t;

typedef struct {
    edge_t *edges;
    int capacity;
    int size;
} graph_t;

graph_t* allocate_graph();
void free_graph(graph_t **graph);
void load_txt(const char *fname, graph_t *graph);
void load_bin(const char *fname, graph_t *graph);
void save_txt(const graph_t * const graph, const char *fname);
void save_bin(const graph_t * const graph, const char *fname);

#endif
