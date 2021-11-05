#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <iomanip>

#include "utils.h"

using namespace std;

const int INF = numeric_limits<int>::max() / 2;

class Graph {
public:
    typedef int node_t;
    vector<vector<node_t>> adj_list;
    int n_nodes, n_edges;

    Graph() : n_nodes(0), n_edges(0) {}

    void fill_with_edges(int number_of_nodes, int number_of_edges) {
        n_nodes = number_of_nodes;
        n_edges = number_of_edges;

        adj_list.resize(1 + n_nodes);
        for (int i = 0; i < n_edges; ++i) {
            node_t a = get_integer_from_stdin();
            node_t b = get_integer_from_stdin();
            adj_list[a].push_back(b);
            adj_list[b].push_back(a);
        }
    }

    void print_adj_list() {
        int setw_val = int(to_string(n_nodes).length());

        for (int i = 1; i <= n_nodes; ++i) {
            cout << '[' << setw(setw_val) << setfill(' ') << i << ']';
            for (const auto &n: adj_list[i]) cout << ' ' << setw(setw_val) << setfill(' ') << n;
            cout << '\n';
        }
    }
};

class SpecialGraph : public Graph {
public:
    typedef array<int, 2> values_t;
    int n_special_nodes, moves_per_component;
    vector<values_t> values;
    enum values_accessor_t {MOVES_LEFT, COMPONENT};

    SpecialGraph() : n_special_nodes(0), moves_per_component(0) {
        cin >> n_nodes >> n_edges >> n_special_nodes >> moves_per_component;
        fill_with_edges(n_nodes, n_edges);

        values.resize(n_nodes + 1);
        fill(values.begin(), values.end(), values_t{0, INF});
    }

    void print_values() {
        int setw_val = 3;

        for (int i = 1; i <= n_nodes; ++i) {
            cout << '[' << setw(setw_val - 1) << setfill(' ') << i << ']';
            for (const auto &v: values[i]) cout << ' ' << setw(setw_val) << setfill(' ') << (v < INF ? to_string(v) : "inf");
            cout << '\n';
        }
    }

    static void print_queue(queue<node_t> q) {
        node_t m = 0;
        queue<node_t> qc = q;
        while (!qc.empty()) {
            if (qc.front() > m) m = qc.front();
            qc.pop();
        }

        int setw_val = int(to_string(m).length());
        while (!q.empty()) {
            cout << setw(setw_val) << setfill(' ') << q.front() << ' ';
            q.pop();
        }
        cout << '\n';
    }

    void print_answer() {
        int max_component = 0;
        int nodes_in_first_component = 0;

        for (int i = 1; i <= n_nodes; ++i) {
            int c = values[i][COMPONENT];
            if (c > max_component) max_component = c;
            if (c == 0) nodes_in_first_component++;
        }

        cout << max_component << ' ' << nodes_in_first_component << '\n';;
    }

    bool is_special(const node_t &n) const {
        return n <= n_special_nodes;
    }

    void set_values(const node_t &n, const int &moves_left, const int &component) {
        values[n][MOVES_LEFT] = moves_left;
        values[n][COMPONENT] = component;
    }

    bool in_same_component(const node_t &a, const node_t &b) {
        return values[a][COMPONENT] == values[b][COMPONENT];
    }

    static bool values_better(const values_t &a, const values_t &b) {
        return a[COMPONENT] < b[COMPONENT] || (a[COMPONENT] == b[COMPONENT] && a[MOVES_LEFT] > b[MOVES_LEFT]);
    }

    bool update_node_from(const node_t &prev, const node_t &n) {
        values_t new_values = values[prev];

        if (!(is_special(prev) && is_special(n))) {
            // decrease number of moves left
            if (new_values[MOVES_LEFT] > 0) {
                new_values[MOVES_LEFT]--;
            } else {
                new_values[MOVES_LEFT] = moves_per_component;
                new_values[COMPONENT]++;
            }

            if (is_special(n)) new_values[MOVES_LEFT] = moves_per_component;
        }

        if (values_better(new_values, values[n])) {
            values[n] = new_values;
            return true;
        }

        return false;
    }

    void modified_bfs(node_t n) {
        queue<node_t> q, next_q;
        q.push(n);
        set_values(n, moves_per_component, 0);

        while (!q.empty()) {
            n = q.front();
            q.pop();

            for (const auto &next: adj_list[n]) {
                if (update_node_from(n, next)) {
                    if (in_same_component(n, next)) {
                        q.push(next);
                    } else {
                        next_q.push(next);
                    }
                }
            }

            if (q.empty()) {
                q = next_q;
                next_q = queue<node_t>();
            }
        }
    }
};

int main() {
    SpecialGraph g;
    g.modified_bfs(1);
    g.print_answer();

    return 0;
}
