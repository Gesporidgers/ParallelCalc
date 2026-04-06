#include <iostream>
#include <fstream>
#include <omp.h>
#include <string>
#include <vector>
#include "csv_writer.h"

#define FULL	//для более быстрой проверки

using namespace std;

float* generate_array(int n, bool empty = false) {
	float* arr = new float[n * n];
	srand(time(0));
	if (empty)
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < n; ++j)
				arr[i * n + j] = 0;
		}
	else
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < n; ++j)
				arr[i * n + j] = (float)rand() / RAND_MAX - 0.5;
		}
	return arr;
}

double R(double t, double n) {
	return (2 * n * n * n) / (t * 1e9);
}

void classic_times(float* arr, float* res, int n) {
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
			for (int k = 0; k < n; ++k) {
				res[i * n + j] += arr[i * n + k] * arr[k * n + j];
			}

}

void alt_times(float* arr, float* res, int n) {
	for (int i = 0; i < n; ++i)
		for (int k = 0; k < n; ++k) {
			float tmp = arr[i * n + k];
			for (int j = 0; j < n; ++j) {
				res[i * n + j] += tmp * arr[k * n + j];
			}
		}


}

void parallel_times(float* arr, float* res, int n) {
#pragma omp parallel for
	for (int i = 0; i < n; ++i)
		for (int k = 0; k < n; ++k)
			for (int j = 0; j < n; ++j) {
				res[i * n + j] += arr[i * n + k] * arr[k * n + j];
			}

}

void block_times(float* arr, float* res, int n, int BLOCK_SIZE) {
#pragma omp parallel for
	for (int ii = 0; ii < n; ii += BLOCK_SIZE)
		for (int jj = 0; jj < n; jj += BLOCK_SIZE)
			for (int kk = 0; kk < n; kk += BLOCK_SIZE)
				for (int i = ii; i < n && i < ii + BLOCK_SIZE; ++i)
					for (int k = kk; k < n && k < kk + BLOCK_SIZE; ++k)
						for (int j = jj; j < n && j < jj + BLOCK_SIZE; ++j) {
							res[i * n + j] += arr[i * n + k] * arr[k * n + j];
						}

}

int main()
{
#ifdef FULL					
	vector<int> Ns{ 256,512,1024,2048,4096 };
#else
	vector<int> Ns{ 256,512,1024 };
#endif // FULL

	string filename1 = "R(N).csv";
	string filename2 = "R(BLOCK).csv";
	string filename3 = "MainTable.csv";
	string filename4 = "S(thread).csv";
	string filename5 = "BestBlock.csv";
	ofstream outFile1(filename1);
	csv_writer csv(filename5);
	
	double ijkTime;
	vector<double> Rs;
#pragma omp parallel
	{
		int id = omp_get_thread_num();
	}
	outFile1 << "N,Rclassic,Ralt,Rpar\n";
	csv << "N" << "BestBlock";
	csv.end_row();
	cout << "File R(N).csv" << endl;
	for (int N : Ns) {
		if (Rs.size() != 0) Rs.clear();
		cout << "Calculating for N=" << N << endl;
		float* arr = generate_array(N);
		float* res = generate_array(N, true);

		double start = omp_get_wtime();
		classic_times(arr, res, N);		//классическое умножение
		double end = omp_get_wtime();
		double time = end - start;
		outFile1 << N << ',' << R(time, N) << ',';
		cout << '.';

		start = omp_get_wtime();
		alt_times(arr, res, N);			//ikj
		end = omp_get_wtime();
		time = end - start;
		outFile1 << R(time, N) << ',';
		cout << '.';

		start = omp_get_wtime();
		parallel_times(arr, res, N);//параллелька
		end = omp_get_wtime();
		outFile1 << R(end - start, N) << '\n';
		cout << '.';
		double Rtmp = -1;
		int sz;
		for (int i = 32; i <= 256; i *= 2)
		{
			double start = omp_get_wtime();
			block_times(arr, res, N, i);//блочная параллелька
			double end = omp_get_wtime();
			cout << '.';
			if (R(end - start, N) > Rtmp) {
				Rtmp = R(end - start, N);
				sz = i;
			}
		}
		cout << endl;
		csv << N << sz;
		csv.end_row();// сделать вывод в файл
		delete[] arr, res;
	}
	outFile1.close();
	csv.close();
	//Блочная параллелька
	ofstream outFile2(filename2);
	outFile2 << "size,R\n";
	omp_set_num_threads(16);
	cout << "Block parallel (N=2048). File R(BLOCK).csv" << endl;
	double Rtmp = -1;
	int sz;
	for (int i = 32; i <= 256; i *= 2)
	{
		float* arr = generate_array(2048);
		float* res = generate_array(2048, true);
		double start = omp_get_wtime();
		block_times(arr, res, 2048, i);//блочная параллелька
		double end = omp_get_wtime();
		outFile2 << i << ',' << R(end - start, 2048) << '\n';
		cout << '.';
		
		delete[] arr, res;
	}
	cout << endl;
	outFile2.close();



#ifdef FULL

	// Файл для таблицы 3.2 и файл для зависимости S от потоков
	cout << "File MainTable.csv" << endl;
	ofstream outFile3(filename4);
	ofstream outFile4(filename3);
	outFile3 << "threads,S\n";
	outFile4 << "ver,time,R\n";

	float* arr = generate_array(2048);
	float* res = generate_array(2048, true);

	double start = omp_get_wtime();
	classic_times(arr, res, 2048);		//классическое умножение
	double end = omp_get_wtime();
	double time = end - start;
	outFile4 << "classic," << time << ',' << R(time, 2048) << '\n';
	cout << '.';

	start = omp_get_wtime();
	alt_times(arr, res, 2048);			//ikj
	end = omp_get_wtime();
	time = end - start;
	ijkTime = time;
	outFile4 << "ikj," << time << ',' << R(time, 2048) << '\n';
	cout << '.';

	start = omp_get_wtime();
	parallel_times(arr, res, 2048);//параллелька
	end = omp_get_wtime();
	outFile4 << "OpenMP," << end - start << ',' << R(end - start, 2048) << '\n';
	cout << '.';

	start = omp_get_wtime();
	block_times(arr, res, 2048, 32);//блочная параллелька
	end = omp_get_wtime();
	outFile4 << "block + OpenMP," << end - start << ',' << R(end - start, 2048) << '\n';
	cout << '.' << endl;
	delete[] arr, res;

	cout << "File S(thread).csv" << endl;
	for (int i = 1; i <= 16; i *= 2)
	{
		float* arr = generate_array(2048);
		float* res = generate_array(2048, true);
		omp_set_num_threads(i);
		double start = omp_get_wtime();
		parallel_times(arr, res, 2048); //параллелька
		double end = omp_get_wtime();
		outFile3 << i << ',' << ijkTime / (end - start) << '\n';
		delete[] arr, res;
		cout << '.';
	}
	cout << endl;
	outFile3.close();
#endif // FULL
}

