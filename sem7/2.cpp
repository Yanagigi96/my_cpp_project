#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

using namespace std;
using namespace chrono;

void multiply_rows(int start_row, int end_row, int n,
                   const vector<vector<double>>& A,
                   const vector<vector<double>>& B,
                   vector<vector<double>>& C) {
    for (int i = start_row; i < end_row; ++i) {
        for (int k = 0; k < n; ++k) {
            double aik = A[i][k];
            for (int j = 0; j < n; ++j) {
                C[i][j] += aik * B[k][j];
            }
        }
    }
}

int main() {
    int n = 800;
    vector<vector<double>> A(n, vector<double>(n, 1.0));
    vector<vector<double>> B(n, vector<double>(n, 2.0));
    vector<vector<double>> C(n, vector<double>(n, 0.0));

    int num_threads = thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4;

    vector<thread> threads;
    int rows_per_thread = n / num_threads;

    auto start = high_resolution_clock::now();

    for (int t = 0; t < num_threads; ++t) {
        int start_row = t * rows_per_thread;
        int end_row = (t == num_threads - 1) ? n : start_row + rows_per_thread;
        threads.emplace_back(multiply_rows, start_row, end_row, n,
                             cref(A), cref(B), ref(C));
    }

    for (auto& th : threads) th.join();

    auto end = high_resolution_clock::now();
    duration<double, milli> ms = end - start;
    cout << "Время (наивно, " << num_threads << " потоков): " << ms.count() << " мс" << endl;

    return 0;
}