#include <iostream>
#include <algorithm>
#include <vector>
#include <queue>

using namespace std;

class RetroGame {
private:
    vector<queue<int>> eggs;
    int playing_area_width, max_egg_height, n_eggs;

public:
    RetroGame() : playing_area_width(0), max_egg_height(0), n_eggs(0) {}

    void initialize_from_stdin() {
        cin >> playing_area_width >> n_eggs;
        // one-based indexing for computation simplicity
        eggs = vector<queue<int>>(1 + playing_area_width, queue<int>());

        auto tmp_eggs = vector<vector<int>>(1 + playing_area_width, vector<int>());
        for (int i = 0; i < n_eggs; ++i) {
            int egg_column, egg_height;
            cin >> egg_column >> egg_height;
            tmp_eggs[egg_column].push_back(egg_height);

            if (egg_height > max_egg_height) {
                max_egg_height = egg_height;
            }
        }

        // construct queues from sorted arrays
        for (int i = 1; i <= playing_area_width; ++i) {
            sort(tmp_eggs[i].begin(), tmp_eggs[i].end());
            for (const auto &egg: tmp_eggs[i]) {
                eggs[i].push(egg);
            }
        }
    }

    int find_best_score() {
        const int UNDEFINED = -1;
        const int N_BASKET_POSITIONS = playing_area_width - 1;
        vector<int> best_score(1 + playing_area_width, UNDEFINED), new_best_score(1 + playing_area_width, UNDEFINED);
        best_score[1] = 0;

        for (int height = 1; height <= max_egg_height; ++height) {
            vector<int> n_eggs_at_position(1 + playing_area_width);
            for (int i = 1; i <= playing_area_width; ++i) {
                n_eggs_at_position[i] = !eggs[i].empty() && height == eggs[i].front();
                if (n_eggs_at_position[i]) {
                    eggs[i].pop();
                }
            }

            for (int i = 1; i < playing_area_width; ++i) {
                n_eggs_at_position[i] += n_eggs_at_position[i + 1];
            }

            for (int pos = 1; pos <= N_BASKET_POSITIONS; ++pos) {
                new_best_score[pos] = max({best_score[pos - 1], best_score[pos], best_score[pos + 1]});
                if (new_best_score[pos] != UNDEFINED) {
                    new_best_score[pos] += n_eggs_at_position[pos];
                }
            }

            best_score.swap(new_best_score);
        }

        return *max_element(best_score.begin(), best_score.end());
    }
};

int main() {
    RetroGame rg;
    rg.initialize_from_stdin();
    cout << rg.find_best_score() << '\n';

    return 0;
}
