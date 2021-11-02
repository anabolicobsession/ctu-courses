#include <iostream>
#include <vector>
#include <iomanip>
#include <queue>
#include <limits>

using namespace std;

const int INT_MAX_LEN = 10;

/**
 * Read one non-negative integer from standard input, ignoring any non-digit characters.
 * Useful for fast reading.
 *
 * @return integer if one was in input, EOF otherwise.
 */
int get_integer_from_stdin() {
    unsigned char str[INT_MAX_LEN];
    int c, i = 0;

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

class Desert {
private:
    typedef int village_t;
    typedef vector<village_t> villages_t;
    typedef array<int, 2> params_t;
    const int IDX_SATURATION = 0, IDX_N_PACKAGES = 1;

    vector<villages_t> adj_list;
    int n_villages, n_routes, n_friendly_villages, saturation;

    bool is_friendly(const int &v) const {
        return v < n_friendly_villages;
    }

    static int get_printable_village(const int &v) {
        return v + 1;
    }

    array<int, 2> compute_params_for_new_village(params_t params, const village_t &a, const village_t &b) const {
        if (is_friendly(a)) {
            if (!is_friendly(b)) params[IDX_SATURATION]--;
        } else {
            params[IDX_SATURATION]--;
            if (params[IDX_SATURATION] == -1) {
                params[IDX_SATURATION] = saturation;
                params[IDX_N_PACKAGES]++;
            }
            if (is_friendly(b)) params[IDX_SATURATION] = saturation;
        }

        return params;
    }

    bool params_better(const params_t &a, const params_t &b) const {
        return a[IDX_N_PACKAGES] < b[IDX_N_PACKAGES] || a[IDX_N_PACKAGES] == b[IDX_N_PACKAGES] && a[IDX_SATURATION] > b[IDX_SATURATION];
    }

    void run_modified_dfs(village_t v, vector<bool> &visited, vector<array<int, 2>> &best_params) const {
        visited[v] = true;

        for (const auto &nv: adj_list[v]) {
            params_t params = compute_params_for_new_village(best_params[v], v, nv);
            if (params_better(params, best_params[nv])) best_params[nv] = params;

            if (!visited[nv] && best_params[nv][IDX_N_PACKAGES] == best_params[v][IDX_N_PACKAGES]) {
                run_modified_dfs(nv, visited, best_params);
            }
        }
    }

public:
    Desert() : n_villages(0), n_routes(0), n_friendly_villages(0), saturation(0) {
        cin >> n_villages >> n_routes >> n_friendly_villages >> saturation;

        adj_list.resize(n_villages);
        for (int i = 0; i < n_routes; ++i) {
            adj_list.emplace_back();
            village_t v1 = get_integer_from_stdin() - 1; // indexing from zero
            village_t v2 = get_integer_from_stdin() - 1;
            adj_list[v1].push_back(v2);
            adj_list[v2].push_back(v1);
        }
    }

    void print_adj_list() const {
        int setw_val = int(to_string(n_villages - 1).length());
        for (int village = 0; village < n_villages; ++village) {
            cout << '[' << setw(setw_val) << setfill(' ') << get_printable_village(village) << ']'; // indexing from one
            for (int neighboring_village: adj_list[village]) cout << ' ' << setw(setw_val) << setfill(' ') << get_printable_village(neighboring_village);
            cout << '\n';
        }
    }

    void run_modified_bfs(village_t v= 0) const {
        queue<village_t> depth_queue, next_depth_queue;
        depth_queue.push(v);

        vector<bool> visited(n_villages, false);
        visited[v] = true;

        vector<array<int, 2>> best_params(n_villages, {numeric_limits<int>::min(), numeric_limits<int>::max()});
        best_params[v] = {saturation, 0};

        while (!depth_queue.empty()) {
            v = depth_queue.front();
            depth_queue.pop();

            vector<bool> visited_copy = visited;
            run_modified_dfs(v, visited_copy, best_params);

            for (const auto &nv: adj_list[v]) {
                array<int, 2> params = compute_params_for_new_village(best_params[v], v, nv);
                if (params_better(params, best_params[nv])) {
                    best_params[nv] = params;
                }

                if (!visited[nv]) {
                    visited[nv] = true;
                    if (best_params[v][IDX_SATURATION] != 0 && is_friendly(nv)) {
                        depth_queue.push(nv);
                    } else {

                        if (best_params[v][IDX_SATURATION] != 0 && is_friendly(nv)) {
                            depth_queue.push(nv);
                        } else {
                            next_depth_queue.push(nv);
                        }
                    }
                }
            }

            if (depth_queue.empty()) {
                depth_queue = next_depth_queue;
                next_depth_queue = queue<village_t>();
            }
        }

        int max_n_packages = 0;
        int zero_packages = 0;
        for (int i = 0; i < n_villages; ++i) {
            if (best_params[i][IDX_N_PACKAGES] > max_n_packages) max_n_packages = best_params[i][IDX_N_PACKAGES];
            if (best_params[i][IDX_N_PACKAGES] == 0) zero_packages++;

        }
        cout << max_n_packages << ' ' << zero_packages << '\n';
    }
};



int main() {
    Desert ds;
    ds.run_modified_bfs();

    return 0;
}
