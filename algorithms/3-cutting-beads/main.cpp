#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <iomanip>

using namespace std;

const int INT_MAX_LEN = 10;
const int N_EDGE_NODES = 2;
const int EDGE_START = 0;
const int EDGE_END = 1;
const int NODE_DOESNT_EXIST = -1;

typedef int node_idx_t;
typedef array<node_idx_t, N_EDGE_NODES> edge_t;

/**
 * Read one non-negative integer from standard input, ignoring any non-digit characters.
 *
 * @return integer if one was in input, EOF otherwise.
 */
int get_integer_from_stdin() {
    unsigned char str[INT_MAX_LEN];
    int c = 0, i = 0;

    while ((c = getchar()) != EOF && !isdigit(c));
    while (c != EOF && isdigit(c)) {
        str[i++] = (unsigned char)c;
        c = getchar();
    }

    int num = 0, m = 1;
    while (i--) {
        num += (str[i] - '0') * m;
        m *= 10;
    }

    return m != 1 ? num : EOF;
}

class Tree {
private:
    static const int N_MAX_NODE_NEIGHBORS = 3;
    typedef array<node_idx_t, N_MAX_NODE_NEIGHBORS> node_neighbors_t;

public:
    vector<node_neighbors_t> neighbors_table;
    int n_nodes, n_red_nodes;

    Tree() : n_nodes(0), n_red_nodes(0) {}

    void fill_with_stdin() {
        n_nodes = get_integer_from_stdin();
        n_red_nodes = get_integer_from_stdin();

        neighbors_table.reserve(n_nodes);
        for (int i = 0; i < n_nodes; ++i) {
            for (int j = 0; j < N_MAX_NODE_NEIGHBORS; ++j) {
                neighbors_table[i][j] = NODE_DOESNT_EXIST;
            }
        }

        for (int i = 0; i < n_nodes - 1; ++i) {
            // convert to indexing from zero
            array<node_idx_t, N_EDGE_NODES> edge_nodes = {get_integer_from_stdin() - 1, get_integer_from_stdin() - 1};
            for (int j = 0; j < N_EDGE_NODES; ++j) {
                neighbors_table[edge_nodes[j]][get_node_degree(edge_nodes[j])] = edge_nodes[N_EDGE_NODES - j - 1];
            }
        }
    }

    int get_node_degree(node_idx_t node) {
        int degree = 0;
        while (degree < N_MAX_NODE_NEIGHBORS && neighbors_table[node][degree] != NODE_DOESNT_EXIST) degree++;
        return degree;
    }

    edge_t get_any_directed_edge_with_start_node_degree_1() {
        for (int i = 0; i < n_nodes; ++i) {
            if (get_node_degree(i) == 1) return {i, neighbors_table[i][0]};
        }
        // never reachable, because there are always at least two nodes with degree one in a tree
        return {NODE_DOESNT_EXIST, NODE_DOESNT_EXIST};
    }

    vector<edge_t> get_next_directed_edges(edge_t edge) {
        vector<edge_t> next_edges;
        node_neighbors_t neighbors = neighbors_table[edge[EDGE_END]];
        for (int i = 0; i < N_MAX_NODE_NEIGHBORS && neighbors[i] != NODE_DOESNT_EXIST; ++i) {
            if (neighbors[i] != edge[EDGE_START]) next_edges.push_back({edge[EDGE_END], neighbors[i]});
        }
        return next_edges;
    }

    void print_neighbors_table() {
        int setw_val = int(to_string(n_nodes).length());
        for (int i = 0; i < n_nodes; ++i) {
            cout << "[" << setw(setw_val) << setfill(' ') << i << "]";
            for (int j = 0; j < N_MAX_NODE_NEIGHBORS; ++j) {
                if (neighbors_table[i][j] != NODE_DOESNT_EXIST) cout << ' ' << setw(setw_val) << setfill(' ') << neighbors_table[i][j];
                else cout << ' ' << setw(setw_val) << setfill(' ') << ' ';
            }
            cout << '\n';
        }
    }
};

bool edge_less(const edge_t &a, const edge_t &b) {
    int as = a[EDGE_START], ae = a[EDGE_END];
    int bs = b[EDGE_START], be = b[EDGE_END];

    if (as > ae) swap(as, ae);
    if (bs > be) swap(bs, be);

    return as < bs || (as == bs && ae < be);
}

int compute_node_value(Tree &tr, node_idx_t node) {
    return int(node < tr.n_red_nodes);
}

int compute_minimized_metrics(Tree &tr, int val) {
    // use multiplication by 2 to avoid non-integer types
    return abs(tr.n_red_nodes - 2 * val);
}

int solve_subtree(Tree &tr, edge_t directed_edge, int &best_subtree_val, edge_t &best_directed_edge) {
    int subtree_val = 0;
    for (const auto &de: tr.get_next_directed_edges(directed_edge)) {
        subtree_val += solve_subtree(tr, de, best_subtree_val, best_directed_edge);
    }
    subtree_val += compute_node_value(tr, directed_edge[EDGE_END]);

    int metrics_val = compute_minimized_metrics(tr, subtree_val);
    int metrics_best_val = compute_minimized_metrics(tr, best_subtree_val);

    if (metrics_val < metrics_best_val || (metrics_val == metrics_best_val && edge_less(directed_edge, best_directed_edge))) {
        best_subtree_val = subtree_val;
        best_directed_edge = directed_edge;
    }

    return subtree_val;
}

int main() {
    Tree tr;
    tr.fill_with_stdin();

    int best_subtree_val = 0;
    edge_t best_directed_edge = {NODE_DOESNT_EXIST, NODE_DOESNT_EXIST};
    solve_subtree(tr, tr.get_any_directed_edge_with_start_node_degree_1(), best_subtree_val, best_directed_edge);

    // convert to the node indexing from the assignment
    edge_t best_edge = {best_directed_edge[0] + 1, best_directed_edge[1] + 1};
    if (best_edge[EDGE_START] > best_edge[EDGE_END]) {
        swap(best_edge[EDGE_START], best_edge[EDGE_END]);
    }
    cout << best_edge[EDGE_START] << " " << best_edge[EDGE_END] << '\n';

    return 0;
}
