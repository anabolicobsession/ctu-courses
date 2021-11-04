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

        if (!was_empty) cout << '\n';
    }

    void set_values(node_t n, int moves_left, int component) {
        values[n][MOVES_LEFT] = moves_left;
        values[n][COMPONENT] = component;
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

                int moves_left = moves_per_component - 1;

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

    vector<node_t> find_special_nodes_in_component(node_t n, const vector<bool> &visited_init) {
        vector<node_t> snc;
        if (is_special(n)) snc.push_back(n);
        if (values[n][MOVES_LEFT] == 0) return snc;

        queue<node_t> cq;
        cq.push(n);

        vector<bool> cq_visited(n_nodes, false);
        cq_visited[n] = true;

        while (!cq.empty()) {
            n = cq.front();
            cq.pop();

            queue<node_t> q, next_q;
            q.push(n);

            vector<bool> visited = visited_init;
            int moves_left = is_special(n) ? moves_per_component - 1 : values[n][MOVES_LEFT] - 1;

            while (!q.empty()) {
                n = q.front();
                q.pop();

                for (const auto &next: adj_list[n]) {
                    if (!visited[next]) {
                        visited[next] = true;

                        if (is_special(next) && !cq_visited[next]) {
                            cq_visited[next] = true;
                            snc.push_back(next);
                            cq.push(next);
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

        return snc;
    }

    queue<node_t> process_queue(queue<node_t> q, vector<bool> &visited) {
        queue<node_t> new_q;
        vector<bool> processed(n_nodes, false);

        while (!q.empty()) {
            node_t n = q.front();
            q.pop();

            vector<node_t> snc = find_special_nodes_in_component(n, visited);
            if (!snc.empty()) {
                for (const auto &sn: snc) {
                    if (!processed[sn]) {
                        processed[sn] = true;
                        visited[sn] = true;

                        set_values(sn, moves_per_component, values[n][COMPONENT]);
                        new_q.push(sn);
                    }
                }
            } else {
                new_q.push(n);
            }
        }

        return new_q;
    }

    void modified_bfs(Graph::node_t n) {
        queue<node_t> q, next_q;
        q.push(n);

        vector<bool> visited(1 + n_nodes, false);
        visited[n] = true;

        set_values(n, moves_per_component, 0);
        q = process_queue(q, visited);

        int DEBUG = 0;

        while (!q.empty()) {
            if (DEBUG) print_queue(q);

            n = q.front();
            q.pop();

            for (const auto &next: adj_list[n]) {
                explore_node(next, n);

                if (!visited[next]) {
                    visited[next] = true;
                    if (in_same_component(next, n)) q.push(next); else next_q.push(next);
                }
            }

            if (q.empty()) {
                q = process_queue(next_q, visited);
                next_q = queue<node_t>();

                if (DEBUG) {
                    if (!q.empty()) {
                        cout << "---------- New depth --------------------" << '\n';
                    } else {
                        cout << '\n';
                    }
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
