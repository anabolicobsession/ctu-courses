#include <iostream>
#include <vector>
#include <thread>
#include <condition_variable>
#include <regex>
 
using namespace std;
 
mutex mutex_stdout;
mutex mutex_factory;
condition_variable cv_factory;
condition_variable cv_workers_activity;
 
const int NOT_FOUND = -1;
const int PHASE_COUNT = 6;
 
enum place { NUZKY, VRTACKA, OHYBACKA, SVARECKA, LAKOVNA, SROUBOVAK, FREZA, PLACE_COUNT };
 
const vector<string> place_str = { "nuzky", "vrtacka", "ohybacka", "svarecka", "lakovna", "sroubovak", "freza", };
 
const vector<int> place_time_ms = {100, 200, 150, 300, 400, 250, 500 };
 
enum product { A, B, C, PRODUCT_COUNT };
 
const vector<string> product_str = { "A", "B", "C" };
 
const vector<vector<int>> phase_places = {
        {NUZKY, VRTACKA, OHYBACKA, SVARECKA, VRTACKA, LAKOVNA},
        {VRTACKA, NUZKY, FREZA, VRTACKA, LAKOVNA, SROUBOVAK},
        {FREZA, VRTACKA, SROUBOVAK, VRTACKA, FREZA, LAKOVNA}
};
 
int find_string(const vector<string> &strs, const string &str) {
    for (int i = 0; i < int(strs.size()); ++i) if (strs[i] == str) return i;
    return NOT_FOUND;
}
 
class Worker {
public:
    string name;
    int spec;
    bool fired;
 
    Worker(string name, int spec) : name(move(name)), spec(spec), fired(false) {}
};
 
class Workers {
private:
    vector<Worker*> ws;
 
    int name_to_idx(const string &name) const {
        for (int i = 0; i < int(ws.size()); ++i) if (ws[i]->name == name) return i;
        return NOT_FOUND;
    }
 
public:
    virtual ~Workers() {
        for (const auto &w: ws) delete w;
        ws.clear();
    }
 
    void add(const string &name, int spec) {
        lock_guard<mutex> lock(mutex_factory);
        ws.emplace_back(new Worker(name, spec));
    }
 
    void fire(const string &name) {
        lock_guard<mutex> lock(mutex_factory);
        ws[name_to_idx(name)]->fired = true;
    }
 
    void fire_all() {
        lock_guard<mutex> lock(mutex_factory);
        for (const auto &w: ws) w->fired = true;
    }
 
    void remove(const string &name) {
        lock_guard<mutex> lock(mutex_factory);
        int idx = name_to_idx(name);
        delete ws[idx];
        ws.erase(ws.begin() + idx);
    }
 
    bool has_worker(const string &name) const {
        lock_guard<mutex> lock(mutex_factory);
        return name_to_idx(name) != NOT_FOUND;
    }
 
    Worker* get_worker(const string &name) {
        lock_guard<mutex> lock(mutex_factory);
        return ws[name_to_idx(name)];
    }
 
    vector<bool> get_list_of_places() {
        vector<bool> list(PLACE_COUNT, false);
        for (const auto &w: ws) if (!list[w->spec]) list[w->spec] = true;
        return list;
    }
};
 
class FactoryResources {
private:
    vector<int> ready_places;
    int place_count;
    vector<vector<int>> parts;
 
public:
    FactoryResources() : place_count(0) {
        ready_places = vector<int>(PLACE_COUNT, 0);
        parts = vector<vector<int>>(PRODUCT_COUNT, vector<int>(PHASE_COUNT, 0));
    }
 
    void add_place(int place) {
        lock_guard<mutex> lock(mutex_factory);
        ready_places[place]++;
        place_count++;
    }
 
    void remove_place(int place) {
        lock_guard<mutex> lock(mutex_factory);
        ready_places[place]--;
        place_count--;
    }
 
    void return_place(int place) {
        lock_guard<mutex> lock(mutex_factory);
        ready_places[place]++;
    }
 
    void add_part(int product, int phase) {
        lock_guard<mutex> lock(mutex_factory);
        parts[product][phase]++;
    }
 
