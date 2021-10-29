#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <iomanip>

using namespace std;

const int INT_MAX_LEN = 10;
const int N_EDGE_NODES = 2;
const int N_NODE_NEIGHBORS = 3;
const int NEIGHBOR_DOESNT_EXIST = -1;

typedef int node_idx_t;

/**
 * Read 1 non-negative integer from standard input ignoring any non-digit characters.
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

class BinaryTree {
private:
    vector<array<node_idx_t, N_NODE_NEIGHBORS>> neighbors_table;
    int n_nodes, n_red_nodes;

public:
    BinaryTree() : n_nodes(0), n_red_nodes(0) {}

    void fill_with_edges_from_stdin() {
        n_nodes = get_integer_from_stdin();
        n_red_nodes = get_integer_from_stdin();

        neighbors_table.reserve(n_nodes);
        for (int i = 0; i < n_nodes; ++i) {
            for (int j = 0; j < N_NODE_NEIGHBORS; ++j) {
                neighbors_table[i][j] = NEIGHBOR_DOESNT_EXIST;
            }
        }

        for (int i = 0; i < n_nodes - 1; ++i) {
            // convert to indexing from zero
            array<node_idx_t, N_EDGE_NODES> edge_nodes = {get_integer_from_stdin() - 1, get_integer_from_stdin() - 1};

            for (int j = 0; j < N_EDGE_NODES; ++j) {
                int k = 0;
                while (neighbors_table[edge_nodes[j]][k] != NEIGHBOR_DOESNT_EXIST) ++k;
                neighbors_table[edge_nodes[j]][k] = edge_nodes[N_EDGE_NODES - j - 1];
            }
        }
    }

    void print() {
        int setw_value = int(to_string(n_nodes).length());
        for (int i = 0; i < n_nodes; ++i) {
            cout << "[" << setw(setw_value) << setfill(' ') << i << "]";
            for (int j = 0; j < N_NODE_NEIGHBORS; ++j) {
                if (neighbors_table[i][j] != NEIGHBOR_DOESNT_EXIST) cout << ' ' << setw(setw_value) << setfill(' ') << neighbors_table[i][j];
                else cout << ' ' << setw(setw_value) << setfill(' ') << ' ';
            }
            cout << '\n';
        }
    }
};

int main() {
    BinaryTree bt;
    bt.fill_with_edges_from_stdin();
    bt.print();

    return 0;
}
