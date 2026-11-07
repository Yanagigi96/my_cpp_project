#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>

using namespace std;
using namespace chrono;

// Функция потока: умножение строк с [start_row, end_row)
void multiply_rows(int start_row, int end_row, int n,
                   const vector<vector<double>>& A,
                   const vector<vector<double>>& B,
                   vector<vector<double>>& C,
                   atomic<bool>& start_flag) {
    // Ожидание сигнала (без busy-wait)
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
    int n = 800; // Размер матрицы

    // Инициализация матриц
    vector<vector<double>> A(n, vector<double>(n, 1.0));
    vector<vector<double>> B(n, vector<double>(n, 2.0));
    vector<vector<double>> C(n, vector<double>(n, 0.0));

    int num_threads = thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4;
    
    cout << "Потоков: " << num_threads << endl;

    atomic<bool> start_flag(false);
    vector<thread> threads;

    // Создаем потоки заранее, каждый получает свой диапазон строк
    int rows_per_thread = n / num_threads;
    for (int t = 0; t < num_threads; ++t) {
        int start_row = t * rows_per_thread;
        int end_row = (t == num_threads - 1) ? n : start_row + rows_per_thread;
        threads.emplace_back(multiply_rows, start_row, end_row, n,
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
    
    cout << "Время умножения (n=" << n << "): " << ms.count() << " мс" << endl;

    // Выборочная проверка результата
    cout << "C[0][0] = " << C[0][0] << " (Ожидается: " << n * 1.0 * 2.0 << ")" << endl;

    return 0;
}