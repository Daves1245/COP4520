#include <bits/stdc++.h>
#include <atomic>
using namespace std;
using ll = long long;
bool isprime(int n) {
    if (n < 2) return 0;
    if (n == 2) return 1;
    if (n == 3) return 1;
    if (!(n & 1)) return 0;
    if (n % 3 == 0) return 0;
    for (int i = 5; i * i <= n; i += 2) {
        if ((n % i) == 0) return 0;
    }
    return 1;
}
atomic<ll> prime_sum = 0;
atomic<ll> prime_cnt = 0;
vector<int> is_prime(1e8, 0);
void check_region(int l, int r) {
    for (int n = l; n < r; n++) {
        if (isprime(n)) {
            prime_sum += n;
            prime_cnt++;
            is_prime[n] = 1;
        }
    }
}
constexpr int N = 1e8;
int main() {
    ofstream out("primes.txt");
    auto start = chrono::high_resolution_clock::now();
    int len = N / 8;
    vector<thread> threads;
    for (int i = 0; i < 8; i++) {
        threads.push_back(thread(check_region, i * len, (i + 1) * len));
    }
    for (auto &t : threads) {
        t.join();
    }
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    cout << duration.count() << "s " << prime_cnt << " " << prime_sum << endl;
    vector<int> primes;
    for (int i = N - 1; primes.size() < 10; i--) {
        if (is_prime[i]) primes.push_back(i);
    }
    sort(primes.begin(), primes.end());
    for (auto p : primes) cout << p << " "; cout << endl;

    return 0;
}

