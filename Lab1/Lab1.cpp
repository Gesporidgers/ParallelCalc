#include <iostream>
#include <fstream>
#include <omp.h>
#include <iomanip>
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
#pragma omp parallel
	{
		double local_sum = 0;
#pragma omp for
		for (int i = 0; i < n; i++) {
			local_sum += arr[i];
		}
#pragma omp critical
		{
			sum += local_sum;
		}
	}
	
	return sum;
}

double atomic_sum(double* arr, int n) {
	double sum = 0;
#pragma omp parallel
	{
		double local_sum = 0;
#pragma omp for
		for (int i = 0; i < n; i++) {
			local_sum += arr[i];
		}
#pragma omp atomic
		sum += local_sum;
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
	outFile << "N,Ts,Tred,Tcrit,Tatom,Sred,Ered,Scrit,Ecrit,Satom,Eatom\n";
	outFile << fixed << setprecision(6);
	cout << fixed << setprecision(10);
	vector<int> Ns{ 10000,100000,1000000,10000000/*,1000000000*/ }; //размерности массивов
	for (int N : Ns) {
		cout << "Calculating for N=" << N << endl; // для понимания что делает программа
		double* arr = new double[N];
		generate_array(arr, N); // создаём массив
		double sum;
		double timeseq=0, timepar=0, timecrit=0, sred=0, ered=0, scrit=0, ecrit=0, timeatom=0, satom=0, eatom=0;

		
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
			
			//прогон параллельный но с атомарной операцией
			start = omp_get_wtime();
			sum = atomic_sum(arr, N);
			end = omp_get_wtime();
			double ta = end - start;
			cout << '.'; // подобие полосы загрузки для понимания
			// прибавляем получившееся для последующего усреднения
			
			timeseq += ts; timepar += tp; timecrit += tc; timeatom += ta; sred += timeseq / timepar; ered += timeseq / (timepar * 16); scrit += timeseq / timecrit; ecrit += timeseq / (timecrit * 16); satom += timeseq / timeatom; eatom += timeseq / (timeatom * 16);
		}
		delete[] arr;
		outFile << N << ',' << timeseq / 20 << ',' << timepar / 20 << ',' << timecrit / 20 << ','<< timeatom / 20 << ',' 
			<< (timeseq / 20) / (timepar / 20) << ',' << (timeseq / 20) / ((timepar / 20)*16) << ','
			<< (timeseq / 20)/(timecrit/20) << ',' << (timeseq / 20)/((timecrit/20)*16)<< ','
			<< (timeseq / 20)/(timeatom/20) << ',' << (timeseq / 20)/((timeatom/20)*16) << '\n';
		cout << endl; // вывод в csv файл усреднённых значений
	}
	outFile.close();
}
