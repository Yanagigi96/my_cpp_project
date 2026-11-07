#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>

using namespace std;
using namespace chrono;

// Функция потока для многопоточного умножения
void multiply_rows(int start_row, int end_row, int n,
                   const vector<vector<double>>& A,
                   const vector<vector<double>>& B,
                   vector<vector<double>>& C,
                   atomic<bool>& start_flag) {
    // Ожидание сигнала старта (без busy-wait)
    while (!start_flag.load(memory_order_acquire)) {
        this_thread::yield();
    }

    // Порядок (i, k, j) — кэш-дружественный
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
    int sizes[] = {300, 500, 800, 1500};
    int num_threads = thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4;

    cout << "n\t1 поток (мс)\t4 потока (мс)\tУскорение (T1 / T4)" << endl;

    for (int n : sizes) {
        // Инициализация матриц
        vector<vector<double>> A(n, vector<double>(n, 1.0));
        vector<vector<double>> B(n, vector<double>(n, 2.0));
        vector<vector<double>> C_single(n, vector<double>(n, 0.0));
        vector<vector<double>> C_multi(n, vector<double>(n, 0.0));

        // --- 1. Однопоточный замер ---
        auto start_single = high_resolution_clock::now();
        for (int i = 0; i < n; ++i) {
            for (int k = 0; k < n; ++k) {
                double aik = A[i][k];
                for (int j = 0; j < n; ++j) {
                    C_single[i][j] += aik * B[k][j];
                }
            }
        }
        auto end_single = high_resolution_clock::now();
        duration<double, milli> ms_single = end_single - start_single;

        // --- 2. Многопоточный замер ---
        atomic<bool> start_flag(false);
        vector<thread> threads;
        int rows_per_thread = n / num_threads;

        for (int t = 0; t < num_threads; ++t) {
            int start_row = t * rows_per_thread;
            int end_row = (t == num_threads - 1) ? n : start_row + rows_per_thread;
            threads.emplace_back(multiply_rows, start_row, end_row, n,
                                 cref(A), cref(B), ref(C_multi), ref(start_flag));
        }

        auto start_multi = high_resolution_clock::now();
        start_flag.store(true, memory_order_release); // Сигнал к старту
        for (auto& th : threads) th.join();           // Ждем завершения
        auto end_multi = high_resolution_clock::now();
        
        duration<double, milli> ms_multi = end_multi - start_multi;

        // Вычисление ускорения
        double speedup = ms_single.count() / ms_multi.count();

        // Вывод строки таблицы
        cout << n << "\t" 
             << ms_single.count() << "\t\t" 
             << ms_multi.count() << "\t\t" 
             << speedup << endl;
    }

    return 0;
}