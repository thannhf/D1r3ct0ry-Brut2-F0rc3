#include <iostream>
#include <fstream>
#include <string>   
#include <vector>
#include <thread>
#include <mutex>
#include <cpr/cpr.h>

using namespace std;

mutex mtx;
size_t index_primary = 0;
vector<string> wordlist;
string target_url = "http://target.com/"; 

void scan_worker() {
    while(true) {
        string word;

        mtx.lock();
        if(index_primary >= wordlist.size()) {
            mtx.unlock();
            break;
        }
        word = wordlist[index_primary];
        index_primary++;
        mtx.unlock();

        auto r = cpr::Get(cpr::Url{target_url + word}, cpr::Timeout{5000});

        if(r.status_code == 200 || r.status_code == 403) {
            lock_guard<mutex> lock(mtx);
            cout << "[" << r.status_code << "] Found: " << target_url << word << endl;
        }
    }
}

int main() {
    ifstream file("wordlist.txt");
    string line;

    if (!file.is_open()) {
        cerr << "Loi: Khong tim thay file wordlist.txt!" << endl;
        return 1;
    }

    while (getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back(); 
        if (!line.empty()) wordlist.push_back(line);
    }
    file.close();

    if (wordlist.empty()) {
        cout << "Wordlist rong!" << endl;
        return 0;
    }

    cout << "Ban muon quet voi bao nhieu luong: ";
    int num_threads;
    cin >> num_threads;

    if (num_threads > wordlist.size()) num_threads = wordlist.size();

    vector<thread> workers;
    cout << "---- Dang quet " << wordlist.size() << " duong dan voi " << num_threads << " luong ----" << endl;

    for(int i = 0; i < num_threads; ++i) {
        workers.push_back(thread(scan_worker));
    }

    for(auto &t : workers) {
        t.join();
    }

    cout << "--- Completed ---" << endl;
    return 0;
}