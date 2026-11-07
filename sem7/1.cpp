#include <iostream>
#include <vector>
#include <chrono>

using namespace std;
using namespace chrono;

int main() {
    int n = 800;
    vector<vector<double>> A(n, vector<double>(n, 1.0));
    vector<vector<double>> B(n, vector<double>(n, 2.0));
    vector<vector<double>> C(n, vector<double>(n, 0.0));

    auto start = high_resolution_clock::now();

    for (int i = 0; i < n; ++i) {
        for (int k = 0; k < n; ++k) {
            double aik = A[i][k];
            for (int j = 0; j < n; ++j) {
                C[i][j] += aik * B[k][j];
            }
        }
    }

    auto end = high_resolution_clock::now();
    duration<double, milli> ms = end - start;
    
    cout << "Время (1 поток, i,k,j): " << ms.count() << " мс" << endl;
    return 0;
}