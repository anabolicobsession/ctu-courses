#include <iostream>
#include <algorithm>
#include <iomanip>
#include <vector>

using namespace std;

const int LAND_SOLD = -2;
const int OUT_OF_BORDER = -3;
const int LAND_BORDER = -1;
const int LAND_HEIGHT = 0;
const int LAND_WIDTH = 1;
const int AMOUNT_OF_LAND = 2;

/*
 * Notes:
 * Use default unsigned char array instead of vector
 * Add enum
 */

template <typename T>
void print_matrix(const vector<vector<T>> &M) {
    T max_value = numeric_limits<T>::min();
    for (const auto &row: M) {
        T new_value = *max_element(row.begin(), row.end());
        if (new_value > max_value) max_value = new_value;
    }

    int setw_value = int(to_string(max_value).length());
    for (int y = -1; y < (int)M.size(); ++y) {
        for (int x = -1; x < (int)M[0].size(); ++x) {
            if (x >= 0 && y >= 0) {
                cout << setw(1 + setw_value + 1) << setfill(' ') << M[y][x] << " ";
            }
            else if (x == -1 && y != -1) {
                cout << "[" << setw(setw_value) << setfill(' ') << y << "]";
            } else if (x != -1 && y == -1) {
                cout << "[" << setw(setw_value) << setfill(' ') << x << "] ";
            } else {
                cout << setw(setw_value + 3) << setfill(' ') << " ";
            }
        }
        cout << '\n';
    }
}

int get_land_area(const vector<int> &l) {
    return l[LAND_HEIGHT] * l[LAND_WIDTH];
}

bool compare_lands(const vector<int> &a, const vector<int> &b) {
    return get_land_area(a) < get_land_area(b);
}

void pop_land(vector<vector<int>> &lands, int idx) {
    if (lands[idx][AMOUNT_OF_LAND] > 1) {
        lands[idx][AMOUNT_OF_LAND] -= 1;
    } else {
        lands.erase(lands.begin() + idx);
    }
}

int add_land_to_grid(vector<vector<int>> &grid, vector<int> &land, int x, int y) {
    int profit = 0;
    int y_bound = land[LAND_HEIGHT];
    int x_bound = land[LAND_WIDTH];
    for (int i = -1; i <= y_bound; ++i) {
        for (int j = -1; j <= x_bound; ++j) {
            if (x + j < 0 || y + i < 0 || x + j >= grid[0].size()) continue;
            if (y + i >= grid.size()) break;

            int *cell = &grid[y + i][x + j];
            if (i >= 0 && j >= 0 && i < y_bound && j < x_bound) {
                profit += *cell;
                *cell = LAND_SOLD;
            } else {
                *cell = LAND_BORDER;
            }
        }
    }

    return profit;
}

int calc_profit(const vector<vector<int>> &grid, const vector<int> &land, int x, int y) {
    int lh = land[LAND_HEIGHT];
    int lw = land[LAND_WIDTH];

    int profit = -1;
    if (x + lw - 1 < grid[0].size() && y + lh - 1 < grid.size()) {
        profit = 0;
        for (int i = 0; i < lh; ++i) {
            for (int j = 0; j < lw; ++j) {
                profit += grid[y + i][x + j];
            }
        }
    }

    return profit;
}

int best_profit_after_land_sale(vector<vector<int>> grid, vector<vector<int>> lands, int land_id, int sx, int sy, int profit) {
    int max_profit = profit + add_land_to_grid(grid, lands[land_id], sx, sy);
    pop_land(lands, land_id);

//    if (grid[0][0] == -2 && grid[1][0] == -2) {
//            print_matrix(grid);
//            cout << "Now: " << max_profit << ", was popped: " << land_id << '\n';
//            print_matrix(lands);
//    }
    if (max_profit == 11) {
        print_matrix(grid);
        cout << "Now: " << max_profit << '\n';
        print_matrix(lands);
    }

    int best_new_land_profit = -1;
    vector<vector<int>> best_coords(1, {0, 0, 0});

    for (int y = int(grid.size() - 1); y >= 0 ; --y) {
        for (int x = int(grid[0].size() - 1); x >= 0; --x) {
            if ((x == 0 && grid[y][x] != LAND_BORDER) ||
                (x > 0 && grid[y][x - 1] == LAND_BORDER && grid[y][x] != LAND_BORDER) ||
                (y == 0 && grid[y][x] != LAND_BORDER) ||
                (y > 0 && grid[y - 1][x] == LAND_BORDER && grid[y][x] != LAND_BORDER) ||
                (x > 0 && y > 0 && grid[y - 1][x - 1] == LAND_BORDER && grid[y][x] != LAND_BORDER)) {
                for (int i = 0; i < lands.size(); i++) {


                    int pr = calc_profit(grid, lands[i], x, y);

                    if (max_profit == 49 && x == 3 && y == 3) {
//                        cout << "Land: " << i << ", pr: " << pr << '\n';
                    }


                    if (pr > best_new_land_profit) {
                        best_new_land_profit = pr;
                        best_coords.clear();
                        best_coords.push_back({x, y, i});
                    } else if (pr == best_new_land_profit) {
                        best_coords.push_back({x, y, i});
                    }

                }
            }
        }
    }

    int new_max_profit = max_profit;
    if (best_new_land_profit > -1) {
        for (const auto &coords: best_coords) {
            int tmp = best_profit_after_land_sale(grid, lands, coords[2], coords[0], coords[1], max_profit);
            if (tmp > new_max_profit) new_max_profit = tmp;
        }
        max_profit = new_max_profit;
    }

    return max_profit;
}

int find_best_profit(vector<vector<int>> &grid, vector<vector<int>> &lands) {
    sort(lands.begin(), lands.end(), compare_lands);

    int max_profit = 0;
    for (int i = 0; i < lands.size() && get_land_area(lands[i]) == get_land_area(lands[0]); ++i) {
        int profit = best_profit_after_land_sale(grid, lands, i, 0, 0, 0);
        if (profit > max_profit) max_profit = profit;
    }

    return max_profit;
}

int main() {
    int M, N;
    cin >> M >> N;
    vector<vector<int>> grid(M, vector<int>(N, 0));
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            cin >> grid[i][j];
        }
    }

//    print_matrix(grid);

    int T;
    cin >> T;
    const unsigned int N_LAND_PARAMETERS = 3;
    vector<vector<int>> lands(T, vector<int> (N_LAND_PARAMETERS, 0));
    for (int i = 0; i < T; ++i) {
        for (int j = 0; j < N_LAND_PARAMETERS; ++j) {
            cin >> lands[i][j];
        }
    }

    cout << find_best_profit(grid, lands) << "\n";

    return 0;
}
