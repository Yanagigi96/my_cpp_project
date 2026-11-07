#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>

using namespace std;
using namespace chrono;

// Функция потока с ПЛОХИМ порядком циклов (i, j, k)
void multiply_rows_bad(int start_row, int end_row, int n,
                       const vector<vector<double>>& A,
                       const vector<vector<double>>& B,
                       vector<vector<double>>& C,
                       atomic<bool>& start_flag) {
    // Ожидание сигнала старта (без busy-wait)
    while (!start_flag.load(memory_order_acquire)) {
        this_thread::yield();
    }

    // Плохой порядок (i, j, k) — постоянные промахи кэша
    for (int i = start_row; i < end_row; ++i) {
        for (int j = 0; j < n; ++j) {
            for (int k = 0; k < n; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

int main() {
    int n = 800;
    int num_threads = thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4;

    // Инициализация матриц
    vector<vector<double>> A(n, vector<double>(n, 1.0));
    vector<vector<double>> B(n, vector<double>(n, 2.0));
    vector<vector<double>> C(n, vector<double>(n, 0.0));

    atomic<bool> start_flag(false);
    vector<thread> threads;
    int rows_per_thread = n / num_threads;

    // Создаем потоки заранее
    for (int t = 0; t < num_threads; ++t) {
        int start_row = t * rows_per_thread;
        int end_row = (t == num_threads - 1) ? n : start_row + rows_per_thread;
        threads.emplace_back(multiply_rows_bad, start_row, end_row, n,
                             cref(A), cref(B), ref(C), ref(start_flag));
    }

    // Замер ТОЛЬКО времени вычислений
    auto start_time = high_resolution_clock::now();
    
    // Даем потокам старт
    start_flag.store(true, memory_order_release); 

    // Ждем завершения
    for (auto& th : threads) {
        th.join();
    }

    auto end_time = high_resolution_clock::now();
    duration<double, milli> ms = end_time - start_time;
    
    cout << "Время (плохой порядок i, j, k, " << num_threads << " потоков, n=" << n << "): " 
         << ms.count() << " мс" << endl;

    return 0;
}