#include <iostream>
#include <fstream>
#include <omp.h>
#include <string>
#define N 100000

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

double critical_sum(double* arr, int n) {

    double sum = 0;
#pragma omp parallel for
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
    outFile << "Ts,Tred,Tcrit,Sred,Ered,Scrit,Ecrit\n";
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

        start = omp_get_wtime();
        sum = critical_sum(array, N);
        end = omp_get_wtime();
        double timecrit = end - start;
        outFile << timeseq << ',' << timepar << ',' << timecrit<< ',' << timeseq / timepar << ',' << timeseq / (timepar * 16) << ',' << timeseq / timecrit << ',' << timeseq / (timecrit * 16) << '\n';
    }
    outFile.close();
}
