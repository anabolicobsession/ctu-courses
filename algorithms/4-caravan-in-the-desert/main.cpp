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

const int INT_INF = numeric_limits<int>::max() / 2;

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
            for (const auto &v: values[i]) cout << ' ' << setw(setw_val) << setfill(' ') << (v < INT_INF ? to_string(v) : "inf");
            cout << '\n';
        }
    }

    int dist_nearest_special_node_in_component(node_t node, vector<bool> visited, node_t &s_node) {
        int dist = 0;
        s_node = node;
        if (is_special(node)) return dist;
        dist++;

        queue<node_t> q, next_q;
        q.push(node);

        while (!q.empty()) {
            if (dist > values[node][MOVES_LEFT]) break;
            node_t n = q.front();
            q.pop();

            for (const auto &next: adj_list[n]) {
                if (!visited[next]) {
                    visited[next] = true;

                    if (is_special(next)) {
                        s_node = next;
                        return dist;
                    }

                    next_q.push(next);
                }
            }

            if (q.empty()) {
                dist++;
                q = next_q;
                next_q = queue<node_t>();
            }
        }

        return INT_INF;
    }

    vector<vector<node_t>> get_component_special_nodes() {
        vector<vector<node_t>> csn;
        vector<bool> component_node_visited(1 + n_special_nodes, false);

        if (n_special_nodes == 0) csn.push_back({1});

        for (int adj_list_node = 1; adj_list_node <= n_special_nodes; ++adj_list_node) {
            if (component_node_visited[adj_list_node]) continue;
            csn.push_back({adj_list_node});

            queue<node_t> component_q;
            component_q.push(adj_list_node);

            while (!component_q.empty()) {
                node_t component_node = component_q.front();
                component_q.pop();

                component_node_visited[component_node] = true;

                queue<node_t> q, next_q;
                q.push(component_node);

                vector<bool> visited(1 + n_nodes);
                visited[component_node] = true;

                int moves_left = n_moves_per_component - 1;

                while (!q.empty()) {
                    node_t n = q.front();
                    q.pop();

                    for (const auto &next: adj_list[n]) {
                        if (!visited[next]) {
                            visited[next] = true;

                            if (is_special(next) && !component_node_visited[next]) {
                                component_node_visited[next] = true;
                                component_q.push(next);
                                csn[csn.size() - 1].push_back(next);
                            }

                            next_q.push(next);
                        }
                    }

                    if (q.empty()) {
                        if (moves_left == 0) break; else moves_left--;
                        q = next_q;
                        next_q = queue<node_t>();
                    }
                }
            }
        }

        return csn;
    }

    void print_queue(queue<node_t> q) {
        bool was_empty = q.empty();
        while (!q.empty()) {
            cout << q.front() << ' ';
            q.pop();
        }
        if (!was_empty) cout << '\n';
    }

    void modified_bfs(Graph::node_t n) {
        vector<bool> visited(1 + n_nodes, false);
        visited[n] = true;

        queue<node_t> q, next_q, end_q;
        q.push(n);

        values[n][SpecialGraph::MOVES_LEFT] = n_moves_per_component;
        values[n][SpecialGraph::COMPONENT] = 0;

        vector<vector<node_t>> csn = get_component_special_nodes();
        bool first_component = true;
        bool is_end = false;
        int comp = 0;

        int DEBUG = 0;

        while (true) {
            if ((q.empty() || first_component) && !is_end) {
                if (next_q.empty() && !first_component) {
                    is_end = true;

                    while(!end_q.empty()) {
                        q.push(end_q.front());
                        end_q.pop();
                    }

                    goto leave_condition;
                }

                n = !first_component ? next_q.front() : q.front();
                first_component = false;
                node_t special_node;

                if (dist_nearest_special_node_in_component(n, visited, special_node) != INT_INF) {
                    int csn_idx = -1;
                    for (int i = 0; i < csn.size(); ++i) {
                        for (const auto &comp_node: csn[i]) {
                            if (comp_node == special_node) {
                                csn_idx = i;
                                goto loop_end;
                            }
                        }
                    }
                    loop_end:;

                    q = queue<node_t>();
                    for (const auto &comp_node: csn[csn_idx]) {
                        q.push(comp_node);
                        values[comp_node][MOVES_LEFT] = n_moves_per_component;
                        values[comp_node][COMPONENT] = comp;
                        visited[comp_node] = true;
                    }

                    while (!next_q.empty()) {
                        end_q.push(next_q.front());
                        next_q.pop();
                    }
                } else {
                    q = next_q;
                }
                next_q = queue<node_t>();

                if (DEBUG && !q.empty()) cout << "-------------------- New depth --------------------" << '\n';
            }

            leave_condition:

            if (DEBUG) print_queue(q);
            n = q.front();
            q.pop();

            for (const auto &next: adj_list[n]) {
                explore_node(next, n);

                if (!visited[next]) {
                    visited[next] = true;

                    if (in_same_component(next, n) || is_end) {
                        q.push(next);
                    } else {
                        next_q.push(next);
                    }

//                    if (DEBUG) cout << "+(" << e[PriorityQueue::VALUE] << "," << e[PriorityQueue::PRIORITY] << ")" << '\n';
                }
            }

            if (q.empty()) {
                if (next_q.empty() && end_q.empty()) {
                    break;
                }
                comp++;
            }
        }

    }

    void modified_bfs_xxx(Graph::node_t n) {
        vector<bool> visited(1 + n_nodes, false);
        visited[n] = true;

        PriorityQueue pq, next_pq;
        pq.push(PriorityQueue::elem_t{n, 0});

        values[n][SpecialGraph::MOVES_LEFT] = n_moves_per_component;
        values[n][SpecialGraph::COMPONENT] = 0;

        int DEBUG = 0;

        bool searching_special_nodes = true;

        while(!pq.empty()) {
            if (DEBUG) {
                pq.print();
//                print_values();
//                cout << '\n';
            }

            bool best_node_found = false;
            bool special_node_in_component_found;

//            while (!best_node_found) {
//                PriorityQueue::elem_t pq_elem = pq.pop();
//                n = pq_elem[PriorityQueue::VALUE];
////                int real_priority = compute_node_priority(n, visited, special_node_in_component_found);
//
//                if (pq_elem[PriorityQueue::PRIORITY] == real_priority) {
//                    best_node_found = true;
//                } else {
//                    pq.push({pq_elem[PriorityQueue::VALUE], real_priority});
//                }
//            }
//
//            for (const auto &next: adj_list[n]) {
//                explore_node(next, n);
//
//                if (!visited[next]) {
//                    visited[next] = true;
//
//                    int priority = compute_node_priority(next, visited, special_node_in_component_found);
//                    PriorityQueue::elem_t e{next, priority};
//
//                    if (in_same_component(next, n)) {
//                        if (special_node_in_component_found && priority != 0) {
//                            visited[next] = false;
//                        }
//                        pq.push(e);
//                    } else {
//                        next_pq.push(e);
//                    }
//
////                    if (DEBUG) cout << "+(" << e[PriorityQueue::VALUE] << "," << e[PriorityQueue::PRIORITY] << ")" << '\n';
//                }
//            }

            if (pq.empty()) {
                if (DEBUG && !next_pq.empty()) cout << "-------------------- New depth --------------------" << '\n';

                pq = next_pq;
                next_pq = PriorityQueue();
                searching_special_nodes = true;
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
