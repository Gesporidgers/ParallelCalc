#include <iostream>
#include <fstream>
#include <omp.h>
#include <string>
#include <algorithm>
#define N 10000

using namespace std;

void generate_array(double* arr, int n) {
    srand(time(0));
    for (int i = 0; i<n;++i) {
        arr[i] = rand() % 2 - 0.5;
    }
}

double sequential_sum(double* arr, int n) {
    double sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}



double parallel_sum(double* arr, int n) {

    double sum = 0;
#pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main()
{
    double* array = new double[N];
    generate_array(array, N); // создаём массив
    double sum;
    
    //откроем файл
    string filename = "N" + to_string(N) + ".csv";
    ofstream outFile(filename);

    //пустышка для выделения потоков
#pragma omp parallel
    {
    int id = omp_get_thread_num(); 
    }
    outFile << "Ts,Tp,S,E\n";
    for (int i = 0; i < 20; i++) {
        double start = omp_get_wtime();
        sum = sequential_sum(array, N);
        double end = omp_get_wtime();
        double timeseq = end - start;
        
        //прогон с замером и готовыми потоками, чтобы не тратилось время
        start = omp_get_wtime();
        sum = parallel_sum(array, N);
        end = omp_get_wtime();
        double timepar = end - start;
        outFile << timeseq << ',' << timepar << ',' << timeseq/timepar << ',' << timeseq / (timepar*16) << '\n';
    }
    outFile.close();
}
