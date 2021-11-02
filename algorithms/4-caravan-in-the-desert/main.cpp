#include <iostream>
#include <vector>
#include <iomanip>

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

class Caravan {
public:
    vector<vector<int>> adj_list;
    int n_villages, n_routes, n_friendly_villages, saturation;

    Caravan() : n_villages(0), n_routes(0), n_friendly_villages(0), saturation(0) {
        cin >> n_villages >> n_routes >> n_friendly_villages >> saturation;

        adj_list.resize(n_villages);
        for (int i = 0; i < n_routes; ++i) {
            adj_list.emplace_back();
            int node1 = get_integer_from_stdin() - 1; // indexing from zero
            int node2 = get_integer_from_stdin() - 1;
            adj_list[node1].push_back(node2);
            adj_list[node2].push_back(node1);
        }
    }

    void print_adj_list() {
        int setw_val = int(to_string(n_villages - 1).length());
        for (int i = 0; i < n_villages; ++i) {
            cout << '[' << setw(setw_val) << setfill(' ') << i + 1 << ']'; // indexing from one
            for (int node: adj_list[i]) cout << ' ' << setw(setw_val) << setfill(' ') << node + 1;
            cout << '\n';
        }
    }
};

int main() {

    return 0;
}
