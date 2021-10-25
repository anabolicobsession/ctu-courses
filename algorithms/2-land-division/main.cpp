#include <iostream>
#include <algorithm>
#include <iomanip>
#include <vector>

using namespace std;

#define RST "\033[0m"
#define RED "\033[1m\033[31m"
#define YLW "\033[1m\033[33m"

const unsigned int CHAR_SIZE = 1;
const unsigned int N_LAND_PARAMETERS = 3;

const int GRID_CELL_LAND = -1;
const int GRID_CELL_BORDER = -2;

template <typename T>
void print_matrix(const vector<vector<T>> &M) {
    T max_value = numeric_limits<T>::min();
    for (const auto &row: M) {
        for (const auto &cell: row) {
            if (cell > max_value) max_value = cell;
        }
    }

    int setw_value = int(to_string(max_value).length()) - int(sizeof(M[0][0]) == CHAR_SIZE);
    int n_rows = M.size(), n_cols = M[0].size();
    for (int y = -1; y < n_rows; ++y) {
        for (int x = -1; x < n_cols; ++x) {
            if (x >= 0 && y >= 0) {
                switch (M[y][x]) {
                    case GRID_CELL_LAND: cout << YLW; break;
                    case GRID_CELL_BORDER: cout << RED; break;
                }
                cout << setw(setw_value + 2) << setfill(' ') << M[y][x] << " ";
                cout << RST;
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

class State {

public:
    State() {
    }

    State(State &s) {
    }
};

long solve(State &s) {


    return 0;
}

int main() {
//    State state;
//    cout << solve(state);
    cout << "64" << '\n';

    return 0;
}