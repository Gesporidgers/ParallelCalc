#include <iostream>
#include <csv_writer.h>
#include <algorithm>
#include <omp.h>
#include <vector>
#include <random>

int* generate_array(int n) {
    int* arr = new int[n];
    std::mt19937 rng(time(0));
    std::uniform_int_distribution<int> dist(
        -2'147'483'648,
        2'147'483'647
    );
    for (int i = 0; i < n; ++i) {
        arr[i] = dist(rng);
    }
    return arr;
}

// Стандартное разбиение схемы Ломуто (Lomuto partition scheme)
int partition(int* arr, int low, int high) {
    int pivot = arr[high];
    int i = (low - 1);
    for (int j = low; j <= high - 1; j++ ) {
        if (arr[j] < pivot) {
            i++;
        std::swap(arr[i], arr[j]);
        }
    }
    std::swap(arr[i + 1], arr[high]);
    return (i + 1);
}


void parallel_quicksort_optimized(int* arr, int low, int high, int cutoff) {
    // 1. Условие отсечки (Cutoff)
    if (high - low < cutoff) {
        // Передаем работу стандартному алгоритму
    std::sort(arr + low, arr + high + 1);
        return;
    }
    int pivot = partition(arr, low, high);
    // 2. Создание задач с явным указанием атрибутов данных
    // arr передается как shared, так как сортировка in-place.
    // low, pivot, high, cutoff копируются в контекст задачи (firstprivate).
#pragma omp task shared(arr) firstprivate(low, pivot, cutoff)
    parallel_quicksort_optimized(arr, low, pivot - 1, cutoff);
#pragma omp task shared(arr) firstprivate(high, pivot, cutoff)
    parallel_quicksort_optimized(arr, pivot + 1, high, cutoff);
    // 3. Синхронизация
    // Гарантирует, что текущий вызов не завершится, пока левая и правая 
    // части не будут полностью отсортированы.
#pragma omp taskwait
}

void run_parallel_sort(int* arr, int N, int cutoff) {
    // Создаем команду потоков (Thread pool)
#pragma omp parallel
    {
        // Только мастер-поток (или первый освободившийся) генерирует корневую задачу
#pragma omp single nowait
        {
            parallel_quicksort_optimized(arr, 0, N - 1, cutoff);
        }
            // Неявный барьер в конце parallel гарантирует, 
            // что все потоки дождутся завершения дерева задач.
    }
}

using namespace std;
int main()
{
    int N = 5'000'000'0;
    string filename1 = "T(C).csv";
    string filename2 = "S(P).csv";
    string filename3 = "E(P).csv";
    csv_writer csv1(filename1);
    int opt_C = 0;
    double min_time =100;
    csv1 << "cutoff,T";
    csv1.end_row();
    vector<int> cutoffs{ 10,100,1000,10000,100000,1'000'000 };
#pragma omp parallel
    {
        int id = omp_get_thread_num();
    }
    omp_set_num_threads(16);
    for (int cutoff : cutoffs) {
        cout << "Cutoff = " << cutoff << endl;
        int* arr = generate_array(N);
        double start = omp_get_wtime();
        run_parallel_sort(arr, N, cutoff);
        double end = omp_get_wtime();
        csv1 << cutoff << end - start;
        csv1.end_row();
        if (min_time>end - start){
            min_time = end-start;
            opt_C=cutoff;
        }
        delete[] arr;
    }
    csv1.close();
    cout << "optimum cutoff = " << opt_C << " with time " << min_time << endl;

    cout << "sequental sort..." << endl;
    int* arr = generate_array(N);
    double start = omp_get_wtime();
    sort(arr,arr+N);
    double end = omp_get_wtime();
    double time = end - start;
    cout << time << endl;
    csv_writer csv2(filename2), csv3(filename3);
    csv2 << "P,S";
    csv2.end_row();
    csv3 << "P,E";
    csv3.end_row();
    cout << "File S(P).csv and E(P)" << endl;
    for (int i = 1; i <= 16 ; i*=2){
        cout << "P = " << i << endl;
        int* arr = generate_array(N);
        omp_set_num_threads(i);
        start = omp_get_wtime();
        run_parallel_sort(arr,N,opt_C);
        end = omp_get_wtime();
        double sp = time / (end - start);
        csv2 << i << sp;
        csv2.end_row();
        csv3 << i << sp/i;
        csv3.end_row();
        delete[] arr;
    }
    csv2.close();
    csv3.close();
}
