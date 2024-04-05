#include <bits/stdc++.h>
#include <thread>
#include <random>
using namespace std;

#define NUM_THREADS 8

struct entry {
    int timestamp;
    int temp;
    int owner;
};

vector<struct entry> readings(60 * NUM_THREADS);

atomic<int> arr_index = 0;

void sensor(int id) {
    mt19937 rng(random_device{}()); // Generator
    uniform_int_distribution<int> dist(-100, 70); // Distribution in [0, 1]
                                                  //
    for (int i = 0; i < 60; i++) {
        // this is guaranteed read-write-modify by c++ standard
        int idx = arr_index++;
        readings[idx] = {
            i,
            dist(rng),
            id
        };
    }
}

void calculate() {
    /*
       // debug check sensible values
    for (int i = 0; i < readings.size(); i++) {
        auto r = readings[i];
        cout << i << ": " << endl;
        cout << "time: " << r.timestamp << endl;
        cout << "temp: " << r.temp << endl;
        cout << "owner: " << r.owner << endl;
    }
    */

    priority_queue<int, vector<int>, less<int>> min_temps;
    priority_queue<int, vector<int>, greater<int>> max_temps;
    vector<vector<int>> readings_by_owner(8, vector<int>());

    for (auto r : readings) {
        max_temps.push(r.temp);
        min_temps.push(r.temp);
        if (max_temps.size() > 5) max_temps.pop();
        if (min_temps.size() > 5) min_temps.pop();
        readings_by_owner[r.owner].push_back(r.temp);
    }

    for (auto &v : readings_by_owner) {
        sort(v.begin(), v.end());
    }

    int ma = 0;
    int ma_time = 0;
    for (int i = 0; i < readings_by_owner.size(); i++) {
        int ma_cur = -100;
        int mi_cur = 70;
        for (int j = 0; j < readings_by_owner[i].size() - 10; j++) {
            ma_cur = max(ma_cur, readings_by_owner[i][j]);
            mi_cur = min(mi_cur, readings_by_owner[i][j]);
        }
        if (abs(ma_cur - mi_cur) > ma) {
            ma = abs(ma_cur - mi_cur);
            ma_time = i + 1; // one index
        }
    }

    cout << "5 highest readings: ";
    while (max_temps.size()) {
        cout << max_temps.top() << " "; max_temps.pop();
    }
    cout << endl;
    cout << "5 lowest readings: "; 
    while (min_temps.size()) {
        cout << min_temps.top() << " "; min_temps.pop();
    }
    cout << endl;
    cout << "Max different temp reading: " << ma << endl;
    cout << "Time at which max different temp reading was recorded: " << ma_time << " - " << ma_time + 10 << endl;
    cout << endl;
}

int main() {
    vector<thread> sensors;

    int hour = 1;
    while (hour < 100) {
        for (int i = 0; i < NUM_THREADS; i++) {
            sensors.push_back(thread(sensor, i));
        }

        for (auto &t : sensors) {
            t.join();
        }

        sensors.clear();
        cout << "Hour " << hour++ << ": " << endl;
        calculate();
        arr_index = 0;
    }
    return 0;
}
