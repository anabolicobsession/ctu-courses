#include <iostream>
#include <iomanip>
#include <limits>
#include <vector>

using namespace std;

void print_matrix(vector<vector<int>> M) {
    int mx = numeric_limits<int>::min();
    for (const auto &row: M) {
        for (const auto &e: row) {
            mx = max(e, mx);
        }
    }

    int setw_val = max(3, int(to_string(mx).length()));
    for (int i = -1; i < int(M.size()); ++i) {
        for (int j = -1; j < int(M[0].size()); ++j) {
            if (i == -1 && j == -1) {
                cout << setw(setw_val) << setfill(' ') << ' ' << ' ';
            } else if (i == -1) {
                cout << "[" << setw(setw_val - 2) << setfill(' ') << j << "] ";
            } else if (j == -1) {
                cout << "[" << setw(setw_val - 2) << setfill(' ') << i << "] ";
            } else {
                if (M[i][j] != numeric_limits<int>::min() && M[i][j] != numeric_limits<int>::max()) {
                    cout << setw(setw_val) << setfill(' ') << M[i][j] << ' ';
                } else {
                    cout << setw(setw_val) << setfill(' ') << ' ' << ' ';
                }
            }
        }
        cout << '\n';
    }
}

class TotemVillages {
private:
    vector<int> fighters;
    int n_villages;
    int totem_price;
    int fighter_price;

public:
    TotemVillages() : n_villages(0), totem_price(0), fighter_price(0) {}

    void initialize_from_stdin() {
        cin >> n_villages >> totem_price >> fighter_price;
        fighters.resize(n_villages);
        for (int i = 0; i < n_villages; ++i) {
            cin >> fighters[i];
        }
    }

    int compute_max_profit_from_purchase_of_totems() const {
        vector<vector<int>> profit_matrix(n_villages, vector<int>(n_villages, numeric_limits<int>::min()));
        vector<vector<int>> fighters_sum(n_villages, vector<int>(n_villages, 0));

        for (int i = 0; i < n_villages; ++i) {
            profit_matrix[i][i] = 0;
            fighters_sum[i][i] = fighters[i];
        }
        auto max_profit_matrix = profit_matrix;

        for (int len = 2; len <= n_villages; ++len) {
            for (int start = 0, end = start + len - 1; end < n_villages; ++start, ++end) {
                fighters_sum[start][end] = fighters_sum[start][end - 1] + fighters_sum[end][end];

                for (int totem = start; totem < end; ++totem) {
                    int diff_in_fighters = abs(fighters_sum[start][totem] - fighters_sum[totem + 1][end]);
                    int profit = totem_price - fighter_price * diff_in_fighters + profit_matrix[start][totem] + profit_matrix[totem + 1][end];
                    profit_matrix[start][end] = max(profit, profit_matrix[start][end]);

                    int left_max_profit = max_profit_matrix[start][totem];
                    int right_max_profit = max_profit_matrix[totem + 1][end];
                    int max_profit = max(left_max_profit + right_max_profit, max(left_max_profit, right_max_profit));
                    max_profit_matrix[start][end] = max(max_profit, max(profit_matrix[start][end], max_profit_matrix[start][end]));
                }
            }
        }

        return max_profit_matrix[0][n_villages - 1];
    }
};

int main() {
    TotemVillages tvs;
    tvs.initialize_from_stdin();
    cout << tvs.compute_max_profit_from_purchase_of_totems() << '\n';

    return 0;
}
