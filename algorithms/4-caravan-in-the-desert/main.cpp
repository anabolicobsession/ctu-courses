#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <functional>
#include <iomanip>

#include "utils.h"

using namespace std;


class PriorityQueue {
public:
    typedef array<int, 2> elem_t;
    enum accessor_t {VALUE, PRIORITY};

private:
    struct PriorityQueueCompare {
        bool operator()(const elem_t &a, const elem_t &b)  {
            return b[PRIORITY] < a[PRIORITY];
        }
    };

    priority_queue<elem_t, vector<elem_t>, PriorityQueueCompare> pq;

public:
    PriorityQueue() = default;

    void push(elem_t e) {
        pq.push(e);
    }

    elem_t pop() {
        elem_t e = pq.top();
        pq.pop();
        return e;
    }

    bool empty() {
        return pq.empty();
    }

    void print() {
        priority_queue<elem_t, vector<elem_t>, PriorityQueueCompare> pq_copy = pq;
        bool was_empty = pq_copy.empty();
        while (!pq_copy.empty()) {
            cout << '(' << pq_copy.top()[VALUE] << ',' << pq_copy.top()[PRIORITY] << ')' << ' ';
            pq_copy.pop();
        }
        if (!was_empty) cout << '\n';
    }
};


class Graph {
public:
    typedef int node_t;
    vector<vector<node_t>> adj_list;
    int n_nodes, n_edges;

    Graph() : n_nodes(0), n_edges(0) {}

    void fill_with_edges(int number_of_nodes, int number_of_edges) {
        n_nodes = number_of_nodes;
        n_edges = number_of_edges;

        adj_list.resize(n_nodes + 1);
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

const int INT_INF = numeric_limits<int>::max();

class SpecialGraph : public Graph {
public:
    typedef array<int, 2> values_t;
    int n_special_nodes, n_moves_per_component;
    vector<values_t> values;
    enum values_accessor_t {MOVES_LEFT, COMPONENT};

    SpecialGraph() : n_special_nodes(0), n_moves_per_component(0) {
        cin >> n_nodes >> n_edges >> n_special_nodes >> n_moves_per_component;
        fill_with_edges(n_nodes, n_edges);

        values.resize(n_nodes + 1);
        fill(values.begin(), values.end(), values_t{0, INT_INF});
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
                new_values[MOVES_LEFT] = n_moves_per_component;
                new_values[COMPONENT]++;
            }

            if (is_special(n)) new_values[MOVES_LEFT] = n_moves_per_component;
        }

        if (new_values[COMPONENT] < values[n][COMPONENT] ||
            new_values[COMPONENT] == values[n][COMPONENT] && new_values[MOVES_LEFT] > values[n][MOVES_LEFT]) {
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
            for (const auto &v: values[i]) cout << ' ' << setw(setw_val) << setfill(' ') << (v < INT_INF ? to_string(v) : "inf");
            cout << '\n';
        }
    }

    int compute_node_priority(node_t node, vector<bool> visited) {
        int moves = 0;
        if (is_special(node)) return moves;

        queue<node_t> q, next_q;
        q.push(node);

        moves++;
        while (!q.empty()) {
            if (moves > values[node][MOVES_LEFT]) break;

            node_t n = q.front();
            q.pop();

            for (const auto &next: adj_list[n]) {
                if (!visited[next]) {
                    if (is_special(next)) {
                        return moves;
                    } else {
                        visited[next] = true;
                        next_q.push(next);
                    }
                }
            }

            if (q.empty()) {
                moves++;
                q = next_q;
                next_q = queue<node_t>();
            }
        }

        return INT_INF - values[node][MOVES_LEFT];
    }

    void modified_bfs(Graph::node_t n) {
        vector<bool> visited(1 + n_nodes, false);
        visited[n] = true;

        PriorityQueue pq, next_pq;
        pq.push(PriorityQueue::elem_t{n, 0});

        values[n][SpecialGraph::MOVES_LEFT] = n_moves_per_component;
        values[n][SpecialGraph::COMPONENT] = 0;

        int DEBUG = 0;

        while(!pq.empty()) {
            if (DEBUG) {
                pq.print();
//                print_values();
//                cout << '\n';
            }

            bool best_node_found = false;

            while (!best_node_found) {
                PriorityQueue::elem_t pq_elem = pq.pop();
                n = pq_elem[PriorityQueue::VALUE];
                int real_priority = compute_node_priority(n, visited);

                if (pq_elem[PriorityQueue::PRIORITY] == real_priority) {
                    best_node_found = true;
                } else {
                    pq.push({pq_elem[PriorityQueue::VALUE], real_priority});
                }
            }

            for (const auto &next: adj_list[n]) {
                explore_node(next, n);

                if (!visited[next]) {
                    visited[next] = true;
                    PriorityQueue::elem_t e{next, compute_node_priority(next, visited)};

                    if (in_same_component(next, n)) {
                        pq.push(e);
                    } else {
                        next_pq.push(e);
                    }

//                    if (DEBUG) cout << "+(" << e[PriorityQueue::VALUE] << "," << e[PriorityQueue::PRIORITY] << ")" << '\n';
                }
            }

            if (pq.empty()) {
                if (DEBUG && !pq.empty()) cout << "-------------------- New depth --------------------" << '\n';

                pq = next_pq;
                next_pq = PriorityQueue();
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
    }
};

int main() {
    SpecialGraph g;
    g.modified_bfs(1);
    g.print_answer();

    return 0;
}
