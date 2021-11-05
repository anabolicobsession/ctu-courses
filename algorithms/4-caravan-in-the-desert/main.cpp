#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <functional>
#include <iomanip>

#include <unistd.h>

#include "utils.h"

using namespace std;


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

const int INF = numeric_limits<int>::max() / 2;

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

    bool is_special(node_t n) const {
        return n <= n_special_nodes;
    }

    void explore_node(node_t n, node_t prev) {
        values_t new_values = values[prev];

//        cout << '(' << n << ',' << values[n][MOVES_LEFT] << ',' << values[n][COMPONENT] << ')';

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

        if (new_values[COMPONENT] < values[n][COMPONENT] ||
            (new_values[COMPONENT] == values[n][COMPONENT] && new_values[MOVES_LEFT] > values[n][MOVES_LEFT])) {
            values[n] = new_values;
        }

//        cout << " -> (" << n << "," << values[n][MOVES_LEFT] << ',' << values[n][COMPONENT] << ")" << '\n';
    }

    bool in_same_component(node_t a, node_t b) {
        return values[a][COMPONENT] == values[b][COMPONENT];
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
        bool was_empty = q.empty();

        while (!q.empty()) {
            cout << setw(setw_val) << setfill(' ') << q.front() << ' ';
            q.pop();
        }

        cout << '\n';
    }

    void set_values(node_t n, int moves_left, int component) {
        values[n][MOVES_LEFT] = moves_left;
        values[n][COMPONENT] = component;
    }

    vector<bool> merge_visited(vector<bool> a, vector<bool> b) {
        vector<bool> c(1 + n_nodes, false);

        for (int i = 1; i <= n_nodes; ++i) {
            c[i] = a[i] || b[i];
        }

        return c;
    }

    queue<node_t> merge_queues(queue<node_t> a, queue<node_t> b) {
        queue<node_t> new_q;
        vector<bool> enqueued(1 + n_nodes, false);

        for (auto q: {a, b}) {
            while (!q.empty()) {
                node_t n = q.front();

                if (!enqueued[n]) {
                    enqueued[n] = true;
                    new_q.push(n);
                }

                q.pop();
            }
        }

        return new_q;
    }

    void make_local_boom(node_t n, vector<bool> &visited, queue<node_t> &next_q) {
        queue<node_t> q, sn_q;
        sn_q.push(n);

        vector<bool> local_visited;
        visited[n] = true;

        while (!q.empty() || !sn_q.empty()) {
            if (!q.empty()) {
                n = q.front();
                q.pop();
            } else if (!sn_q.empty()) {
                n = sn_q.front();
                sn_q.pop();

                local_visited = vector<bool>(1 + n_nodes, false);
                local_visited[n] = true;
            }

            for (const auto &next: adj_list[n]) {
                explore_node(next, n);

                if (!local_visited[next]) {

                    if (in_same_component(next, n)) {

                        if (is_special(next)) {
                            if (!visited[next]) sn_q.push(next);
                        }
                        else {
                            q.push(next);
                        }
                    }
                    else {
                        next_q.push(next);
                    }

                    visited[next] = true;
                    local_visited[next] = true;
                }
            }
        }
    }

    void modified_bfs(Graph::node_t n) {
        queue<node_t> q, sn_q, next_q;
        sn_q.push(n);

        vector<bool> visited(1 + n_nodes, false), sn_visited;
        visited[n] = true;
        set_values(n, moves_per_component, 0);

        // O(): MOVES1 < MOVES2 => no continue

        bool sn_mode = false;
        int DEBUG = 1;

        while (!q.empty() || !sn_q.empty()) {
            bool node_found = false;

            while (!node_found && !q.empty()) {
                n = q.front();
                q.pop();

                // O(): Then take n again from q
                if (!is_special(n)) {
                    node_found = true;
                } else {
                    sn_q.push(n);
                }

//                if (q.empty() && sn_mode) {
//                    sn_mode = false;
//                }
            }

            if (!node_found) {
                n = sn_q.front();
                sn_q.pop();

                sn_mode = true;
                sn_visited = vector<bool>(1 + n_nodes, false);
                sn_visited[n] = true;
            }

            for (const auto &next: adj_list[n]) {
                explore_node(next, n); // move down

                if (!visited[next]) {
                    if (in_same_component(next, n)) {
                        q.push(next);
                    } else {
                        next_q.push(next);
                    }

                    visited[next] = true;
                }
                else if (sn_mode && !sn_visited[next]) {

                    if (in_same_component(next, n)) {
                        if (!is_special(next) || (is_special(next) && !visited[next])) {
                            q.push(next);
                        }
                    } else {
                        if (!visited[next]) {
                            next_q.push(next);
                        }
                    }

                    sn_visited[next] = true;
                    visited[next] = true;
                }
            }

            if (q.empty() && sn_q.empty()) {
                q = next_q;
                next_q = queue<node_t>();

                if (sn_mode) {
                    sn_mode = false;
                }
            }
        }
    }

    void print_answer() {
        int max_component = 0;
        int nodes_in_first_component = 0;
        for (int i = 1; i <= n_nodes; ++i) {
            int c = values[i][COMPONENT];
            if (c > max_component) max_component = c;
            if (c == 0) nodes_in_first_component++;
        }

        cout << max_component << ' ' << nodes_in_first_component << '\n';

//        for (int i = 1; i < n_nodes; ++i) {
//            if (values[i][COMPONENT] == INF) cout << i << ' ';
//        }
//        cout << '\n';
    }
};

int main() {
    SpecialGraph g;
    g.modified_bfs(1);
    g.print_answer();

    return 0;
}
