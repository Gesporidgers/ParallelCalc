// Lab2.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
#include <iostream>
#include <fstream>
#include <omp.h>
#include <iomanip>
#include <string>
#include <vector>

using namespace std;

double* generate_array(int n, bool empty = false) {
	double* arr = new double[n * n];
	srand(time(0));
	if (empty)
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < n; ++j)
				arr[i * n + j] = 0;
		}
	else
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < n; ++j)
				arr[i * n + j] = (double)rand() / RAND_MAX - 0.5;
		}
	return arr;
}

double R(double t, double n) {
	return (2 * n * n * n) / (t * 1e9);
}

void classic_times(double* arr, double* res, int n) {
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
			for (int k = 0; k < n; ++k) {
				res[i * n + j] += arr[i * n + k] * arr[k * n + j];
			}

}

void alt_times(double* arr, double* res, int n) {
	for (int i = 0; i < n; ++i)
		for (int k = 0; k < n; ++k)
			for (int j = 0; j < n; ++j) {
				res[i * n + j] += arr[i * n + k] * arr[k * n + j];
			}

}

void parallel_times(double* arr, double* res, int n) {
#pragma omp parallel for
	for (int i = 0; i < n; ++i)
		for (int k = 0; k < n; ++k)
			for (int j = 0; j < n; ++j) {
				res[i * n + j] += arr[i * n + k] * arr[k * n + j];
			}

}

void block_times(double* arr, double* res, int n, int BLOCK_SIZE) {
#pragma omp parallel for
	for (int ii = 0; ii < n; ii += BLOCK_SIZE)
		for (int jj = 0; jj < n; jj += BLOCK_SIZE)
			for (int kk = 0; kk < n; kk += BLOCK_SIZE)
				for (int i = 0; i < n && i < ii + BLOCK_SIZE; ++i)
					for (int k = 0; k < n && k < kk + BLOCK_SIZE; ++k)
						for (int j = 0; j < n && j < jj + BLOCK_SIZE; ++j) {
							res[i * n + j] += arr[i * n + k] * arr[k * n + j];
						}

}

int main()
{
	vector<int> Ns{ 256,512/*,1024,2048,4096*/ };
#pragma omp parallel
	{
		int id = omp_get_thread_num();
	}
	for (int N : Ns) {
		cout << "Calculating for N=" << N << endl;
		double* arr = generate_array(N);
		double* res = generate_array(N, true);

		double start = omp_get_wtime();
		classic_times(arr, res, N);		//классическое умножение
		double end = omp_get_wtime();
		double time = end - start;
		cout << time << ' ' << R(time, N) << endl;

		start = omp_get_wtime();
		alt_times(arr, res, N);			//ikj
		end = omp_get_wtime();
		cout << end - start << ' ' << R(end - start, N) << ' ' << time / (end - start) << endl << endl;
		time = end - start;

		for (int i = 1; i <= 16; i *= 2)
		{
			// отдельный файл для графика и последний результат в основной csv
			omp_set_num_threads(i);
			start = omp_get_wtime();
			parallel_times(arr, res, N);//параллелька
			end = omp_get_wtime();
			cout << i << ' ' << end - start << ' ' << R(end - start, N) << ' ' << time / (end - start) << endl;
		}
		delete[] arr, res;
	}
	omp_set_num_threads(16);
	cout << "Block parallel (N=512):" << endl;
	for (int i = 32; i <= 256; i *= 2)
	{
		double* arr = generate_array(512);
		double* res = generate_array(512, true);
		double start = omp_get_wtime();
		block_times(arr, res, 512, i);//блочная параллелька
		double end = omp_get_wtime();
		cout << i << ' ' << end - start << ' ' << R(end - start, 512) << endl;
		cout << endl;
		delete[] arr, res;
	}
	
	

}

