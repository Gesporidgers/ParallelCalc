#include <iostream>
#include <fstream>
#include <omp.h>
#include <string>
#include <vector>

using namespace std;

void generate_array(double* arr, int n) {
	srand(time(0));
	for (int i = 0; i < n; ++i) {
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
	//откроем файл
	string filename = "out.csv";
	ofstream outFile(filename);
	//пустышка для выделения потоков
#pragma omp parallel
	{
		int id = omp_get_thread_num();
	}
	outFile << "N,Ts,Tred,Tcrit,Sred,Ered,Scrit,Ecrit\n";
	vector<int> Ns{ 10000,1000000,10000000/*,1000000000*/ }; //размерности массивов
	for (int N : Ns) {
		cout << "Calculating for N=" << N << endl; // для понимания что делает программа
		double* arr = new double[N];
		generate_array(arr, N); // создаём массив
		double sum;
		double timeseq=0, timepar=0, timecrit=0, sred=0, ered=0, scrit=0, ecrit=0;

		
		for (int i = 0; i < 20; i++) {
			//прогон последовательный
			double start = omp_get_wtime();
			sum = sequential_sum(arr, N);
			double end = omp_get_wtime();
			double ts = end - start;

			//прогон параллельный с готовыми потоками, чтобы не тратилось время
			start = omp_get_wtime();
			sum = parallel_sum(arr, N);
			end = omp_get_wtime();
			double tp = end - start;

			//прогон параллельный но с мьютексом
			start = omp_get_wtime();
			sum = critical_sum(arr, N);
			end = omp_get_wtime();
			double tc = end - start;
			// прибавляем получившееся для последующего усреднения
			timeseq += ts; timepar += tp; timecrit += tc; sred += timeseq / timepar; ered += timeseq / (timepar * 16); scrit += timeseq / timecrit; ecrit += timeseq / (timecrit * 16);
		}
		delete[] arr;
		outFile << N << ',' << timeseq / 20 << ',' << timepar / 20 << ',' << timecrit / 20 << ',' << sred / 20 << ',' << ered / 20 << ',' << scrit / 20 << ',' << ecrit / 20 << '\n';
		
	}
	outFile.close();
}
