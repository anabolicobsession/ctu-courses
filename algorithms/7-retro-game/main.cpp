#include <iostream>
#include <vector>

using namespace std;

class RetroGame {
private:
    vector<vector<int>> eggs;
    int playing_area_width, n_moves, n_eggs;

public:
    RetroGame() : playing_area_width(0), n_moves(0), n_eggs(0) {}

    void initialize_from_stdin() {
        cin >> playing_area_width >> n_eggs;
        // one-based indexing for computation simplicity
        auto tmp_eggs = vector<vector<int>>(1 + playing_area_width, vector<int>());

        for (int i = 0; i < n_eggs; ++i) {
            int egg_position, egg_move;
            cin >> egg_position >> egg_move;
            tmp_eggs[egg_position].push_back(egg_move);

            if (egg_move > n_moves) {
                n_moves = egg_move;
            }
        }

        // swap rows (egg positions) and columns (egg moves)
        eggs = vector<vector<int>>(1 + n_moves, vector<int>());

        for (int egg_pos = 1; egg_pos <= playing_area_width; ++egg_pos) {
            for (const auto &egg_move: tmp_eggs[egg_pos]) {
                // if basket can reach position
                if (egg_move >= egg_pos - 2) {
                    eggs[egg_move].push_back(egg_pos);
                }
            }
        }
    }

    int find_best_score() {
        const int UNDEFINED = -1;
        // manual allocation is more effective than vectors
        int *best_score = new int[1 + playing_area_width], *new_best_score = new int[1 + playing_area_width], *n_eggs_at_pos = new int[1 + playing_area_width];
        for (int i = 0; i <= playing_area_width; ++i) {
            best_score[i] = new_best_score[i] = UNDEFINED;
        }
        best_score[1] = 0;

        for (int move = 1; move <= n_moves; ++move) {
            for (int i = 0; i <= playing_area_width; ++i) {
                n_eggs_at_pos[i] = 0;
            }

            for (const auto &egg_pos: eggs[move]) {
                n_eggs_at_pos[egg_pos - 1]++;
                n_eggs_at_pos[egg_pos]++;
            }

            for (int pos = 1; pos < playing_area_width; ++pos) {
                new_best_score[pos] = n_eggs_at_pos[pos] + max(max(best_score[pos - 1], best_score[pos]), best_score[pos + 1]);
            }

            swap(best_score, new_best_score);
        }

        int mx = -1;
        for (int i = 1; i < playing_area_width; ++i) {
            if (best_score[i] > mx) mx = best_score[i];
        }
        delete[] best_score;
        delete[] new_best_score;
        delete[] n_eggs_at_pos;

        return mx;
    }
};

int main() {
    RetroGame rg;
    rg.initialize_from_stdin();
    cout << rg.find_best_score() << '\n';

    return 0;
}
