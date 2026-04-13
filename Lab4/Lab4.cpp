#include <iostream>
#include <csv_writer.h>
#include <algorithm>
#include <omp.h>
#include <vector>
#include <random>

#define IDX(y, x, W, H) (((y + H) % H) * W + ((x + W) % W))
using CellType = unsigned char;

CellType *generate_array(int n, bool empty = false)
{
    CellType *arr = new CellType[n];
    if (empty)
        for (int i = 0; i < n; ++i)
        {
            arr[i] = 0;
        }
    else
    {
        std::mt19937 rng(time(0));
        std::uniform_int_distribution<CellType> dist(
            0,
            1);
        for (int i = 0; i < n; ++i)
        {
            arr[i] = dist(rng);
        }
    }
    return arr;
}

CellType **generate_array2D(int n, bool empty = false)
{
    CellType **arr = new CellType *[n];
    if (empty)
        for (int i = 0; i < n; ++i)
        {
            arr[i] = new CellType[n];
            for (int j = 0; j < n; ++j)
                arr[i][j] = 0;
        }

    else
    {
        std::mt19937 rng(time(0));
        std::uniform_int_distribution<CellType> dist(
            0,
            1);
        for (int i = 0; i < n; ++i)
        {
            arr[i] = new CellType[n];
            for (int j = 0; j < n; ++j)
                arr[i][j] = dist(rng);
        }
    }
    return arr;
}

void free2D(CellType** arr, int n){
    for (int i = 0; i < n;++i)
        delete[] arr[i];
    delete[] arr;
}

void simulate_optimal(const CellType *grid_in, CellType *grid_out,
                      int *alive_per_row, int W, int H)
{
#pragma omp parallel for schedule(static)
    for (int y = 0; y < H; ++y)
    {
        int local_alive = 0;
        for (int x = 0; x < W; ++x)
        {
            // Чтение окрестности Мура (8 соседей)
            int neighbors =
                grid_in[IDX(y - 1, x - 1, W, H)] +
                grid_in[IDX(y - 1, x, W, H)] +
                grid_in[IDX(y - 1, x + 1, W, H)] +
                grid_in[IDX(y, x - 1, W, H)] +
                grid_in[IDX(y, x + 1, W, H)] +
                grid_in[IDX(y + 1, x - 1, W, H)] +
                grid_in[IDX(y + 1, x, W, H)] +
                grid_in[IDX(y + 1, x + 1, W, H)];
            CellType current = grid_in[IDX(y, x, W, H)];
            CellType next_state = 0;
            if (current == 1 && (neighbors == 2 || neighbors == 3))
                next_state = 1;
            else if (current == 0 && neighbors == 3)
                next_state = 1;
            grid_out[IDX(y, x, W, H)] = next_state;
            local_alive += next_state;
        }
        alive_per_row[y] = local_alive;
    }
}

void simulate_dynamic_fs(const CellType *grid_in, CellType *grid_out,
                         int *alive_per_row, int W, int H)
{
#pragma omp parallel for schedule(dynamic, 1)
    for (int y = 0; y < H; ++y)
    {
        int local_alive = 0;
        for (int x = 0; x < W; ++x)
        {
            // Чтение окрестности Мура (8 соседей)
            int neighbors =
                grid_in[IDX(y - 1, x - 1, W, H)] +
                grid_in[IDX(y - 1, x, W, H)] +
                grid_in[IDX(y - 1, x + 1, W, H)] +
                grid_in[IDX(y, x - 1, W, H)] +
                grid_in[IDX(y, x + 1, W, H)] +
                grid_in[IDX(y + 1, x - 1, W, H)] +
                grid_in[IDX(y + 1, x, W, H)] +
                grid_in[IDX(y + 1, x + 1, W, H)];
            CellType current = grid_in[IDX(y, x, W, H)];
            CellType next_state = 0;
            if (current == 1 && (neighbors == 2 || neighbors == 3))
                next_state = 1;
            else if (current == 0 && neighbors == 3)
                next_state = 1;
            grid_out[IDX(y, x, W, H)] = next_state;
            local_alive += next_state;
        }
        alive_per_row[y] = local_alive;
    }
}

void simulate_fragmented(CellType **grid_in, CellType **grid_out, int W, int H)
{
#pragma omp parallel for schedule(static)
    for (int y = 0; y < H; ++y)
    {
        
        for (int x = 0; x < W; ++x)
        {
            // Чтение окрестности Мура (8 соседей)
            int neighbors =
                grid_in[(y - 1 + H) % H][(x - 1 + W) % W] + // 1
                grid_in[(y - 1 + H) % H][(x + W) % W] +     // 2
                grid_in[(y - 1 + H) % H][(x + 1 + W) % W] + // 3
                grid_in[(y + H) % H][(x - 1 + W) % W] +     // 4
                grid_in[(y + H) % H][(x + 1 + W) % W] +     // 5
                grid_in[(y + 1 + H) % H][(x - 1 + W) % W] + // 6
                grid_in[(y + 1 + H) % H][(x + W) % W] +     // 7
                grid_in[(y + 1 + H) % H][(x + 1 + W) % W];  // 8
            // .// вычисление next_state .//
            CellType current = grid_in[y][x];
            CellType next_state = 0;
            if (current == 1 && (neighbors == 2 || neighbors == 3))
                next_state = 1;
            else if (current == 0 && neighbors == 3)
                next_state = 1;
            grid_out[y][x] = next_state;
        }
    }
}

using namespace std;
int main()
{
    string filename = "histogramm.csv";
    csv_writer csv1(filename);
    csv1 << "iteration,static,fragmented";
    csv1.end_row();
#pragma omp parallel
    {
        int id = omp_get_thread_num();
    }
    CellType *arrIn = generate_array(15000 * 15000);
    CellType **arrIn2D = generate_array2D(15000);
    cout << "Static + fragmented" << endl;
    //omp_set_num_threads(1);
    for (int i = 0; i < 100; ++i)
    {
        CellType *arrOut = generate_array(15000 * 15000, true);
        CellType **arrOut2D = generate_array2D(15000, true);
        int *alive = new int[15000];
        double start = omp_get_wtime();
        simulate_optimal(arrIn, arrOut, alive, 15000, 15000);
        double end = omp_get_wtime();
        double time_static = end - start;
        
        start = omp_get_wtime();
        simulate_fragmented(arrIn2D, arrOut2D, 15000, 15000);
        end = omp_get_wtime();
        double time_frag = end -start;
        cout << i + 1 << "/100" << " time_static: " << time_static << " time_frag: "<< time_frag << endl;
        csv1 << i+1 << (15000*15000)/time_static << (15000*15000)/time_frag;
        csv1.end_row();

        arrIn = arrOut;
        arrIn2D = arrOut2D;
        free2D(arrOut2D,15000);
        delete[] alive;
        delete[] arrOut;
    }
    
    csv1.close();
    
}