#include <bits/stdc++.h>
#include <mutex>
#include <thread>
#include <random>

#define MAX_N 5000000

using namespace std;

atomic<int> notes_written{0};

struct node {
    int val;
    node *next;
    mutex n_lock;

    node(int v) : val(v), next(nullptr) {}
};

class llist {
private:
    node *head = new node(0);
    node *tail = new node(MAX_N);

public:
    llist() {
        head->next = tail;
    }

    // insert value into the list without sorted order. we use this to emulate the unordered bag
    // that servants take from later
    void unordered_insert(int v) {
        head->n_lock.lock();
        node *pred = head;
        node *cur = pred->next;
        cur->n_lock.lock();

        node *newnode = new node(v);
        newnode->next = cur;
        pred->next = newnode;
        pred->n_lock.unlock();
        cur->n_lock.unlock();
    }

    // fine grade synchronization
    void insert(int v) {
        // cout << "F" << endl;
        // base case: lock head as predecessor, only increment when locked
        head->n_lock.lock();
        node *pred = head;
        node *cur = pred->next;
        cur->n_lock.lock();

        while (cur->val < v) {
            // cout << "G" << endl;
            // inductive step: unlock only after next is locked
            pred->n_lock.unlock();
            pred = cur;
            // update our next
            cur= cur->next;
            cur->n_lock.lock();
        }

        if (cur->val != v) {
            // cout << "H" << endl;
            // only create if we find a place for the present (this avoids a memory leak)
            node *newnode = new node(v); 
            newnode->next = cur;
            pred->next = newnode;
        }

        // cout << "I" << endl;
        pred->n_lock.unlock();
        cur->n_lock.unlock();
    }

    // remove from the head of the list (will return value of present removed)
    int remove() {
        // cout << "J" << endl;
        head->n_lock.lock();
        // cout << "K" << endl;
        node *pred = head;
        // cout << "L" << endl;
        node *cur = pred->next;
        // cout << "M" << endl;
        cur->n_lock.lock();
        // cout << "N" << endl;

        if (cur != tail) {
            int ret = cur->val;
            pred->next = cur->next;
            cur->n_lock.unlock();
            delete cur;
            pred->n_lock.unlock();
            return ret;
        }
        // cout << "O" << endl;
        cur->n_lock.unlock();
        pred->n_lock.unlock();
        return -1;
    }

    bool search(int v) {
        // similar logic to insert
        head->n_lock.lock();
        node *pred = head;
        node *cur= pred->next;
        cur->n_lock.lock();

        while (cur->val < v) {
            pred->n_lock.unlock();
            pred = cur;
            cur = cur->next;
            cur->n_lock.lock();
        }

        if (cur->val == v) {
            cur->n_lock.unlock();
            pred->n_lock.unlock();
            return true;
        } else {
            cur->n_lock.unlock();
            pred->n_lock.unlock();
            return false;
        }
    }


    void print() {
        node *current = head->next;
        while (current != tail) {
            cout << current->val << " ";
            current = current->next;
        }
        cout << '\n';
    }
};

int main() {
    // faster io
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    llist ll;
    vector<thread> pool;
    int threads = 4;

    vector<int> presents(MAX_N);
    vector<int> seen(MAX_N, 0);
    for (int i = 0; i < presents.size(); i++) {
        presents[i] = i + 1;
    }

    // cpp reference shuffle()
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(presents.begin(), presents.end(), default_random_engine(seed));
    srand(seed);

    // We'll use the concurrent linked list to simulate the unordered bag that each servant takes a present from
    llist bag;
    // int cnt = 0;
    for (auto i : presents) {
        bag.unordered_insert(i);
        /*
        if (!(cnt % 1000)) {
            cout << "presents added: " << cnt << endl;
        }
        cnt++;
        */
    }

    auto choice = [&]() -> void {
        // Alternate between adding a present to the chain and removing while writing a thank you note
        for (int i = 0; i < presents.size(); i++) {
            if (i & 1) {
                int present = bag.remove();
                if (present == -1) continue;
                ll.insert(present);
            } else {
                if (ll.remove() != -1) {
                    notes_written++;
                }
            }
        }
        // cout << "C" << endl;
    };

    for (int i = 0; i < threads; i++) {
        pool.push_back(thread(choice));
    }

    // cout << "D" << endl;
    for (auto &t : pool) {
        t.join();
    }

    cout << "Servants done working" << endl;
    cout << "Notes written: " << notes_written << endl;

    // cout << "E" << endl;

    cout << "Chain: ";
    ll.print();
    return 0;
}