    bool try_to_find_work(int spec, int &product, int &phase) {
        lock_guard<mutex> lock(mutex_factory);
        product = NOT_FOUND, phase = NOT_FOUND;
        if (ready_places[spec] <= 0) return false;


        for (int j = PHASE_COUNT - 1; j >= 0; --j) {
            for (int i = 0; i < PRODUCT_COUNT; ++i) {
                if (phase_places[i][j] == spec && parts[i][j] > 0) {
                    parts[i][j]--;
                    ready_places[spec]--;
                    product = i;
                    phase = j;
                    return true;
                }
            }
        }
 
        return false;
    }
 
    bool someone_can_work(Workers &ws) {
        lock_guard<mutex> lock(mutex_factory);
 
        int rp_sum = 0;
        for (const auto &rp: ready_places) rp_sum += rp;
        bool some_places_are_occupied = rp_sum != place_count;
 
        vector<bool> place_has_worker = ws.get_list_of_places();
        for (int i = 0; i < PLACE_COUNT; ++i) if (ready_places[i] <= 0) place_has_worker[i] = false;
        bool there_are_parts_that_can_be_processed = false;
 
        for (int i = 0; i < PRODUCT_COUNT; ++i) {
            for (int j = 0; j < PHASE_COUNT; ++j) {
                if (parts[i][j] > 0 && place_has_worker[phase_places[i][j]]) {
                    there_are_parts_that_can_be_processed = true;
                    goto func_end;
                }
            }
        }
 
        func_end:
        return some_places_are_occupied || there_are_parts_that_can_be_processed;
    }
};
 
void worker_routine(Workers &ws, const string &name, FactoryResources &fr) {
    Worker *w = ws.get_worker(name);
    int product, phase;
 
    while (!w->fired) {
        while (!w->fired && !fr.try_to_find_work(w->spec, product, phase)) {
            {
                unique_lock<mutex> lock(mutex_factory);
                cv_factory.wait(lock);
            }
        }
        if (w->fired) break;
 
        {
            lock_guard<mutex> lock(mutex_stdout);
            cout << w->name << " " << place_str[w->spec] << " " << phase + 1 << " " << product_str[product] << endl;
        }
 
        this_thread::sleep_for(chrono::milliseconds(place_time_ms[w->spec]));
 
        fr.return_place(w->spec);
        if (phase + 1 == PHASE_COUNT) {
            {
                lock_guard<mutex> lock(mutex_stdout);
                cout << "done " << product_str[product] << endl;
            }
        } else {
            fr.add_part(product, phase + 1);
            cv_factory.notify_all();
        }
 
        cv_workers_activity.notify_one();
    }
 
    ws.remove(w->name);
}
 
int main() {
    Workers ws;
    FactoryResources fr;
    vector<thread> worker_threads;
    string line;

    while (getline(cin, line)) {
        const auto re = regex{"\\s+"};
        const auto args = vector<string>(
                sregex_token_iterator{line.begin(), line.end(), re, -1},
                sregex_token_iterator{}
        );
        if (args.empty()) continue;
 
        if (args[0] == "make" && args.size() >= 2) {
            int product = find_string(product_str, args[1]);
            if (product >= 0) {
                fr.add_part(product, 0);
                cv_factory.notify_all();
            }
        } else if (args[0] == "start" && args.size() >= 3) {
            int spec = find_string(place_str, args[2]);
            if (spec >= 0) {
                ws.add(args[1], spec);
                worker_threads.emplace_back(thread(
                        worker_routine,
                        ref(ws),
                        args[1],
                        ref(fr))
                );
            }
        } else if (args[0] == "end" && args.size() >= 2) {
            if(ws.has_worker(args[1])) {
                ws.fire(args[1]);
                cv_factory.notify_all();
            }
        } else if (args[0] == "add" && args.size() >= 2) {
            int place = find_string(place_str, args[1]);
            if (place >= 0) {
                fr.add_place(place);
                cv_factory.notify_all();
            }
        } else if (args[0] == "remove" && args.size() >= 2) {
            int place = find_string(place_str, args[1]);
            if (place >= 0) {
                fr.remove_place(place);
            }
        }
    }
 
    while(fr.someone_can_work(ws)) {
        {
            unique_lock<mutex> lock(mutex_factory);
            cv_workers_activity.wait(lock);
        }
    }
 
    ws.fire_all();
    cv_factory.notify_all();
 
    for (auto &th: worker_threads) {
        th.join();
    }
 
    return 0;
}