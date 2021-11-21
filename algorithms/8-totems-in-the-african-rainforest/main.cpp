#include <iostream>
#include <vector>

using namespace std;

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
};

int main() {
    TotemVillages tvs;
    tvs.initialize_from_stdin();

    return 0;
}
