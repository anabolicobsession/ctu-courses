#include <iostream>
#include <algorithm>
#include <iomanip>
#include <vector>

using namespace std;

#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[1m\033[31m"
#define COLOR_GREEN "\033[1m\033[32m"

const unsigned int CHAR_SIZE = 1;
const int GRID_CELL_LAND = -1;
const int GRID_CELL_BORDER = -2;
const unsigned int N_LAND_PARAMETERS = 3;
const unsigned int LAND_HEIGHT_IDX = 0;
const unsigned int LAND_WIDTH_IDX = 1;
const unsigned int LAND_AMOUNT_IDX = 2;

typedef vector<int> row_t;
typedef vector<row_t> matrix_t;

template <typename T>
void print_matrix(const vector<vector<T>> &M) {
    T max_value = numeric_limits<T>::min();
    for (const auto &row: M) {
        for (const auto &cell: row) {
            if (abs(cell) > max_value) max_value = abs(cell);
        }
    }

    int setw_value = int(to_string(max_value).length()) - int(sizeof(M[0][0]) == CHAR_SIZE);
    int n_rows = M.size(), n_cols = M[0].size();
    for (int y = -1; y < n_rows; ++y) {
        for (int x = -1; x < n_cols; ++x) {
            if (x >= 0 && y >= 0) {
                switch (M[y][x]) {
                    case GRID_CELL_LAND: cout << COLOR_GREEN; break;
                    case GRID_CELL_BORDER: cout << COLOR_RED; break;
                }
                cout << setw(setw_value + 2) << setfill(' ') << M[y][x] << " ";
                cout << COLOR_RESET;
            }
            else if (x != -1 && y == -1) {
                cout << "[" << setw(setw_value) << setfill(' ') << x << "] ";
            }
            else if (x == -1 && y != -1) {
                cout << "[" << setw(setw_value) << setfill(' ') << y << "]";
            }
            else {
                cout << setw(setw_value + 3) << setfill(' ') << " ";
            }
        }
        cout << '\n';
    }
}

bool compare_lands(const row_t &l1, const row_t &l2) {
    return l1[LAND_HEIGHT_IDX] * l1[LAND_WIDTH_IDX] < l2[LAND_HEIGHT_IDX] * l2[LAND_WIDTH_IDX];
}

class State {
private:
    unsigned long profit;
    vector<vector<int>> lands;

public:
    vector<vector<int>> grid;
    int grid_n_rows, grid_n_cols;
    int lands_n_rows, lands_n_cols;

    State(const matrix_t &grid, const matrix_t &lands) : profit(0) {
        this->grid = grid;
        grid_n_rows = int(grid.size());
        grid_n_cols = int(grid[0].size());
        this->lands = lands;
        lands_n_rows = int(lands.size());
        lands_n_cols = int(lands[0].size());
        sort(this->lands.begin(), this->lands.end(), compare_lands);
    }

    unsigned long get_profit() const {
        return profit;
    }

    void pop_land(int land_idx) {
        if (lands[land_idx][LAND_AMOUNT_IDX] > 1) {
            lands[land_idx][LAND_AMOUNT_IDX]--;
        } else {
            lands.erase(lands.begin() + land_idx);
            lands_n_rows--;
        }
    }

    /**
     * Try to add a land to the grid.
     *
     * @return true, if the land was added, false otherwise
     */
    bool try_to_add_land(int land_idx, int x1, int y1) {
        int land_width = lands[land_idx][LAND_WIDTH_IDX];
        int land_height = lands[land_idx][LAND_HEIGHT_IDX];
        if (x1 + land_width - 1 >= grid_n_cols || y1 + land_height - 1 >= grid_n_rows) return false;

        for (int i = 0; i < land_height; ++i) {
            for (int j = 0; j < land_width; ++j) {
                if (grid[y1 + i][x1 + j] == GRID_CELL_LAND || grid[y1 + i][x1 + j] == GRID_CELL_BORDER) return false;
            }
        }

        for (int i = -1; i <= land_height; ++i) {
            for (int j = -1; j <= land_width; ++j) {
                int x = x1 + j, y = y1 + i;

                if (j != -1 && j != land_width && i != -1 && i != land_height) {
                    profit += grid[y][x];
                    grid[y][x] = GRID_CELL_LAND;
                } else if (x >= 0 && x < grid_n_cols && y >= 0 && y < grid_n_rows) {
                    grid[y][x] = GRID_CELL_BORDER;
                }
            }
        }
        pop_land(land_idx);

        return true;
    }

    void print() {
        cout << "Current profit: " << profit << '\n';
        print_matrix(grid);
        print_matrix(lands);
        cout << '\n';
    }
};

State solve(State &st) {
    State best_st = st;
    for (int y = st.grid_n_rows - 1; y >= 0; --y) {
        for (int x = st.grid_n_cols - 1; x >= 0; --x) {
            if (st.grid[y][x] == GRID_CELL_BORDER) break;

            if ((x == 0 || st.grid[y][x - 1] == GRID_CELL_BORDER) &&
                (y == 0 || st.grid[y - 1][x] == GRID_CELL_BORDER)) {

                for (int l = 0; l < st.lands_n_rows; ++l) {
                    State new_st = st;
                    if (new_st.try_to_add_land(l, x, y)) {
                        State solution_st = solve(new_st);
                        if (solution_st.get_profit() > best_st.get_profit()) best_st = solution_st;
                    }
                }
            }
        }
    }

    return best_st;
}

int main() {
    int M, N;
    cin >> M >> N;
    matrix_t grid(M, row_t (N, 0));
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            cin >> grid[i][j];
        }
    }

    int T;
    cin >> T;
    matrix_t lands(T, row_t (N_LAND_PARAMETERS, 0));
    for (int i = 0; i < T; ++i) {
        for (int j = 0; j < N_LAND_PARAMETERS; ++j) {
            cin >> lands[i][j];
        }
    }

    State start_state = State(grid, lands);
    State final_state = solve(start_state);
    cout << final_state.get_profit() << '\n';

    return 0;
}
